package com.ktxsoftware.kore;

import java.util.ArrayList;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

public class KoreView extends GLSurfaceView implements View.OnTouchListener {
	private static KoreView instance;
	public static int mouseX;
	public static int mouseY;
	public static boolean mouseButton;
	public static ArrayList<KoreKeyEvent> keyEvents;
	public static Object inputLock = new Object();
	
	private boolean lastMouseButton;
	
	private InputMethodManager inputManager;
	
	public static KoreView getInstance() {
		return instance;
	}
	
	//unused
	public KoreView(Context context) {
		super(context);
	}
	
	public KoreView(KoreActivity activity) {
		super(activity);
		instance = this;
		setFocusable(true);
		setFocusableInTouchMode(true);
		keyEvents = new ArrayList<KoreKeyEvent>();
		setEGLContextClientVersion(2);
   		setRenderer(new KoreRenderer(activity.getApplicationContext()));
   		setOnTouchListener(this);
   		inputManager = (InputMethodManager)activity.getSystemService(Context.INPUT_METHOD_SERVICE);
	}
	
	public void showKeyboard() {
		inputManager.toggleSoftInputFromWindow(getApplicationWindowToken(), InputMethodManager.SHOW_IMPLICIT, 0);
	}
	
	public void hideKeyboard() {
		inputManager.hideSoftInputFromWindow(getApplicationWindowToken(), InputMethodManager.HIDE_NOT_ALWAYS);
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
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		synchronized(inputLock) {
			keyEvents.add(new KoreKeyEvent(keyCode, true));
		}
		return true;
	}
	
	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		synchronized(inputLock) {
			keyEvents.add(new KoreKeyEvent(keyCode, false));
		}
	    return true;
	}
}
