package com.ktxsoftware.kore;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

public class KoreView extends GLSurfaceView implements View.OnTouchListener {
	private KoreRenderer renderer;
	private InputMethodManager inputManager;
	
	//unused
	public KoreView(Context context) {
		super(context);
	}
	
	public KoreView(KoreActivity activity) {
		super(activity);
		setFocusable(true);
		setFocusableInTouchMode(true);
		setPreserveEGLContextOnPause(true);
		setEGLContextClientVersion(2);
   		setRenderer(renderer = new KoreRenderer(activity.getApplicationContext(), this));
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
	public boolean onTouch(View view, final MotionEvent event) {
		final int index = event.getActionIndex();
		int maskedAction = event.getActionMasked();
		final int ACTION_DOWN = 0;
		final int ACTION_MOVE = 1;
		final int ACTION_UP = 2;
		int action = -1;
		action = (maskedAction == MotionEvent.ACTION_DOWN || maskedAction == MotionEvent.ACTION_POINTER_DOWN) ? ACTION_DOWN : -1;
		action = (action == -1 && maskedAction == MotionEvent.ACTION_MOVE) ? ACTION_MOVE : action;
		action = (action == -1 && (maskedAction == MotionEvent.ACTION_UP || maskedAction == MotionEvent.ACTION_POINTER_UP || maskedAction == MotionEvent.ACTION_CANCEL))
				? ACTION_UP : action;
		final int finalAction = action;
		queueEvent(new Runnable() {
			@Override
			public void run() {
				renderer.touch(event.getPointerId(index), Math.round(event.getX(index)), Math.round(event.getY(index)), finalAction);
			}
		});
		return true;
	}
	
	@Override
	public boolean onKeyDown(final int keyCode, KeyEvent event) {
		switch (event.getKeyCode()) {
		case KeyEvent.KEYCODE_VOLUME_DOWN:
		case KeyEvent.KEYCODE_VOLUME_MUTE:
		case KeyEvent.KEYCODE_VOLUME_UP:
		case KeyEvent.KEYCODE_BACK:
			return false;
		}
		this.queueEvent(new Runnable() {
			@Override
			public void run() {
				renderer.key(keyCode, true);
			}
		});
		return true;
	}
	
	@Override
	public boolean onKeyUp(final int keyCode, KeyEvent event) {
		switch (event.getKeyCode()) {
		case KeyEvent.KEYCODE_VOLUME_DOWN:
		case KeyEvent.KEYCODE_VOLUME_MUTE:
		case KeyEvent.KEYCODE_VOLUME_UP:
		case KeyEvent.KEYCODE_BACK:
			return false;
		}
		this.queueEvent(new Runnable() {
			@Override
			public void run() {
				renderer.key(keyCode, false);
			}
		});
	    return true;
	}
	
	public void accelerometer(final float x, final float y, final float z) {
		queueEvent(new Runnable() {
			@Override
			public void run() {
				renderer.accelerometer(x, y, z);
			}
		});
	}
	
	public void gyro(final float x, final float y, final float z) {
		queueEvent(new Runnable() {
			@Override
			public void run() {
				renderer.gyro(x, y, z);
			}
		});
	}
}
