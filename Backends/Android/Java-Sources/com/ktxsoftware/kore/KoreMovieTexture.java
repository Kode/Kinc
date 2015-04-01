package com.ktxsoftware.kore;

import android.graphics.SurfaceTexture;
import android.graphics.SurfaceTexture.OnFrameAvailableListener;
import android.opengl.GLES20;
import android.opengl.Matrix;

public class KoreMovieTexture {
	public int textureId;
	SurfaceTexture surfaceTexture;
	boolean updateTexture;
	float[] stMatrix = new float[16];

	private static final int GL_TEXTURE_EXTERNAL_OES = 0x8D65;

	public KoreMovieTexture(/*OnFrameAvailableListener onFrameAvailableListener*/) {
		Matrix.setIdentityM(stMatrix, 0);

		int[] textures = new int[1];
		GLES20.glGenTextures(1, textures, 0);
		textureId = textures[0];
		GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId);
		// Can't do mipmapping with camera source
		GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES,
				GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
		GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES,
				GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
		// Clamp to edge is the only option
		GLES20.glTexParameteri(GL_TEXTURE_EXTERNAL_OES,
				GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
		GLES20.glTexParameteri(GL_TEXTURE_EXTERNAL_OES,
				GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);

		surfaceTexture = new SurfaceTexture(textureId);

		//surfaceTexture.setOnFrameAvailableListener(onFrameAvailableListener);

		updateTexture = false;
	}

	public boolean update() {
		boolean ret = updateTexture;
		if (updateTexture) {

			surfaceTexture.updateTexImage();
			updateTexture = false;
			surfaceTexture.getTransformMatrix(stMatrix);
		}
		return ret;
	}
}
