package com.ktxsoftware.kore;

import android.app.Activity;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;

public class KoreActivity extends Activity {
	@Override
	protected void onCreate(Bundle state) {
		super.onCreate(state);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(new KoreView(this));
		int bufferSize = AudioTrack.getMinBufferSize(44100,  AudioFormat.CHANNEL_OUT_STEREO,  AudioFormat.ENCODING_PCM_16BIT);

		final AudioTrack audio = new AudioTrack(AudioManager.STREAM_MUSIC, 44100, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT, bufferSize, AudioTrack.MODE_STREAM);
		audio.play();

		Runnable audioThread = new Runnable() {
			public void run() {
				Thread.currentThread().setPriority(Thread.MIN_PRIORITY);
				byte[] audioBuffer = new byte[1000];
				for (;;) {
					KoreLib.writeAudio(audioBuffer, audioBuffer.length);
					audio.write(audioBuffer, 0, audioBuffer.length);
				}
			}
		};
		new Thread(audioThread).start();
	}
}
