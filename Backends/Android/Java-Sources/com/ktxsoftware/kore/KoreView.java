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
	public static ArrayList<KoreTouchEvent> touchEvents;
	public static ArrayList<KoreKeyEvent> keyEvents;
	public static Object inputLock = new Object();
	
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
		touchEvents = new ArrayList<KoreTouchEvent>();
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
		int index = event.getActionIndex();
		int maskedAction = event.getActionMasked();
		int action = -1;
		action = (maskedAction == MotionEvent.ACTION_DOWN || maskedAction == MotionEvent.ACTION_POINTER_DOWN) ? KoreTouchEvent.ACTION_DOWN : -1;
		action = (action == -1 && maskedAction == MotionEvent.ACTION_MOVE) ? KoreTouchEvent.ACTION_MOVE : action;
		action = (action == -1 && (maskedAction == MotionEvent.ACTION_UP || maskedAction == MotionEvent.ACTION_POINTER_UP || maskedAction == MotionEvent.ACTION_CANCEL))
				? KoreTouchEvent.ACTION_UP : action;
		KoreTouchEvent e = new KoreTouchEvent(event.getPointerId(index), Math.round(event.getX(index)), Math.round(event.getY(index)), action);
		synchronized(inputLock) {
			touchEvents.add(e);
		}
		return true;
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		KoreKeyEvent e = new KoreKeyEvent(keyCode, true);
		synchronized(inputLock) {
			keyEvents.add(e);
		}
		return true;
	}
	
	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		KoreKeyEvent e = new KoreKeyEvent(keyCode, false);
		synchronized(inputLock) {
			keyEvents.add(e);
		}
	    return true;
	}
}
