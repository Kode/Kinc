package com.ktxsoftware.kore;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.SurfaceView;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

public class KoreView extends GLSurfaceView implements View.OnTouchListener {

	final int ACTION_DOWN = 0;
	final int ACTION_MOVE = 1;
	final int ACTION_UP = 2;

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
		
		final int id = event.getPointerId(index);

		switch (maskedAction) {
			case MotionEvent.ACTION_DOWN:
			case MotionEvent.ACTION_POINTER_DOWN:

				final float fx = event.getX(index);
				final float fy = event.getY(index);

				queueEvent(new Runnable() {
					@Override
					public void run() {
						renderer.touch(id, Math.round(fx), Math.round(fy), ACTION_DOWN);
					}
				});
				break;
			case MotionEvent.ACTION_MOVE:
				int pointerCount = event.getPointerCount();
				for(int i = 0; i < pointerCount; ++i)
				{
					final float fxm = event.getX(i);
					final float fym = event.getY(i);

					final int idx = event.getPointerId(i);

					queueEvent(new Runnable() {
						@Override
						public void run() {
							renderer.touch(idx, Math.round(fxm), Math.round(fym), ACTION_MOVE);
						}
					});
				}

				break;
			case MotionEvent.ACTION_UP:
			case MotionEvent.ACTION_POINTER_UP:
			case MotionEvent.ACTION_CANCEL:

				final float fxu = event.getX(index);
				final float fyu = event.getY(index);

				queueEvent(new Runnable() {
					@Override
					public void run() {
						renderer.touch(id, Math.round(fxu), Math.round(fyu), ACTION_UP);
					}
				});

				break;
		}

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


