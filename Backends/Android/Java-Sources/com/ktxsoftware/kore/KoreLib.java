package com.ktxsoftware.kore;

import android.content.res.AssetManager;

public class KoreLib {
	static {
		System.loadLibrary("Kore");
	}

	public static native void init(int width, int height, AssetManager assetManager, String apkPath, String filesDir);
	public static native void step();
	public static native void touch(int index, int x, int y, int action);
	public static native boolean keyboardShown();
	public static native boolean keyUp(int code);
	public static native boolean keyDown(int code);
	public static native void accelerometerChanged(float x, float y, float z);
	public static native void gyroChanged(float x, float y, float z);
	public static native void writeAudio(byte[] buffer, int size);
	
	public static native void onCreate();
	public static native void onStart();
	public static native void onPause();
	public static native void onResume();
	public static native void onStop();
	public static native void onRestart();
	public static native void onDestroy();
	
	// For Cardboard VR
	public static native void gaze(float x, float y, float z, float w);
}
