package com.ktxsoftware.kore;

import android.app.Activity;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;

public class KoreActivity extends Activity implements AudioTrack.OnPlaybackPositionUpdateListener {
	private AudioTrack audio;
	private byte[] audioBuffer;
	
	@Override
	protected void onCreate(Bundle state) {
		super.onCreate(state);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(new KoreView(this));
		int bufferSize = AudioTrack.getMinBufferSize(44100,  AudioFormat.CHANNEL_OUT_STEREO,  AudioFormat.ENCODING_PCM_16BIT);
		audioBuffer = new byte[bufferSize / 3];
		audio = new AudioTrack(AudioManager.STREAM_MUSIC, 44100, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT, bufferSize, AudioTrack.MODE_STREAM);
		audio.setPlaybackPositionUpdateListener(this);
		audio.setPositionNotificationPeriod(bufferSize / 4 / 3);
		audio.play();
	}

	@Override
	public void onMarkerReached(AudioTrack track) {
		
	}

	@Override
	public void onPeriodicNotification(AudioTrack track) {
		KoreLib.writeAudio(audioBuffer, audioBuffer.length);
		track.write(audioBuffer, 0, audioBuffer.length);
	}
}
