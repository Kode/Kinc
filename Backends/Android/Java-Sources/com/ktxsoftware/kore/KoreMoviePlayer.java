package com.ktxsoftware.kore;

import android.view.Surface;

public class KoreMoviePlayer {
	private static KoreMoviePlayer instance;
	private KoreMovieTexture movieTexture;

	public KoreMoviePlayer(String path) {
		movieTexture = new KoreMovieTexture();
		Surface surface = new Surface(movieTexture.surfaceTexture);
		nativeCreate(path, surface);
		surface.release();
		instance = this;
	}
	
	public static KoreMoviePlayer getInstance() {
		return instance;
	}

	public KoreMovieTexture getMovieTexture() {
		return movieTexture;
	}

	public boolean update() {
		return movieTexture.update();
	}
	
	public int getTextureId() {
		return movieTexture.textureId;
	}

	private native void nativeCreate(String path, Surface surface);
}
