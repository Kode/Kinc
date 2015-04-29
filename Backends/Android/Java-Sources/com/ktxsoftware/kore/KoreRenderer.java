package com.ktxsoftware.kore;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.text.method.MetaKeyKeyListener;
import android.view.KeyCharacterMap;

public class KoreRenderer implements GLSurfaceView.Renderer {
	private Context context;
	private boolean keyboardShown = false;
	private KeyCharacterMap keyMap;
	private KoreView view;
	
	public KoreRenderer(Context context, KoreView view) {
		this.context = context;
		this.view = view;
		keyMap = KeyCharacterMap.load(-1);
	}
	
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {

	}

	public void onDrawFrame(GL10 gl) {
		for (KoreMoviePlayer player : KoreMoviePlayer.players) {
			player.update();
		}
		
		KoreLib.step();
		
		if (KoreLib.keyboardShown()) {
			if (!keyboardShown) {
				keyboardShown = true;
				KoreActivity.getInstance().runOnUiThread(new Runnable() {
				    public void run() {
				        view.showKeyboard();
				    }
				});
			}
		}
		else {
			if (keyboardShown) {
				keyboardShown = false;
				KoreActivity.getInstance().runOnUiThread(new Runnable() {
				    public void run() {
				        view.hideKeyboard();
				    }
				});
			}
		}
	}

	public void onSurfaceChanged(GL10 gl, int width, int height) {
		KoreLib.init(width, height, context.getResources().getAssets(), context.getApplicationInfo().sourceDir, context.getFilesDir().toString());
	}
	
	public void key(int keyCode, boolean down) {
		switch (keyCode) {
		case 59: // shift
			if (down) KoreLib.keyDown(0x00000120);
			else KoreLib.keyUp(0x00000120);
			break;
		case 66: // return
			if (down) KoreLib.keyDown(0x00000104);
			else KoreLib.keyUp(0x00000104);
			break;
		case 67: // backspace
			if (down) KoreLib.keyDown(0x00000103);
			else KoreLib.keyUp(0x00000103);
			break;
		default:
			int code = keyMap.get(keyCode, MetaKeyKeyListener.META_SHIFT_ON);
			if (down) KoreLib.keyDown(code);
			else KoreLib.keyUp(code);
			break;
		}
	}
	
	public void touch(int index, int x, int y, int action) {
		KoreLib.touch(index, x, y, action);
	}
	
	public void accelerometer(float x, float y, float z) {
		KoreLib.accelerometerChanged(x, y, z);
	}
	
	public void gyro(float x, float y, float z) {
		KoreLib.gyroChanged(x, y, z);
	}
}
