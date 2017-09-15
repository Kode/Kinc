#include "pch.h"

#include <Kore/Audio3/TextToSpeech.h>
#include <ppltasks.h>

using namespace Kore;
using namespace Windows::Media::SpeechSynthesis;
using namespace Windows::Media::Core;
using namespace Windows::Media::Playback;

SpeechSynthesizer^ speechSynthesizer;
MediaPlayer^ mediaPlayer;

void TextToSpeech::Init() {
	speechSynthesizer = ref new SpeechSynthesizer();
	mediaPlayer = ref new MediaPlayer();
}

Platform::String^ CharToString(const char * char_array) {
	std::string s_str = std::string(char_array);
	std::wstring wid_str = std::wstring(s_str.begin(), s_str.end());
	const wchar_t* w_char = wid_str.c_str();
	return ref new Platform::String(w_char);
}

void TextToSpeech::SpeakText(const char* text) {
	concurrency::create_task(speechSynthesizer->SynthesizeTextToStreamAsync(CharToString(text)))
		.then([](SpeechSynthesisStream^ stream)
	{
		mediaPlayer->Source = MediaSource::CreateFromStream(stream, stream->ContentType);
		//mediaPlayer->Volume = 1;
		mediaPlayer->Play();
	});
}