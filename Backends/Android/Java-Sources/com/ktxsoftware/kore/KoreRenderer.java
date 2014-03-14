package com.ktxsoftware.kore;

import java.util.ArrayList;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.KeyCharacterMap;

public class KoreRenderer implements GLSurfaceView.Renderer {
	private Context context;
	private int lastMouseX;
	private int lastMouseY;
	private boolean lastMouseButton;
	private boolean keyboardShown = false;
	private ArrayList<KoreKeyEvent> keyEvents;
	private KeyCharacterMap keyMap;
	
	public KoreRenderer(Context context) {
		this.context = context;
		keyEvents = new ArrayList<KoreKeyEvent>();
		keyMap = KeyCharacterMap.load(-1);
	}
	
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {

	}

	public void onDrawFrame(GL10 gl) {
		int mouseX;
		int mouseY;
		boolean mouseButton;
		
		synchronized(KoreView.inputLock) {
			mouseX = KoreView.mouseX;
			mouseY = KoreView.mouseY;
			mouseButton = KoreView.mouseButton;
			keyEvents.addAll(KoreView.keyEvents);
			KoreView.keyEvents.clear();
		}
		
		if (mouseButton != lastMouseButton) {
			if (mouseButton) KoreLib.touchDown(mouseX, mouseY);
			else KoreLib.touchUp(mouseX, mouseY);
		}
		else if (mouseX != lastMouseX || mouseY != lastMouseY) {
			KoreLib.touchMove(mouseX, mouseY);
		}
		
		for (int i = 0; i < keyEvents.size(); ++i) {
			int code = keyMap.get(keyEvents.get(i).code, 0);
			System.out.println("Key: " + code);
			if (keyEvents.get(i).down) KoreLib.keyDown(code);
			else KoreLib.keyUp(code);
		}
		keyEvents.clear();

		lastMouseX = mouseX;
		lastMouseY = mouseY;
		lastMouseButton = mouseButton;
		
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
		KoreLib.init(width, height, context.getApplicationInfo().sourceDir, context.getFilesDir().toString());
	}
}
