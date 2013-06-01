package com.ktxsoftware.kore;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.MotionEvent;
import android.view.View;

public class KoreView extends GLSurfaceView implements View.OnTouchListener {
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
		switch (event.getActionMasked()) {
		case MotionEvent.ACTION_MOVE:
			KoreLib.touchMove((int)event.getX(), (int)event.getY());
			break;
		case MotionEvent.ACTION_UP:
			KoreLib.touchUp((int)event.getX(), (int)event.getY());
			break;
		case MotionEvent.ACTION_DOWN:
			KoreLib.touchDown((int)event.getX(), (int)event.getY());
			break;
		}
		return true;
	}
}
