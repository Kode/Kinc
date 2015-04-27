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

namespace {
	int maxSamples = 0;
}

VideoSoundStream::VideoSoundStream(int nChannels, int freq) : bufferSize(1024 * 100 * 10), bufferReadPosition(0), bufferWritePosition(0), read(0), written(0) {
	buffer = new float[bufferSize];
}

void VideoSoundStream::insertData(float* data, int nSamples) {
	//printf("Samples: %i\n", nSamples);
	if (nSamples > maxSamples) {
		printf("Samples: %i\n", nSamples);
		maxSamples = nSamples;
	}
	written += nSamples;
	for (int i = 0; i < nSamples; ++i) {
		float value = data[i];// / 32767.0;
		buffer[bufferWritePosition++] = value;
		if (bufferWritePosition >= bufferSize) {
			bufferWritePosition = 0;
			printf("buffer write back\n");
		}
	}
}

float VideoSoundStream::nextSample() {
	++read;
	if (written <= read) {
		//printf("Out of audio\n");
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
	next = 0;
	audioTime = 0;
}

Video::~Video() {
	stop();
}

void Video::play() {
	AVAssetReader* reader = assetReader;
	[reader startReading];
	
	sound = new VideoSoundStream(2, 44100);
	Mixer::play(sound);
	
	playing = true;
	start = System::time();
}

void Video::pause() {
	playing = false;
	if (sound != nullptr) {
		Mixer::stop(sound);
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
				double startTime = next;
				load(startTime);
				play();
				start -= startTime;
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
			//int width = CVPixelBufferGetWidth(pixelBuffer);
			//int height = CVPixelBufferGetHeight(pixelBuffer);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, CVPixelBufferGetBaseAddress(pixelBuffer));
			image->upload((u8*)CVPixelBufferGetBaseAddress(pixelBuffer));
			CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
		}
		CFRelease(buffer);
	}
	
	//printf("Next %f\n", next);
	
	{
		AVAssetReaderAudioMixOutput* audioOutput = audioTrackOutput;
		while (audioTime / 44100.0 < next + 1) {
			CMSampleBufferRef buffer = [audioOutput copyNextSampleBuffer];
			//audioNext = CMTimeGetSeconds(CMSampleBufferGetOutputPresentationTimeStamp(buffer));
			if (!buffer) return;
			CMItemCount numSamplesInBuffer = CMSampleBufferGetNumSamples(buffer);
			
			//audioTime += numSamplesInBuffer * 2;// / 44100.0;
			//AudioTimeStamp ts;
			//memset(&ts, 0, sizeof(AudioTimeStamp));
			//ts.mSampleTime = currentSampleTime;
			//ts.mFlags |= kAudioTimeStampSampleTimeValid;
			
			AudioBufferList audioBufferList;
	
			//audioBufferList.mNumberBuffers = 1;
			//audioBufferList.mBuffers[0].mNumberChannels = 2;
			//audioBufferList.mBuffers[0].mData = malloc(numSamplesInBuffer * sizeof(SInt16) * 2);
			//audioBufferList.mBuffers[0].mDataByteSize = numSamplesInBuffer * sizeof(SInt16) * 2;
			
			//size_t bufferListSizeNeededOut;
			CMBlockBufferRef blockBufferOut = nil;
			CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(buffer, NULL, &audioBufferList, sizeof(audioBufferList), NULL, NULL, kCMSampleBufferFlag_AudioBufferList_Assure16ByteAlignment, &blockBufferOut);
			//CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(buffer, NULL, &audioBufferList, sizeof(audioBufferList), NULL, NULL, kCMSampleBufferFlag_AudioBufferList_Assure16ByteAlignment, &buffer);
			//Kore::log(Kore::Info, "Audio buffers: %i", audioBufferList.mNumberBuffers);
			//audioTime += audioBufferList.mNumberBuffers;
			for (int bufferCount = 0; bufferCount < audioBufferList.mNumberBuffers; ++bufferCount) {
				float* samples = (float*)audioBufferList.mBuffers[bufferCount].mData;
				sound->insertData(samples, (int)numSamplesInBuffer * 2);
				audioTime += numSamplesInBuffer;
				//sound->insertData(samples, (int)numSamplesInBuffer);
				//for (int i = 0; i < numSamplesInBuffer; ++i) {
				// amplitude for the sample is samples[i], assuming you have linear pcm to start with
				//}
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
