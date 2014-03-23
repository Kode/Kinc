package com.ktxsoftware.kore;

import android.app.Activity;
import android.content.Context;
import android.content.res.Configuration;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;

public class KoreActivity extends Activity implements SensorEventListener {
	public volatile static boolean paused = true;
	private AudioTrack audio;
	private Thread audioThread;
	private int bufferSize;
	
	public static Object sensorLock = new Object();
	private SensorManager sensorManager;
	private Sensor accelerometer, gyro;
	public static float accelerometerX, accelerometerY, accelerometerZ;
	public static float gyroX, gyroY, gyroZ;
	
	private static KoreActivity instance;
	
	public static KoreActivity getInstance() {
		return instance;
	}
	
	@Override
	protected void onCreate(Bundle state) {
		super.onCreate(state);
		instance = this;
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(new KoreView(this));
		bufferSize = AudioTrack.getMinBufferSize(44100, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT) * 2;
		audio = new AudioTrack(AudioManager.STREAM_MUSIC, 44100, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT, bufferSize, AudioTrack.MODE_STREAM);
		
		sensorManager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);
		accelerometer = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
		gyro = sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
	}
	
	@Override
	protected void onPause() {
		super.onPause();
		sensorManager.unregisterListener(this);
		paused = true;
		audio.pause();
		audio.flush();
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		
		sensorManager.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_NORMAL);
		sensorManager.registerListener(this, gyro, SensorManager.SENSOR_DELAY_NORMAL);
		
		if (audioThread != null) {
			try {
				audioThread.join();
			}
			catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		paused = false;
		audio.play();
		Runnable audioRunnable = new Runnable() {
			public void run() {
				Thread.currentThread().setPriority(Thread.MIN_PRIORITY);
				byte[] audioBuffer = new byte[bufferSize / 2];
				for (;;) {
					if (paused) return;
					KoreLib.writeAudio(audioBuffer, audioBuffer.length);
					int written = 0;
					while (written < audioBuffer.length) {
						written += audio.write(audioBuffer, written, audioBuffer.length);
					}
				}
			}
		};
		audioThread = new Thread(audioRunnable);
		audioThread.start();
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

	@Override
	public void onAccuracyChanged(Sensor sensor, int value) {
		
	}

	@Override
	public void onSensorChanged(SensorEvent e) {
		if (e.sensor == accelerometer) {
			synchronized(sensorLock) {
				accelerometerX = e.values[0];
				accelerometerY = e.values[1];
				accelerometerZ = e.values[2];
			}
		}
		else if (e.sensor == gyro) {
			synchronized(sensorLock) {
				gyroX = e.values[0];
				gyroY = e.values[1];
				gyroZ = e.values[2];
			}
		}
	}
}
