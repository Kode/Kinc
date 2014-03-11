package com.ktxsoftware.kore;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;

public class KoreRenderer implements GLSurfaceView.Renderer {
	private Context context;
	private int lastMouseX;
	private int lastMouseY;
	private boolean lastMouseButton;
	
	public KoreRenderer(Context context) {
		this.context = context;
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
		}
		
		if (mouseButton != lastMouseButton) {
			if (mouseButton) KoreLib.touchDown(mouseX, mouseY);
			else KoreLib.touchUp(mouseX, mouseY);
		}
		else if (mouseX != lastMouseX || mouseY != lastMouseY) {
			KoreLib.touchMove(mouseX, mouseY);
		}		

		lastMouseX = mouseX;
		lastMouseY = mouseY;
		lastMouseButton = mouseButton;
		
		KoreLib.step();
	}

	public void onSurfaceChanged(GL10 gl, int width, int height) {
		KoreLib.init(width, height, context.getApplicationInfo().sourceDir, context.getFilesDir().toString());
	}
}
