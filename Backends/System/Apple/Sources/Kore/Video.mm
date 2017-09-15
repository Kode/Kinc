#include "pch.h"

#include "Video.h"

#import <AVFoundation/AVFoundation.h>
#include <Kore/Audio1/Audio.h>
#include <Kore/Graphics4/Texture.h>
#include <Kore/IO/FileReader.h>
#include <Kore/Log.h>
#include <Kore/System.h>
#include <Kore/VideoSoundStream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Kore;

extern const char* iphonegetresourcepath();
extern const char* macgetresourcepath();

VideoSoundStream::VideoSoundStream(int nChannels, int freq) : bufferSize(1024 * 100), bufferReadPosition(0), bufferWritePosition(0), read(0), written(0) {
	buffer = new float[bufferSize];
}

void VideoSoundStream::insertData(float* data, int nSamples) {
	for (int i = 0; i < nSamples; ++i) {
		float value = data[i]; // / 32767.0;
		buffer[bufferWritePosition++] = value;
		++written;
		if (bufferWritePosition >= bufferSize) {
			bufferWritePosition = 0;
		}
	}
}

float VideoSoundStream::nextSample() {
	++read;
	if (written <= read) {
		printf("Out of audio\n");
		return 0;
	}
	if (bufferReadPosition >= bufferSize) {
		bufferReadPosition = 0;
		printf("buffer read back - %i\n", (int)(written - read));
	}
	return buffer[bufferReadPosition++];
}

bool VideoSoundStream::ended() {
	return false;
}

struct Video::Impl {
	id videoAsset;
	id assetReader;
	id videoTrackOutput;
	id audioTrackOutput;
	id url;
};

Video::Video(const char* filename) : playing(false), sound(nullptr), impl(new Impl) {
	char name[2048];
#ifdef KORE_IOS
	strcpy(name, iphonegetresourcepath());
#else
	strcpy(name, macgetresourcepath());
#endif
	strcat(name, "/");
	strcat(name, KORE_DEBUGDIR);
	strcat(name, "/");
	strcat(name, filename);
	impl->url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:name]];
	image = nullptr;
	myWidth = -1;
	myHeight = -1;
	load(0);
}

void Video::load(double startTime) {
	videoStart = startTime;
	AVURLAsset* asset = [[AVURLAsset alloc] initWithURL:impl->url options:nil];
	impl->videoAsset = asset;

	AVAssetTrack* videoTrack = [[asset tracksWithMediaType:AVMediaTypeVideo] objectAtIndex:0];
	NSDictionary* videoOutputSettings =
	    [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA], kCVPixelBufferPixelFormatTypeKey, nil];
	AVAssetReaderTrackOutput* videoOutput = [AVAssetReaderTrackOutput assetReaderTrackOutputWithTrack:videoTrack outputSettings:videoOutputSettings];

	bool hasAudio = [[asset tracksWithMediaType:AVMediaTypeAudio] count] > 0;
	AVAssetReaderAudioMixOutput* audioOutput = nullptr;
	if (hasAudio) {
		AVAssetTrack* audioTrack = [[asset tracksWithMediaType:AVMediaTypeAudio] objectAtIndex:0];
		NSDictionary* audioOutputSettings = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:kAudioFormatLinearPCM], AVFormatIDKey, [NSNumber numberWithFloat:44100.0], AVSampleRateKey,
			[NSNumber numberWithInt:32], AVLinearPCMBitDepthKey, [NSNumber numberWithBool:NO], AVLinearPCMIsNonInterleaved,
	        [NSNumber numberWithBool:YES], AVLinearPCMIsFloatKey, [NSNumber numberWithBool:NO], AVLinearPCMIsBigEndianKey, nil];
		audioOutput = [AVAssetReaderAudioMixOutput assetReaderAudioMixOutputWithAudioTracks:@[ audioTrack ] audioSettings:audioOutputSettings];
	}
	
	AVAssetReader* reader = [AVAssetReader assetReaderWithAsset:asset error:nil];

	if (startTime > 0) {
		CMTimeRange timeRange = CMTimeRangeMake(CMTimeMake(startTime * 1000, 1000), kCMTimePositiveInfinity);
		reader.timeRange = timeRange;
	}

	[reader addOutput:videoOutput];
	if (hasAudio) {
		[reader addOutput:audioOutput];
	}
	
	impl->assetReader = reader;
	impl->videoTrackOutput = videoOutput;
	if (hasAudio) {
		impl->audioTrackOutput = audioOutput;
	}
	else {
		impl->audioTrackOutput = nullptr;
	}
	
	if (myWidth < 0) myWidth = [videoTrack naturalSize].width;
	if (myHeight < 0) myHeight = [videoTrack naturalSize].height;
	int framerate = [videoTrack nominalFrameRate];
	printf("Framerate: %i\n", framerate);
	next = videoStart;
	audioTime = videoStart * 44100;
}

Video::~Video() {
	stop();
	delete impl;
}

#ifdef KORE_IOS
void iosPlayVideoSoundStream(VideoSoundStream* video);
void iosStopVideoSoundStream();
#else
void macPlayVideoSoundStream(VideoSoundStream* video);
void macStopVideoSoundStream();
#endif

void Video::play() {
	AVAssetReader* reader = impl->assetReader;
	[reader startReading];

	sound = new VideoSoundStream(2, 44100);
	// Mixer::play(sound);
#ifdef KORE_IOS
	iosPlayVideoSoundStream(sound);
#else
	macPlayVideoSoundStream(sound);
#endif
	
	playing = true;
	start = System::time() - videoStart;
}

void Video::pause() {
	playing = false;
	if (sound != nullptr) {
		// Mixer::stop(sound);
#ifdef KORE_IOS
		iosStopVideoSoundStream();
#else
		macStopVideoSoundStream();
#endif
		delete sound;
		sound = nullptr;
	}
}

void Video::stop() {
	pause();
}

void Video::updateImage() {
	if (!playing) return;
	{
		AVAssetReaderTrackOutput* videoOutput = impl->videoTrackOutput;
		CMSampleBufferRef buffer = [videoOutput copyNextSampleBuffer];
		if (!buffer) {
			AVAssetReader* reader = impl->assetReader;
			if ([reader status] == AVAssetReaderStatusCompleted) {
				stop();
			}
			else {
				pause();
				load(next);
				play();
			}
			return;
		}
		next = CMTimeGetSeconds(CMSampleBufferGetOutputPresentationTimeStamp(buffer));

		CVImageBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(buffer);

		if (image == nullptr) {
			CGSize size = CVImageBufferGetDisplaySize(pixelBuffer);
			myWidth = size.width;
			myHeight = size.height;
			image = new Graphics4::Texture(width(), height(), Graphics4::Image::BGRA32, false);
		}

		if (pixelBuffer != NULL) {
			CVPixelBufferLockBaseAddress(pixelBuffer, 0);
#ifdef KORE_OPENGL
			image->upload((u8*)CVPixelBufferGetBaseAddress(pixelBuffer), static_cast<int>(CVPixelBufferGetBytesPerRow(pixelBuffer) / 4));
#endif
			CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
			
		}
		CFRelease(buffer);
	}

	if (impl->audioTrackOutput != nullptr) {
		AVAssetReaderAudioMixOutput* audioOutput = impl->audioTrackOutput;
		while (audioTime / 44100.0 < next + 0.1) {
			CMSampleBufferRef buffer = [audioOutput copyNextSampleBuffer];
			if (!buffer) return;
			CMItemCount numSamplesInBuffer = CMSampleBufferGetNumSamples(buffer);
			AudioBufferList audioBufferList;
			CMBlockBufferRef blockBufferOut = nil;
			CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(buffer, NULL, &audioBufferList, sizeof(audioBufferList), NULL, NULL,
			                                                        kCMSampleBufferFlag_AudioBufferList_Assure16ByteAlignment, &blockBufferOut);
			for (int bufferCount = 0; bufferCount < audioBufferList.mNumberBuffers; ++bufferCount) {
				float* samples = (float*)audioBufferList.mBuffers[bufferCount].mData;
				if (audioTime / 44100.0 > next - 0.1) {
					sound->insertData(samples, (int)numSamplesInBuffer * 2);
				}
				else {
					// Send some data anyway because the buffers are huge
					sound->insertData(samples, (int)numSamplesInBuffer);
				}
				audioTime += numSamplesInBuffer;
			}
			CFRelease(blockBufferOut);
			CFRelease(buffer);
		}
	}
}

void Video::update(double time) {
	if (playing && time >= start + next) {
		updateImage();
	}
}

int Video::width() {
	return myWidth;
}

int Video::height() {
	return myHeight;
}

Graphics4::Texture* Video::currentImage() {
	update(System::time());
	return image;
}
