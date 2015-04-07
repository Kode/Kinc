package com.ktxsoftware.kore;

import java.util.ArrayList;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.text.method.MetaKeyKeyListener;
import android.view.KeyCharacterMap;

public class KoreRenderer implements GLSurfaceView.Renderer {
	private Context context;
	private boolean keyboardShown = false;
	private ArrayList<KoreTouchEvent> touchEvents;
	private ArrayList<KoreKeyEvent> keyEvents;
	private KeyCharacterMap keyMap;
	//private boolean shift = false;
	private float lastAccelerometerX, lastAccelerometerY, lastAccelerometerZ;
	private float lastGyroX, lastGyroY, lastGyroZ;
	
	public KoreRenderer(Context context) {
		this.context = context;
		touchEvents = new ArrayList<KoreTouchEvent>();
		keyEvents = new ArrayList<KoreKeyEvent>();
		keyMap = KeyCharacterMap.load(-1);
	}
	
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {

	}

	public void onDrawFrame(GL10 gl) {
		for (KoreMoviePlayer player : KoreMoviePlayer.players) {
			player.update();
		}
		
		synchronized(KoreView.inputLock) {
			touchEvents.addAll(KoreView.touchEvents);
			KoreView.touchEvents.clear();
			keyEvents.addAll(KoreView.keyEvents);
			KoreView.keyEvents.clear();
		}
				
		for (int i = 0; i < touchEvents.size(); ++i) {
			KoreTouchEvent e = touchEvents.get(i);
			KoreLib.touch(e.index, e.x, e.y, e.action);
		}
		touchEvents.clear();
		
		for (int i = 0; i < keyEvents.size(); ++i) {
			KoreKeyEvent e = keyEvents.get(i);
			int original = e.code;
			if (original == 59) { // shift
				if (e.down) KoreLib.keyDown(0x00000120);
				else KoreLib.keyUp(0x00000120);
				continue;
			}
			if (original == 67) { // backspace
				if (e.down) KoreLib.keyDown(0x00000103);
				else KoreLib.keyUp(0x00000103);
				continue;
			}
			if (original == 66) { // return
				if (e.down) KoreLib.keyDown(0x00000104);
				else KoreLib.keyUp(0x00000104);
				continue;
			}
			int code = keyMap.get(original, MetaKeyKeyListener.META_SHIFT_ON);
			//System.out.println("Key: " + code + " from " + original);
			if (e.down) KoreLib.keyDown(code);
			else KoreLib.keyUp(code);
		}
		keyEvents.clear();
		
		float accelerometerX, accelerometerY, accelerometerZ;
		float gyroX, gyroY, gyroZ;
		synchronized(KoreActivity.sensorLock) {
			accelerometerX = KoreActivity.accelerometerX;
			accelerometerY = KoreActivity.accelerometerY;
			accelerometerZ = KoreActivity.accelerometerZ;
			gyroX = KoreActivity.gyroX;
			gyroY = KoreActivity.gyroY;
			gyroZ = KoreActivity.gyroZ;
		}
		if (accelerometerX != lastAccelerometerX || accelerometerY != lastAccelerometerY || accelerometerZ != lastAccelerometerZ) {
			KoreLib.accelerometerChanged(accelerometerX, accelerometerY, accelerometerZ);
			lastAccelerometerX = accelerometerX;
			lastAccelerometerY = accelerometerY;
			lastAccelerometerZ = accelerometerZ;
		}
		if (gyroX != lastGyroX || gyroY != lastGyroY || gyroZ != lastGyroZ) {
			KoreLib.gyroChanged(gyroX, gyroY, gyroZ);
			lastGyroX = gyroX;
			lastGyroY = gyroY;
			lastGyroZ = gyroZ;
		}
		
		KoreLib.step();
		
		if (KoreLib.keyboardShown()) {
			if (!keyboardShown) {
				keyboardShown = true;
				KoreActivity.getInstance().runOnUiThread(new Runnable(){
				    public void run(){
				        KoreView.getInstance().showKeyboard();
				    }
				});
			}
		}
		else {
			if (keyboardShown) {
				keyboardShown = false;
				KoreActivity.getInstance().runOnUiThread(new Runnable(){
				    public void run(){
				        KoreView.getInstance().hideKeyboard();
				    }
				});
			}
		}
	}

	public void onSurfaceChanged(GL10 gl, int width, int height) {
		KoreLib.init(width, height, context.getResources().getAssets(), context.getApplicationInfo().sourceDir, context.getFilesDir().toString());
	}
}
