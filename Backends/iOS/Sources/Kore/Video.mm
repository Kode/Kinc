#include "pch.h"
#include "Video.h"
#include <Kore/IO/FileReader.h>
#include <Kore/Graphics/Texture.h>
#include <Kore/System.h>
#include <Kore/VideoSoundStream.h>
#include <Kore/Audio/Mixer.h>
#include <Kore/Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#import <AVFoundation/AVFoundation.h>

using namespace Kore;

extern const char* iphonegetresourcepath();

VideoSoundStream::VideoSoundStream(int nChannels, int freq) : bufferSize(1024 * 100), bufferReadPosition(0), bufferWritePosition(0), read(0), written(0) {
	buffer = new float[bufferSize];
}

void VideoSoundStream::insertData(float* data, int nSamples) {
	for (int i = 0; i < nSamples; ++i) {
		float value = data[i];// / 32767.0;
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

Video::Video(const char* filename) : playing(false), sound(nullptr) {
	char name[2048];
	strcpy(name, iphonegetresourcepath());
	strcat(name, "/");
	strcat(name, KORE_DEBUGDIR);
	strcat(name, "/");
	strcat(name, filename);
	url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:name]];
	image = nullptr;
	myWidth = -1;
	myHeight = -1;
	load(0);
}

void Video::load(double startTime) {
	videoStart = startTime;
	AVURLAsset* asset = [[AVURLAsset alloc] initWithURL:url options:nil];
	videoAsset = asset;
	
	AVAssetTrack* videoTrack = [[asset tracksWithMediaType:AVMediaTypeVideo] objectAtIndex:0];
	NSDictionary* videoOutputSettings = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA], kCVPixelBufferPixelFormatTypeKey, nil];
	AVAssetReaderTrackOutput* videoOutput = [AVAssetReaderTrackOutput assetReaderTrackOutputWithTrack:videoTrack outputSettings:videoOutputSettings];
	
	AVAssetTrack* audioTrack = [[asset tracksWithMediaType:AVMediaTypeAudio] objectAtIndex:0];
	NSDictionary* audioOutputSettings = [NSDictionary dictionaryWithObjectsAndKeys:
										 [NSNumber numberWithInt:kAudioFormatLinearPCM], AVFormatIDKey,
										 [NSNumber numberWithFloat:44100.0], AVSampleRateKey,
										 [NSNumber numberWithInt:32], AVLinearPCMBitDepthKey,
										 [NSNumber numberWithBool:NO], AVLinearPCMIsNonInterleaved,
										 [NSNumber numberWithBool:YES], AVLinearPCMIsFloatKey,
										 [NSNumber numberWithBool:NO], AVLinearPCMIsBigEndianKey,
										 nil];
	AVAssetReaderAudioMixOutput* audioOutput = [AVAssetReaderAudioMixOutput assetReaderAudioMixOutputWithAudioTracks:@[audioTrack] audioSettings:audioOutputSettings];
	
	AVAssetReader* reader = [AVAssetReader assetReaderWithAsset:asset error:nil];
	
	if (startTime > 0) {
		CMTimeRange timeRange = CMTimeRangeMake(CMTimeMake(startTime * 1000, 1000), kCMTimePositiveInfinity);
		reader.timeRange = timeRange;
	}
	
	[reader addOutput:videoOutput];
	[reader addOutput:audioOutput];
	
	assetReader = reader;
	videoTrackOutput = videoOutput;
	audioTrackOutput = audioOutput;
	
	if (myWidth < 0) myWidth = [videoTrack naturalSize].width;
	if (myHeight < 0) myHeight = [videoTrack naturalSize].height;
	int framerate = [videoTrack nominalFrameRate];
	printf("Framerate: %i\n", framerate);
	next = videoStart;
	audioTime = videoStart * 44100;
}

Video::~Video() {
	stop();
}

void iosPlayVideoSoundStream(VideoSoundStream* video);
void iosStopVideoSoundStream();

void Video::play() {
	AVAssetReader* reader = assetReader;
	[reader startReading];
	
	sound = new VideoSoundStream(2, 44100);
	//Mixer::play(sound);
	iosPlayVideoSoundStream(sound);
	
	playing = true;
	start = System::time() - videoStart;
}

void Video::pause() {
	playing = false;
	if (sound != nullptr) {
		//Mixer::stop(sound);
        iosStopVideoSoundStream();
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
		AVAssetReaderTrackOutput* videoOutput = videoTrackOutput;
		CMSampleBufferRef buffer = [videoOutput copyNextSampleBuffer];
		if (!buffer) {
			AVAssetReader* reader = assetReader;
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
			image = new Texture(width(), height(), Image::RGBA32, false);
		}
		
		if (pixelBuffer != NULL) {
			CVPixelBufferLockBaseAddress(pixelBuffer, 0);
			image->upload((u8*)CVPixelBufferGetBaseAddress(pixelBuffer));
			CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
		}
		CFRelease(buffer);
	}
	
	{
		AVAssetReaderAudioMixOutput* audioOutput = audioTrackOutput;
		while (audioTime / 44100.0 < next + 0.1) {
			CMSampleBufferRef buffer = [audioOutput copyNextSampleBuffer];
			if (!buffer) return;
			CMItemCount numSamplesInBuffer = CMSampleBufferGetNumSamples(buffer);
			AudioBufferList audioBufferList;
			CMBlockBufferRef blockBufferOut = nil;
			CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(buffer, NULL, &audioBufferList, sizeof(audioBufferList), NULL, NULL, kCMSampleBufferFlag_AudioBufferList_Assure16ByteAlignment, &blockBufferOut);
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

Texture* Video::currentImage() {
	update(System::time());
	return image;
}
