#include "pch.h"
#include "SoundOgg.h"
#include <cmath>

using namespace std;
using namespace Kore;

static void affirm(bool) { }
static void affirm(HRESULT) { }

CDirectSoundOgg::CDirectSoundOgg(const char* filename, LPDIRECTSOUND8& pDirectSound) :
	firstHalfPlaying(true),
	m_pBuffer(nullptr),
	m_pPlayMarker(nullptr),
	m_hFirstHalfEvent(0),
	m_hSecondHalfEvent(0),
	m_hPlayThread(0),
	m_bPlayThreadActive(false),
	m_strFilename(filename),
	m_hStopPlaybackEvent(0),
	m_bPlaybackDone(false),
	m_pDirectSound(pDirectSound),
	m_volume(1.0f),
	m_start_time(0.0f),
	vorbis(nullptr),
	vorbis_check(nullptr),
	channels(0),
	paused(false) {

	InitializeCriticalSection(&m_criticalSection);
}

void CDirectSoundOgg::Allocate() {
	FILE* fileHandle = fopen(GetFileName(), "rb");
	if (!fileHandle) {
		char errormsg[100];
		strcpy(errormsg, GetFileName());
		strcat(errormsg, " not found.");
		//throw Exception(errormsg);
	}

	vorbis = stb_vorbis_open_file(fileHandle, 0, nullptr, nullptr);
	vorbis_check = vorbis;
	stb_vorbis_info vorbisInfo = stb_vorbis_get_info(vorbis);
	channels = vorbisInfo.channels;

	/*if ((ov_open(fileHandle, &m_vorbisFile, nullptr, 0)) != 0) {
		fclose(fileHandle);
		throw Kt::Exception(Kt::Text("Error opening ") + GetFileName().c_str());
	}

	vorbis_info *vorbisInfo = ov_info(&m_vorbisFile, -1);*/

	WAVEFORMATEX	    waveFormat;
	memset(&waveFormat, 0, sizeof(waveFormat));
	waveFormat.cbSize			= sizeof(waveFormat);									//how big this structure is
	waveFormat.nChannels		= vorbisInfo.channels;									//how many channelse the OGG contains
	waveFormat.wBitsPerSample	= 16;													//always 16 in OGG
	waveFormat.nSamplesPerSec	= vorbisInfo.sample_rate;								//sampling rate (11 Khz, 22 Khz, 44 KHz, etc.)
	waveFormat.nAvgBytesPerSec	= waveFormat.nSamplesPerSec * waveFormat.nChannels * 2;	//average bytes per second
	waveFormat.nBlockAlign		= 2 * waveFormat.nChannels;								//what block boundaries exist
	waveFormat.wFormatTag		= 1;													//always 1 in OGG

	DSBUFFERDESC dsBufferDescription;
	memset(&dsBufferDescription, 0, sizeof(dsBufferDescription));
	dsBufferDescription.dwSize			= sizeof(dsBufferDescription);		//how big this structure is
	dsBufferDescription.lpwfxFormat		= &waveFormat;						//information about the sound that the buffer will contain
	dsBufferDescription.dwBufferBytes	= BUFFER_HALF_SIZE * 2;				//total buffer size = 2 * half size
	dsBufferDescription.dwFlags			= DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME;		//buffer must support notifications

	LPDIRECTSOUNDBUFFER pTempBuffer = nullptr;

	affirm(m_pDirectSound->CreateSoundBuffer(&dsBufferDescription, &pTempBuffer, NULL));
	affirm(pTempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&m_pBuffer));

	SAFE_RELEASE(pTempBuffer);

	affirm(m_pBuffer->QueryInterface(IID_IDirectSoundNotify8, (void**)&m_pPlayMarker));

	DSBPOSITIONNOTIFY notificationMarker[2];
	m_hFirstHalfEvent = CreateEvent(NULL, false, false, nullptr);
	m_hSecondHalfEvent = CreateEvent(NULL, false, false, nullptr);

	notificationMarker[0].dwOffset = 0;
	notificationMarker[0].hEventNotify = m_hFirstHalfEvent;

	notificationMarker[1].dwOffset = BUFFER_HALF_SIZE;
	notificationMarker[1].hEventNotify = m_hSecondHalfEvent;

	affirm(m_pPlayMarker->SetNotificationPositions(2, notificationMarker));

	m_hStopPlaybackEvent = CreateEvent(NULL, false, false, nullptr);

	Fill(true);
}

CDirectSoundOgg::~CDirectSoundOgg() {
	affirm(vorbis == vorbis_check);
	if (IsPlaying()) Stop();
	DeleteCriticalSection(&m_criticalSection);
}

void CDirectSoundOgg::Cleanup() {
	affirm(vorbis == vorbis_check);
	stb_vorbis_close(vorbis);
	//ov_clear(&m_vorbisFile);
	SAFE_RELEASE(m_pPlayMarker);
	SAFE_CLOSE_HANDLE(m_hFirstHalfEvent);
	SAFE_CLOSE_HANDLE(m_hSecondHalfEvent);
	SAFE_CLOSE_HANDLE(m_hStopPlaybackEvent);
	SAFE_RELEASE(m_pBuffer);
}

bool CDirectSoundOgg::Fill(const bool firstHalf) {
	LPVOID	pFirstSegment;							//holds a pointer to the first segment that we lock
	DWORD	nFirstSegmentSize = 0;					//holds how big the first segment is
	LPVOID	pSecondSegment;							//holds a pointer to the second segment that we lock
	DWORD	nSecondSegmentSize = 0;					//holds how big the second segment is

	if (FAILED(m_pBuffer->Lock(
		(firstHalf ? 0 : BUFFER_HALF_SIZE),			//if we are locking the first half, lock from 0, otherwise lock from BUFFER_HALF_SIZE
		BUFFER_HALF_SIZE,							//how big a chunk of the buffer to block
		&pFirstSegment,								//pointer that will receive the locked segment start address
		&nFirstSegmentSize,							//will return how big the first segment is (should always be BUFFER_HALF_SIZE)
		&pSecondSegment,							//pointer that will receive the second locked segment start address (in case of wrapping)
		&nSecondSegmentSize,						//how big a chunk we wrapped with (in case of wrapping)
		0											//flags: no extra settings
	))) {
		return false;
	}

	assert(nFirstSegmentSize == BUFFER_HALF_SIZE);

	unsigned int nBytesReadSoFar	= 0; //keep track of how many bytes we have read so far
	long nBytesReadThisTime			= 1; //keep track of how many bytes we read per ov_read invokation (1 to ensure that while loop is entered below)
	int nBitStream					= 0; //used to specify logical bitstream 0

	while(nBytesReadSoFar < BUFFER_HALF_SIZE) {
		//nBytesReadThisTime = ov_read(
		//	&m_vorbisFile,								//what file to read from
		//	(char*)pFirstSegment + nBytesReadSoFar,		//where to put the decoded data
		//	BUFFER_HALF_SIZE - nBytesReadSoFar,			//how much data to read
		//	0,											//0 specifies little endian decoding mode
		//	2,											//2 specifies 16-bit samples
		//	1,											//1 specifies signed data
		//	&nBitStream
		//);

		affirm(vorbis == vorbis_check);

		//
		uint num_shorts = (BUFFER_HALF_SIZE - nBytesReadSoFar) / 2;
		if (paused) {
			s16 *buf = (s16*)((char*)pFirstSegment + nBytesReadSoFar);
			for (uint i = 0; i < num_shorts; i++) buf[i] = 0;
			nBytesReadThisTime = num_shorts * 2 * 2;
		}
		else {
			nBytesReadThisTime = stb_vorbis_get_samples_short_interleaved(vorbis, channels, (short*)((char*)pFirstSegment + nBytesReadSoFar), num_shorts) * 2 * 2;

/*			FILE *dump = fopen("d:\\music_dump.wav", "ab");
			if (dump) {
				fwrite((short*)((char*)pFirstSegment + nBytesReadSoFar), nBytesReadThisTime, 1, dump);
				fclose(dump);
			}*/
		}

		nBytesReadSoFar += nBytesReadThisTime;

		affirm(vorbis == vorbis_check);

		if(nBytesReadThisTime == 0) {
			if(m_bLooping) {
				//ov_time_seek(&m_vorbisFile, 0);
				stb_vorbis_seek_start(vorbis);
			}
			else {
				//fill with silence
				for(unsigned int i = nBytesReadSoFar; i < BUFFER_HALF_SIZE; i++) {
					//silence = 0 in 16 bit sampled data (which OGG always is)
					*((char*)pFirstSegment + i) = 0;
				}
				m_bPlaybackDone = true;
				nBytesReadSoFar = BUFFER_HALF_SIZE;
			}
		}
	}
	affirm(vorbis == vorbis_check);
	m_pBuffer->Unlock(pFirstSegment, nFirstSegmentSize, pSecondSegment, nSecondSegmentSize);
	affirm(vorbis == vorbis_check);
	return true;
}

void CDirectSoundOgg::updatePlayParams() {
	//int v = Kt::LinearToDirectSoundLogVol(m_volume);//DSBVOLUME_MIN;
	//HRESULT res = m_pBuffer->SetVolume(v);
	//lf("%d    %d %d %d %d\n", res, DSERR_GENERIC , DSERR_CONTROLUNAVAIL , DSERR_INVALIDPARAM , DSERR_PRIOLEVELNEEDED );
}

bool CDirectSoundOgg::IsPlaying() {
	return GetPlayThreadActive();
}

void CDirectSoundOgg::Stop() {
	affirm(vorbis == vorbis_check);
	if (!IsPlaying()) return;
	affirm(vorbis == vorbis_check);
	SetEvent(m_hStopPlaybackEvent);
	if (WaitForSingleObject(m_hPlayThread, PLAY_THREAD_WAIT_PERIOD_MS) == WAIT_ABANDONED) {
		//throw Kore::Exception("CDirectSoundOgg::Stop(): Player thread could not be stopped. It's terminted.");
		TerminateThread(m_hPlayThread, 1);
		SetPlayThreadActive(false);
		Cleanup();
		//TODO: You should report this error here somehow
	}
	affirm(vorbis == vorbis_check);
	SetPlayThreadActive(false);
}

void CDirectSoundOgg::pause() {
	if (!IsPlaying()) return;
	paused = true;
}

void CDirectSoundOgg::unpause() {
	if (!IsPlaying()) return;
	paused = false;
}

void CDirectSoundOgg::seekToApproximately(float pos) {
	float length_in_samples = (float)stb_vorbis_stream_length_in_samples(vorbis);
	float length_in_seconds = stb_vorbis_stream_length_in_seconds(vorbis);
	// Für den Fall, daß die Position außerhalb des Bereichs ist:
	if (m_bLooping) {
		pos = std::fmod(pos, length_in_seconds);
	}
	else {
		if (pos >= length_in_seconds) pos = length_in_seconds;
	}
	//
	float v = pos * length_in_samples / length_in_seconds;
	stb_vorbis_seek(vorbis, (uint)v);
}

void CDirectSoundOgg::Play(const bool looping, const float music_start_time) {
	if (IsPlaying()) Stop();
	SetPlayThreadActive(true);
	m_bLooping = looping;
	m_start_time = music_start_time;
	unsigned int nThreadID;
	m_hPlayThread = (HANDLE)_beginthreadex(NULL, 0, CDirectSoundOgg::PlayingThread, this, 0, &nThreadID);
}

void CDirectSoundOgg::setVolume(float vol) {
	m_volume = vol;
}

const char* CDirectSoundOgg::GetFileName() {
	/*EnterCriticalSection(&m_criticalSection);
	std::string strFilename = m_strFilename;
	LeaveCriticalSection(&m_criticalSection);
	return strFilename;*/
	return m_strFilename;
}

unsigned int CDirectSoundOgg::PlayingThread(LPVOID lpParam) {
	CDirectSoundOgg *oggInstance = static_cast<CDirectSoundOgg*>(lpParam);
	//try {
		oggInstance->Allocate();
			// Loading the file should be done when the sound is loaded. But it's not
			// a complete load, it's just a touch of the file, and it's in its own
			// thread. It should be fine.
	//}
	//catch (Kore::Exception) {
	//	oggInstance->Cleanup();
	//	return 1;
	//}

	stb_vorbis *vorbis_check = oggInstance->vorbis;

	oggInstance->updatePlayParams();

	oggInstance->seekToApproximately(oggInstance->m_start_time);

	bool bErrorOccured = false;		//assume everything will go ok
	oggInstance->m_bPlaybackDone = false;
	oggInstance->m_pBuffer->Play(0, 0, DSBPLAY_LOOPING);
	bool bPlayingFirstHalf = true;
	HANDLE stlvEventObjects[3];
	stlvEventObjects[0] = oggInstance->m_hFirstHalfEvent;			//push event object for reaching first half
	stlvEventObjects[1] = oggInstance->m_hSecondHalfEvent;		//push event object for reaching second half
	stlvEventObjects[2] = oggInstance->m_hStopPlaybackEvent;		//push event object used to signal for playback stop

	bool	bContinuePlaying = true;		//used to keep track of when to stop the while loop
	bool	bPlaybackDoneProcessed = false;	//used ot only process m_bPlaybackDone once
	int		nStopAtNextHalf = 0;			//0 signals "do not stop", 1 signals "stop at first half", 2 signals "stop at second half"

	while (bContinuePlaying && (!bErrorOccured)) {
		affirm(oggInstance->vorbis == vorbis_check);

		switch(WaitForMultipleObjects(3, &(stlvEventObjects[0]), FALSE, INFINITE)) {
			case WAIT_OBJECT_0:
				if (nStopAtNextHalf == 1) {
					bContinuePlaying = false;
					break;
				}
				
				oggInstance->updatePlayParams();

				if (!(oggInstance->Fill(false))) bErrorOccured = true;
				if ((oggInstance->m_bPlaybackDone) && (!bPlaybackDoneProcessed)) {
					nStopAtNextHalf = 1;
					bPlaybackDoneProcessed = true;
				}
				break;
			case WAIT_OBJECT_0 + 1:
				if (nStopAtNextHalf == 2) {
					bContinuePlaying = false;
					break;
				}

				oggInstance->updatePlayParams();

				if (!(oggInstance->Fill(true))) bErrorOccured = true;
				if ((oggInstance->m_bPlaybackDone) && (!bPlaybackDoneProcessed)) {
					nStopAtNextHalf = 2;
					bPlaybackDoneProcessed = true;
				}
				break;
			case WAIT_OBJECT_0 + 2:
				bContinuePlaying = false;
				break;
		}
	}
	affirm(oggInstance->vorbis == vorbis_check);
	oggInstance->m_pBuffer->Stop();
	affirm(oggInstance->vorbis == vorbis_check);
	oggInstance->Cleanup();
	affirm(oggInstance->vorbis == vorbis_check);
	oggInstance->SetPlayThreadActive(false);
	affirm(oggInstance->vorbis == vorbis_check);

	return (bErrorOccured ? 1 : 0);
}

bool CDirectSoundOgg::GetPlayThreadActive() {
	EnterCriticalSection(&m_criticalSection);
	bool bActive = m_bPlayThreadActive;
	LeaveCriticalSection(&m_criticalSection);
	return bActive;
}

void CDirectSoundOgg::SetPlayThreadActive(bool bActive) {
	EnterCriticalSection(&m_criticalSection);
	m_bPlayThreadActive = bActive;
	LeaveCriticalSection(&m_criticalSection);
}
