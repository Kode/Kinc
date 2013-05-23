#include "pch.h"
#include <Kore/Sound/Sound.h>
#include <Kore/Sound/Music.h>
#import <Foundation/Foundation.h>
#import <AVFoundation/AVAudioPlayer.h>

const char* iphonegetresourcepath();

using namespace Kore;

void Sound::init() {

}

Sound::SoundHandle::SoundHandle(const char* filename, bool loops) {
	char filepath[1001];
	strcpy(filepath, iphonegetresourcepath());
	strcat(filepath, "/Deployment/");
	strcat(filepath, filename);
	NSError* error;
	NSString* path = [[NSString alloc] initWithCString:filepath encoding:NSASCIIStringEncoding];
	NSURL* url = [NSURL fileURLWithPath:path];
	AVAudioPlayer* av_mus_player = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];
	
	[path release];
	
	if (av_mus_player != nullptr) {
		[av_mus_player prepareToPlay];
		if (loops)
			[av_mus_player setNumberOfLoops:-1];
		else
			[av_mus_player setNumberOfLoops:0];
		player = (void*)av_mus_player;
	}
	else {
		player = nullptr;
	}
}

Sound::SoundHandle::~SoundHandle() {
	if (player != nullptr) {
		AVAudioPlayer* av_mus_player = (AVAudioPlayer*)player;
		[av_mus_player stop];
		[av_mus_player release];
		player = nullptr;
	}
}

void Sound::SoundHandle::play() {
	if (player != nullptr) {
		AVAudioPlayer* av_mus_player = (AVAudioPlayer*)player;
		[av_mus_player play];
	}
}

int Sound::SoundHandle::position() {
	if (player != nullptr) {
		AVAudioPlayer* av_mus_player = (AVAudioPlayer*)player;
		return [av_mus_player currentTime];
	}
	else return 0;
}

int Sound::SoundHandle::length() {
	if (player != nullptr) {
		AVAudioPlayer* av_mus_player = (AVAudioPlayer*)player;
		return [av_mus_player duration];
	}
	else return 0;
}

void Music::play(const char* filename) {
	
}

void Music::stop() {
	
}