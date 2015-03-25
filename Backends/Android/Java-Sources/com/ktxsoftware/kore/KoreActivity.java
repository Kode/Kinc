package com.ktxsoftware.kore;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.opengl.GLES20;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.Window;
import android.view.WindowManager;

import com.google.vrtoolkit.cardboard.sensors.MagnetSensor;
import com.google.vrtoolkit.cardboard.sensors.MagnetSensor.OnCardboardTriggerListener;
import com.google.vrtoolkit.cardboard.sensors.NfcSensor;
import com.google.vrtoolkit.cardboard.sensors.NfcSensor.OnCardboardNfcListener;
import com.google.vrtoolkit.cardboard.sensors.HeadTracker;
import com.google.vrtoolkit.cardboard.CardboardDeviceParams;
import com.google.vrtoolkit.cardboard.DistortionRenderer;
import com.google.vrtoolkit.cardboard.HeadMountedDisplay;
import com.google.vrtoolkit.cardboard.HeadMountedDisplayManager;
import com.google.vrtoolkit.cardboard.FieldOfView;
import com.google.vrtoolkit.cardboard.ScreenParams;
import com.ktxsoftware.kore.KoreTouchEvent;


public class KoreActivity extends Activity implements SensorEventListener, OnCardboardTriggerListener,  OnCardboardNfcListener {
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
	
	private MagnetSensor mMagnetSensor;
	private NfcSensor mNfcSensor;
	private DistortionRenderer mDistortion;
	private HeadMountedDisplay mHmd;
	private FieldOfView mFov;
	private float mDistance;
	
	private CardboardDeviceParams mParams;
	
	private HeadMountedDisplayManager hmdManager;
	
	public HeadTracker mHeadTracker;
	
	
	
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
		
		// Cardboard setup
		mMagnetSensor = new MagnetSensor(this);
		mMagnetSensor.setOnCardboardTriggerListener(this);
		

		mNfcSensor = NfcSensor.getInstance(this);
		mNfcSensor.addOnCardboardNfcListener(this);
		
		mNfcSensor.onNfcIntent(getIntent());
		
		
		
		WindowManager windowManager = (WindowManager)getApplicationContext().getSystemService("window");
		
		ScreenParams screenParams = new ScreenParams(windowManager.getDefaultDisplay());
		Log.d("FM", screenParams.toString());
		
		mParams = new CardboardDeviceParams();
		
		
		this.hmdManager = new HeadMountedDisplayManager(this);

		
		mHmd = hmdManager.getHeadMountedDisplay();
		
		
		
		
		
		mDistortion = new DistortionRenderer();
		mDistortion.setTextureFormat(GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE);
		mDistortion.setRestoreGLStateEnabled(true);

		mDistance = 0.1f;
		
		Log.d("FM", mHmd.getCardboardDeviceParams().getDistortion().toString());

		/*float fov = (float) (90.0f * Math.PI / 180.0f);
		float front = 1.0f;
		float back = 100.0f;
		float aspect = 1;

	    float tangent = (float) Math.tan(fov / 2.0f);               // tangent of half vertical fov
	    float height = front * tangent;                 // half height of near plane
	    float width = height * aspect;                  // half width of near plane */

		mFov = new FieldOfView();
		float angle = 45.0f;
		mFov.setBottom(angle);
		mFov.setLeft(angle);
		mFov.setRight(angle);
		mFov.setTop(angle);
		
		// Log.d("FM", "Width: " + mHmd.getScreenParams().getWidth());
		
		mDistortion.onFovChanged(mHmd, mFov, mFov, mDistance);
		
		
		
		mHeadTracker = HeadTracker.createFromContext(this);
		mHeadTracker.startTracking();
		
		
	}
	
	
	public void DistortionBeforeFrame() {
		mDistortion.beforeDrawFrame();
	}
	
	public void DistortionAfterFrame() {
		mDistortion.afterDrawFrame();
	}
	
	public void DistortTexture(int texId) {
		mDistortion.undistortTexture(texId);
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
		
		mMagnetSensor.start();
	    mNfcSensor.onResume(this);
	    
	    // TOD: Set all the events correctly.
	}
	
	@Override
	protected void onStop() {
		super.onStop();
		
		mMagnetSensor.stop();
		mNfcSensor.onPause(this);
	}
	
	@Override
	protected void onRestart() {
		super.onRestart();
		
		mMagnetSensor.start();
		mNfcSensor.onResume(this);
		
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
	@Override
	public void onInsertedIntoCardboard(CardboardDeviceParams paramCardboardDeviceParams) {
		// TODO: Update the params
		hmdManager.updateCardboardDeviceParams(paramCardboardDeviceParams);
		mHmd = hmdManager.getHeadMountedDisplay();
		mDistortion.onFovChanged(mHmd, mFov, mFov, mDistance);
		Log.d("FM", "Inserted into cardboard");
	}

	@Override
	public void onRemovedFromCardboard() {
		
	}
	
	protected void onNfcIntent(Intent intent)
	{
		mNfcSensor.onNfcIntent(intent);
	}
	
	@Override
	public void onCardboardTrigger() {
		// Emulate a touch using the cardboard trigger
		KoreTouchEvent down = new KoreTouchEvent(0, 0, 0, 0);
		KoreTouchEvent up = new KoreTouchEvent(0, 0, 0, 2);
		synchronized(KoreView.inputLock) {
			KoreView.touchEvents.add(down);
			KoreView.touchEvents.add(up);
		}

	}
	
}
