package com.ktxsoftware.kt;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;

public class KtActivity extends Activity {
	private GLSurfaceView view;

	@Override
	protected void onCreate(Bundle state) {
		super.onCreate(state);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN); // (NEW)
        view = new GLSurfaceView(this);
   		view.setRenderer(new KtRenderer(getApplicationContext()));
		setContentView(view);
	}
}