package com.ktxsoftware.kore;

import android.app.NativeActivity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;

public class KoreActivity extends NativeActivity {
	private static KoreActivity instance;
	private InputMethodManager inputManager;

	public static KoreActivity getInstance() {
		return instance;
	}
	
	@Override
	protected void onCreate(Bundle state) {
		super.onCreate(state);
		instance = this;
		inputManager = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
	}

	public static void showKeyboard() {
		getInstance().inputManager.showSoftInput(getInstance().getWindow().getDecorView(), 0);
	}

	public static void hideKeyboard() {
		getInstance().inputManager.hideSoftInputFromWindow(getInstance().getWindow().getDecorView().getWindowToken(), 0);
	}

	public static void loadURL(String url) {
		Intent i = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
		instance.startActivity(i);
	}

	public static int getRotation() {
		Context context = getInstance().getApplicationContext();
		WindowManager manager = (WindowManager)context.getSystemService(Context.WINDOW_SERVICE);
		return manager.getDefaultDisplay().getRotation();
	}

	public static int getScreenDpi() {
		Context context = getInstance().getApplicationContext();
		WindowManager manager = (WindowManager)context.getSystemService(Context.WINDOW_SERVICE);
		android.util.DisplayMetrics metrics = new android.util.DisplayMetrics();
		manager.getDefaultDisplay().getMetrics(metrics);
		return (int)(metrics.density * android.util.DisplayMetrics.DENSITY_DEFAULT);
	}
}
