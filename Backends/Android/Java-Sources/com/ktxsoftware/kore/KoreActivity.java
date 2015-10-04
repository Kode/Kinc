package com.ktxsoftware.kore;

import android.app.NativeActivity;
import android.content.Context;
import android.os.Bundle;
import android.view.WindowManager;

public class KoreActivity extends NativeActivity {
	//static {
	//	System.loadLibrary("kore");
	//}

	private static KoreActivity instance;

	public static KoreActivity getInstance() {
		return instance;
	}
	
	@Override
	protected void onCreate(Bundle state) {
		super.onCreate(state);
		instance = this;
	}

	public static int getRotation() {
		Context context = getInstance().getApplicationContext();
		WindowManager manager = (WindowManager)context.getSystemService(Context.WINDOW_SERVICE);
		return manager.getDefaultDisplay().getRotation();
	}
}
