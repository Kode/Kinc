#pragma once

#include	<windows.h>						//for critical section/threading support
#include	<process.h>						//for threading support
#include	<assert.h>						//for assertion support
#include	<dsound.h>						//DirectSound headers
#include "stb_vorbis.h"

#define SAFE_DELETE(p)			{ if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release(); (p)=NULL; } }
#define SAFE_CLOSE_HANDLE(h)	{ if(h) { CloseHandle(h); (h)=0; } }

//!Holds how large each half of the DirectSound buffer is
const int BUFFER_HALF_SIZE = 15 * 1024;			//15 kb, which is roughly one second of OGG sound
//!Holds how long the main thread should wait for the player thread to shut down (ms)
const int PLAY_THREAD_WAIT_PERIOD_MS = 500;



/*!

\brief Class that can stream and decompress an Ogg Vorbis file into a memory buffer and then play that buffer.



Use this class to stream long sound clips using little memory. To compress a wave file into Ogg Vorbis format, download a free encoder from

http://www.vorbis.org



CDirectSoundOgg creates a thread per object instance being played. Although the threads lie dormant 99.99% of the time while

streaming, they do consume resources. Accordingly, you should take care as to how many CDirectSoundOgg instances you have playing at the 

same time. This class has some latency before playing, as it need to set up a fair bit of details before playback can commence. 



If you call CDirectSoundOgg::Play() while the OGG is already playing, the OGG will stop and start from the beginning. 



\note A thread is only active when the instance is being played. It will not consume resources having many CDirectSoundOgg instances that are

not being played.



\note When playing on a 2.8 GHz HT Pentium 4, the play thread run by this class consumes roughly 1% CPU.



This class is covered by the following license:



=== LICENSE BEGIN ===



© 2004, Bjørn Toft Madsen, http://www.sunbeam60.net

All rights reserved.



Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:



* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

* The name Bjørn Toft Madsen or the website http://www.sunbeam60.net may not be used to endorse or promote products derived from this software without specific prior written permission.



THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



=== LICENSE END ===



This class uses Xiph.Org Foundation software, covered under the license below.



=== LICENSE BEGIN ===



© 2004, Xiph.Org Foundation



Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met: 



* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 

* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution. 

* Neither the name of the Xiph.org Foundation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission. 



This software is provided by the copyright holders and contributors “as is” and any express or implied warranties, including, but not limited to, the implied warranties of merchantability and fitness for a particular purpose are disclaimed. In no event shall the foundation or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage. 



=== LICENSE END===

*/

class CDirectSoundOgg {
private:
	const char* m_strFilename;
	LPDIRECTSOUND8 m_pDirectSound;
	float m_volume;
	float m_start_time;
	bool m_bLooping;
	bool m_bPlaybackDone;
	bool m_bPlayThreadActive;
	bool paused;
protected:
	bool GetPlayThreadActive();
	void SetPlayThreadActive(bool bActive);

	//OggVorbis_File m_vorbisFile;
	stb_vorbis* vorbis;
	int channels;
	LPDIRECTSOUNDBUFFER8 m_pBuffer;
	CRITICAL_SECTION m_criticalSection;
	HANDLE m_hPlayThread;
	bool firstHalfPlaying;
	LPDIRECTSOUNDNOTIFY8 m_pPlayMarker;
	HANDLE m_hFirstHalfEvent;
	HANDLE m_hSecondHalfEvent;
	HANDLE m_hStopPlaybackEvent;

	void Allocate();
	void Cleanup();
	bool Fill(const bool firstHalf);
	static unsigned int WINAPI PlayingThread(LPVOID lpParam);
	void updatePlayParams();
public:
	CDirectSoundOgg(const char* filename, LPDIRECTSOUND8 &pDirectSound);
	virtual ~CDirectSoundOgg();
	const char* GetFileName();
	
	void seekToApproximately(float pos);
	void Play(const bool looping = false, const float start_time = 0.0f);
	void Stop();
	void pause();
	void unpause();
	bool IsPlaying();

	stb_vorbis *vorbis_check;

	void setVolume(float vol);
};
