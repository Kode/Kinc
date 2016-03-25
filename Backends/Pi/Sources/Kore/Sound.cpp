#include "pch.h"
#include <Kore/Audio/Audio.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <alsa/asoundlib.h>
#include <pthread.h>

//apt-get install libasound2-dev

using namespace Kore;

namespace {
    pthread_t threadid;
    bool audioRunning = false;
    snd_pcm_t* playback_handle;
    const int bufferSize = 4096 * 4;
	short buf[bufferSize];

	void copySample(void* buffer) {
		float value = *(float*)&Audio::buffer.data[Audio::buffer.readLocation];
		Audio::buffer.readLocation += 4;
		if (Audio::buffer.readLocation >= Audio::buffer.dataSize) Audio::buffer.readLocation = 0;
		if (value != 0) {
            int a = 3;
            ++a;
		}
		*(s16*)buffer = static_cast<s16>(value * 32767);
	}

	int playback_callback(snd_pcm_sframes_t nframes) {
		int err = 0;
		if (Kore::Audio::audioCallback != nullptr) {
            Kore::Audio::audioCallback(nframes * 2);
            int ni = 0;
            while (ni < nframes) {
                int i = 0;
                for (; ni < nframes && i < bufferSize / 2; ++i, ++ni) {
                    copySample(&buf[i * 2]);
                    copySample(&buf[i * 2 + 1]);
                }
                int err2;
                if ((err2 = snd_pcm_writei(playback_handle, buf, i)) < 0) {
                    //EPIPE is an underrun
                    fprintf (stderr, "write failed (%s)\n", snd_strerror (err2));
                    int recovered = snd_pcm_recover(playback_handle, err2, 1);
                    printf("Recovered: %d\n", recovered);
                }
                else err += err2;
            }
        }
		return err;
	}

	void* doAudio(void *arg) {
        snd_pcm_hw_params_t *hw_params;
        snd_pcm_sw_params_t *sw_params;
        snd_pcm_sframes_t frames_to_deliver;
        //int nfds;
        int err;
        //struct pollfd *pfds;

        if ((err = snd_pcm_open (&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
            fprintf (stderr, "cannot open audio device default (%s)\n",
                 snd_strerror (err));
            exit (1);
        }

        if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
            fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
                 snd_strerror (err));
            exit (1);
        }

        if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
            fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
                 snd_strerror (err));
            exit (1);
        }

        if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
            fprintf (stderr, "cannot set access type (%s)\n",
                 snd_strerror (err));
            exit (1);
        }

        if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
            fprintf (stderr, "cannot set sample format (%s)\n",
                 snd_strerror (err));
            exit (1);
        }

        uint rate = 44100;
        int dir = 0;
        if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &rate, &dir)) < 0) {
            fprintf (stderr, "cannot set sample rate (%s)\n",
                 snd_strerror (err));
            exit (1);
        }

        if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, 2)) < 0) {
            fprintf (stderr, "cannot set channel count (%s)\n",
                 snd_strerror (err));
            exit (1);
        }

        snd_pcm_uframes_t bufferSize = rate / 8;
        if (((err = snd_pcm_hw_params_set_buffer_size(playback_handle, hw_params, bufferSize)) < 0 && (snd_pcm_hw_params_set_buffer_size_near(playback_handle, hw_params, &bufferSize)) < 0)) {
            fprintf (stderr, "cannot set buffer size (%s)\n",
                 snd_strerror (err));
            exit (1);
        }

        if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
            fprintf (stderr, "cannot set parameters (%s)\n",
                 snd_strerror (err));
            exit (1);
        }

        snd_pcm_hw_params_free (hw_params);

        /* tell ALSA to wake us up whenever 4096 or more frames
           of playback data can be delivered. Also, tell
           ALSA that we'll start the device ourselves.
        */

        if ((err = snd_pcm_sw_params_malloc (&sw_params)) < 0) {
            fprintf (stderr, "cannot allocate software parameters structure (%s)\n",
                 snd_strerror (err));
            exit (1);
        }
        if ((err = snd_pcm_sw_params_current (playback_handle, sw_params)) < 0) {
            fprintf (stderr, "cannot initialize software parameters structure (%s)\n",
                 snd_strerror (err));
            exit (1);
        }
        if ((err = snd_pcm_sw_params_set_avail_min (playback_handle, sw_params, 4096)) < 0) {
            fprintf (stderr, "cannot set minimum available count (%s)\n",
                 snd_strerror (err));
            exit (1);
        }
        if ((err = snd_pcm_sw_params_set_start_threshold (playback_handle, sw_params, 0U)) < 0) {
            fprintf (stderr, "cannot set start mode (%s)\n",
                 snd_strerror (err));
            exit (1);
        }
        if ((err = snd_pcm_sw_params (playback_handle, sw_params)) < 0) {
            fprintf (stderr, "cannot set software parameters (%s)\n",
                 snd_strerror (err));
            exit (1);
        }

        /* the interface will interrupt the kernel every 4096 frames, and ALSA
           will wake up this program very soon after that.
        */

        if ((err = snd_pcm_prepare (playback_handle)) < 0) {
            fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                 snd_strerror (err));
            exit (1);
        }

        while (audioRunning) {

            /* wait till the interface is ready for data, or 1 second
               has elapsed.
            */

            if ((err = snd_pcm_wait (playback_handle, 1000)) < 0) {
                    fprintf (stderr, "poll failed (%s)\n", strerror (errno));
                    break;
            }

            /* find out how much space is available for playback data */

            if ((frames_to_deliver = snd_pcm_avail_update (playback_handle)) < 0) {
                if (frames_to_deliver == -EPIPE) {
                    fprintf (stderr, "an xrun occured\n");
                    break;
                } else {
                    fprintf (stderr, "unknown ALSA avail update return value (%i)\n",
                         (int)frames_to_deliver);
                    break;
                }
            }

            //frames_to_deliver = frames_to_deliver > 4096 ? 4096 : frames_to_deliver;

            /* deliver the data */

            if (playback_callback (frames_to_deliver) != frames_to_deliver) {
                    fprintf (stderr, "playback callback failed\n");
                //break; // Do not break so we can recover from errors.
            }
        }

        snd_pcm_close (playback_handle);
        return nullptr;
	}
}

void Audio::init() {
    buffer.readLocation = 0;
	buffer.writeLocation = 0;
	buffer.dataSize = 128 * 1024;
	buffer.data = new u8[buffer.dataSize];

    audioRunning = true;
    pthread_create(&threadid, nullptr, &doAudio, nullptr);
}

void Audio::update() {

}

void Audio::shutdown() {
    audioRunning = false;
}
