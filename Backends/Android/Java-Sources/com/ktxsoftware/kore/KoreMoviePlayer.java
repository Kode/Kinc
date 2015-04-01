package com.ktxsoftware.kore;

import android.graphics.SurfaceTexture.OnFrameAvailableListener;
import android.view.Surface;

public class KoreMoviePlayer {
	private KoreMovieTexture movieTexture;

	public KoreMoviePlayer(String path/*, OnFrameAvailableListener onFrameAvailableListener*/) {
		movieTexture = new KoreMovieTexture(/*onFrameAvailableListener*/);
		Surface surface = new Surface(movieTexture.surfaceTexture);
		nativeCreate(path, surface);
		surface.release();
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
