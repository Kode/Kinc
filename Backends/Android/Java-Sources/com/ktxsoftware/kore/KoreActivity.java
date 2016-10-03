package com.ktxsoftware.kore;

import android.app.NativeActivity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.view.View;
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

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            getWindow().getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);}
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
