package com.ktxsoftware.kore;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.MotionEvent;
import android.view.View;

public class KoreView extends GLSurfaceView implements View.OnTouchListener {
	public static int mouseX;
	public static int mouseY;
	public static boolean mouseButton;
	public static Object inputLock = new Object();
	
	private boolean lastMouseButton;
	
	//unused
	public KoreView(Context context) {
		super(context);
	}
	
	public KoreView(KoreActivity activity) {
		super(activity);
		setEGLContextClientVersion(2);
   		setRenderer(new KoreRenderer(activity.getApplicationContext()));
   		setOnTouchListener(this);
	}
	
	@Override
	public boolean onTouch(View view, MotionEvent event) {
		int mouseX = (int)event.getX();
		int mouseY = (int)event.getY();
		boolean mouseButton = lastMouseButton;
		switch (event.getActionMasked()) {
		case MotionEvent.ACTION_UP:
			mouseButton = false;
			break;
		case MotionEvent.ACTION_DOWN:
			mouseButton = true;
			break;
		}
		lastMouseButton = mouseButton;
		
		synchronized(inputLock) {
			KoreView.mouseX = mouseX;
			KoreView.mouseY = mouseY;
			KoreView.mouseButton = mouseButton;
		}
		
		return true;
	}
}
