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

/* import com.google.vrtoolkit.cardboard.sensors.MagnetSensor;
import com.google.vrtoolkit.cardboard.sensors.MagnetSensor.OnCardboardTriggerListener;
import com.google.vrtoolkit.cardboard.sensors.NfcSensor;
import com.google.vrtoolkit.cardboard.sensors.NfcSensor.OnCardboardNfcListener;
import com.google.vrtoolkit.cardboard.sensors.HeadTracker;
import com.google.vrtoolkit.cardboard.CardboardDeviceParams;
import com.google.vrtoolkit.cardboard.DistortionRenderer;
import com.google.vrtoolkit.cardboard.HeadMountedDisplay;
import com.google.vrtoolkit.cardboard.HeadMountedDisplayManager;
import com.google.vrtoolkit.cardboard.FieldOfView;
import com.google.vrtoolkit.cardboard.ScreenParams; */

public class KoreActivity extends Activity implements SensorEventListener {//, OnCardboardTriggerListener,  OnCardboardNfcListener {
	private AudioTrack audio;
	private KoreAudioRunnable audioRunnable;
	private KoreView view;
	
	public static Object sensorLock = new Object();
	private SensorManager sensorManager;
	private Sensor accelerometer, gyro;
	
	private static KoreActivity instance;
	
/*	private MagnetSensor mMagnetSensor;
	private NfcSensor mNfcSensor;
	private DistortionRenderer mDistortion;
	private HeadMountedDisplay mHmd;
	private FieldOfView mFov;
	private float mDistance;
	
	private CardboardDeviceParams mParams;
	
	private HeadMountedDisplayManager hmdManager;
	
	public HeadTracker mHeadTracker; */
	
	
	
	public static KoreActivity getInstance() {
		return instance;
	}
	
	@Override
	protected void onCreate(Bundle state) {
		super.onCreate(state);
		instance = this;
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(view = new KoreView(this));
		
		int bufferSize = AudioTrack.getMinBufferSize(44100, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT);
		audio = new AudioTrack(AudioManager.STREAM_MUSIC, 44100, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT, bufferSize * 2, AudioTrack.MODE_STREAM);
		audioRunnable = new KoreAudioRunnable(audio, bufferSize);
		Thread audioThread = new Thread(audioRunnable);
		audioThread.start();
		
		sensorManager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);
		accelerometer = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
		gyro = sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
		
		/*
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
		
		*/
		
		view.queueEvent(new Runnable() {
			@Override
			public void run() {
				KoreLib.onCreate();
			}
		});
	}
	
	/*
	public void DistortionBeforeFrame() {
		mDistortion.beforeDrawFrame();
	}
	
	public void DistortionAfterFrame() {
		mDistortion.afterDrawFrame();
	}
	
	public void DistortTexture(int texId) {
		mDistortion.undistortTexture(texId);
	}
	*/
	
	@Override
	protected void onStart() {
		super.onStart();
		
		view.queueEvent(new Runnable() {
			@Override
			public void run() {
				KoreLib.onStart();
			}
		});
	}
	
	@Override
	protected void onPause() {
		super.onPause();
		view.onPause();
		sensorManager.unregisterListener(this);
		audioRunnable.onPause();
		audio.pause();
		audio.flush();
		
		view.queueEvent(new Runnable() {
			@Override
			public void run() {
				KoreLib.onPause();
			}
		});
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		view.onResume();
		sensorManager.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_NORMAL);
		sensorManager.registerListener(this, gyro, SensorManager.SENSOR_DELAY_NORMAL);
		
		audioRunnable.onResume();
		audio.play();
		
		// mMagnetSensor.start();
	    // mNfcSensor.onResume(this);
	    
	    // TOD: Set all the events correctly.
		
		view.queueEvent(new Runnable() {
			@Override
			public void run() {
				KoreLib.onResume();
			}
		});
	}
	
	@Override
	protected void onStop() {
		super.onStop();
		
		//mMagnetSensor.stop();
		//mNfcSensor.onPause(this);
		view.queueEvent(new Runnable() {
			@Override
			public void run() {
				KoreLib.onStop();
			}
		});
	}
	
	@Override
	protected void onRestart() {
		super.onRestart();
		
		//mMagnetSensor.start();
		//mNfcSensor.onResume(this);
		
		view.queueEvent(new Runnable() {
			@Override
			public void run() {
				KoreLib.onRestart();
			}
		});
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		
		view.queueEvent(new Runnable() {
			@Override
			public void run() {
				KoreLib.onDestroy();
			}
		});
	}
	
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
		
		/*switch (newConfig.orientation) {
		case Configuration.ORIENTATION_LANDSCAPE:
			
			break;
		case Configuration.ORIENTATION_PORTRAIT:
			
			break;
			
		}*/
	}

	@Override
	public void onAccuracyChanged(Sensor sensor, int value) {
		
	}

	@Override
	public void onSensorChanged(SensorEvent e) {
		if (e.sensor == accelerometer) {
			view.accelerometer(e.values[0], e.values[1], e.values[2]);
		}
		else if (e.sensor == gyro) {
			view.gyro(e.values[0], e.values[1], e.values[2]);
		}
	}
	
	/*
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
	
	*/
	
}
