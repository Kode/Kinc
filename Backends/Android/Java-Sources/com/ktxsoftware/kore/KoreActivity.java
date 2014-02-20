package com.ktxsoftware.kore;

import android.app.Activity;
import android.content.res.Configuration;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;

public class KoreActivity extends Activity {
	public volatile static boolean paused = true;
	private AudioTrack audio;
	
	@Override
	protected void onCreate(Bundle state) {
		super.onCreate(state);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(new KoreView(this));
		int bufferSize = AudioTrack.getMinBufferSize(44100,  AudioFormat.CHANNEL_OUT_STEREO,  AudioFormat.ENCODING_PCM_16BIT);
		audio = new AudioTrack(AudioManager.STREAM_MUSIC, 44100, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT, bufferSize, AudioTrack.MODE_STREAM);
	}
	
	@Override
	protected void onPause() {
		super.onPause();
		paused = true;
		audio.pause();
		audio.flush();
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		paused = false;
		audio.play();
		Runnable audioRunnable = new Runnable() {
			public void run() {
				Thread.currentThread().setPriority(Thread.MIN_PRIORITY);
				byte[] audioBuffer = new byte[1000];
				for (;;) {
					if (paused) return;
					KoreLib.writeAudio(audioBuffer, audioBuffer.length);
					audio.write(audioBuffer, 0, audioBuffer.length);
				}
			}
		};
		new Thread(audioRunnable).start();
	}
	
	@Override
	protected void onStop() {
		super.onStop();
		
	}
	
	@Override
	protected void onRestart() {
		super.onRestart();
		
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		
	}
	
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
	}
}
