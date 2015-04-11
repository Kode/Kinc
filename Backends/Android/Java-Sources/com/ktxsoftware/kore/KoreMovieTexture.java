package com.ktxsoftware.kore;

import android.graphics.SurfaceTexture;
import android.graphics.SurfaceTexture.OnFrameAvailableListener;
import android.opengl.GLES20;

public class KoreMovieTexture implements OnFrameAvailableListener {
	public int textureId;
	public SurfaceTexture surfaceTexture;
	private boolean updateTexture;

	private static final int GL_TEXTURE_EXTERNAL_OES = 0x8D65;

	public KoreMovieTexture() {
		int[] textures = new int[1];
		GLES20.glGenTextures(1, textures, 0);
		textureId = textures[0];
		GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId);
		GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
		GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
		GLES20.glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
		GLES20.glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);

		surfaceTexture = new SurfaceTexture(textureId);
		surfaceTexture.setOnFrameAvailableListener(this);

		updateTexture = false;
	}

	public boolean update() {
		boolean ret = updateTexture;
		if (updateTexture) {
			surfaceTexture.updateTexImage();
			updateTexture = false;
		}
		return ret;
	}
	
	public void onFrameAvailable(SurfaceTexture surface) {
		if (surfaceTexture == surface) {
			updateTexture = true;
		}
	}
}
