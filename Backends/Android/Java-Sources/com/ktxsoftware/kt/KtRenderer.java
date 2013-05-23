package com.ktxsoftware.kt;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;

public class KtRenderer implements GLSurfaceView.Renderer {
	private Context context;
	
	public KtRenderer(Context context) {
		this.context = context;
	}
	
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {

	}

	public void onDrawFrame(GL10 gl) {
		KtLib.step();
	}

	public void onSurfaceChanged(GL10 gl, int width, int height) {
		KtLib.init(width, height, context.getApplicationInfo().sourceDir);
	}
}