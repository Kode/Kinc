package com.ktxsoftware.kore;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.text.method.MetaKeyKeyListener;
import android.util.Log;
import android.view.KeyCharacterMap;

// import com.google.vrtoolkit.cardboard.HeadTransform;


public class KoreRenderer implements GLSurfaceView.Renderer {
	private Context context;
	private boolean keyboardShown = false;
	private KeyCharacterMap keyMap;
	private KoreView view;
	
	public KoreRenderer(Context context, KoreView view) {
	// Orientation for cardboard
	/* private HeadTransform mHeadTransform;
	private float[] mQuaternion;
	private float[] matrix;
	
	*/
		this.context = context;
		this.view = view;
		keyMap = KeyCharacterMap.load(-1);
		
		//mQuaternion = new float[4];
		//matrix = new float[16];
		//mHeadTransform = new HeadTransform();
	} 
	
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {

	}
	
	/*
	private void updateViewAngles() {
		KoreActivity.getInstance().mHeadTracker.getLastHeadView(matrix, 0);
		getQuaternion(matrix, mQuaternion);
	} */
	
	
	public void getEulerAngles(float[] m, float[] angles) {
		float pitch = (float)Math.asin(m[6]);
		float roll;
		float yaw;

		if ((float)Math.sqrt(1.0F - m[6] * m[6]) >= 0.01F) {
			yaw = (float)Math.atan2(-m[2], m[10]);
			roll = (float)Math.atan2(m[4], m[5]);
		}
		else {
			yaw = 0.0F;
			roll = (float)Math.atan2(m[1], m[0]);
		}
		angles[0] = -pitch;
		angles[1] = -yaw;
		angles[2] = -roll;
	}
	
	public void quatMult(float[]a, float[] b, float[] result) {
		result[0] = a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1];
		result[1] = a[3] * b[1] - a[0] * b[2] + a[1] * b[3] + a[2] * b[0];
		result[2] = a[3] * b[2] + a[0] * b[1] - a[1] * b[0] + a[2] * b[3];
		result[3] = a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2];
	}
	
	public void fromAxisAngle(float[] axis, float radians, float[] result) {
		result[3] = (float) Math.cos(radians / 2.0);
		result[0] = result[1] = result[2] = (float) Math.sin(radians / 2.0);
		result[0] *= axis[0];
		result[1] *= axis[1];
		result[2] *= axis[2];
	}
	
	public void getQuaternion(float[] m, float[] quaternion) {
		float[] angles = new float[3];
		getEulerAngles(m, angles);
		// pitch, yaw, roll
		
		float[] x_axis = new float[3];
		x_axis[0] = 1;
		
		float[] y_axis = new float[3];
		y_axis[1] = 1;
		
		float[] z_axis = new float[3];
		z_axis[2] = 1;
		
		//y, x, z
		float[] quatX = new float[4];
		float[] quatY = new float[4];
		float[] quatZ = new float[4];
		
		fromAxisAngle(x_axis, angles[0], quatX);
		fromAxisAngle(y_axis, angles[1], quatY);
		fromAxisAngle(z_axis, angles[2], quatZ);
		
		Log.d("FM", "Angles "+ angles[0] + " " + angles[1]  + " " + angles[2]);
 
		float[] yx = new float[4];
		quatMult(quatY, quatX, yx);
		
		quatMult(yx, quatZ, quaternion);
		
		
		
		
		/* float tr = m[0] + m[5] + m[15];
		float qw = 0;
		float qx = 0;
		float qy = 0;
		float qz = 0;
		
		
		if (tr > 0) { 
			
			float S = FloatMath.sqrt(tr+1.0f) * 2; // S=4*qw
			  qw = 0.25f * S;
			  qx = (matrix[9] - matrix[6]) / S;
			  qy = (matrix[2] - matrix[8]) / S; 
			  qz = (matrix[4] - matrix[1]) / S; 
			} else if ((matrix[0] > matrix[5])&(matrix[0] > matrix[10])) { 
			  float S = FloatMath.sqrt(1.0f + matrix[0] - matrix[5] - matrix[10]) * 2; // S=4*qx 
			  qw = (matrix[9] - matrix[6]) / S;
			  qx = 0.25f * S;
			  qy = (matrix[1] + matrix[4]) / S; 
			  qz = (matrix[2] + matrix[8]) / S; 
			} else if (matrix[5] > matrix[10]) { 
			  float S = FloatMath.sqrt(1.0f + matrix[5] - matrix[0] - matrix[10]) * 2; // S=4*qy
			  qw = (matrix[2] - matrix[8]) / S;
			  qx = (matrix[1] + matrix[4]) / S; 
			  qy = 0.25f * S;
			  qz = (matrix[6] + matrix[9]) / S; 
			} else { 
			  float S = FloatMath.sqrt(1.0f + matrix[10] - matrix[0] - matrix[5]) * 2; // S=4*qz
			  qw = (matrix[4] - matrix[1]) / S;
			  qx = (matrix[2] + matrix[8]) / S;
			  qy = (matrix[6] + matrix[9]) / S;
			  qz = 0.25f * S;
			}
		quaternion[0] = qx;
		quaternion[1] = qy;
		quaternion[2] = qz;
		quaternion[3] = qw;
		
		
		
		float yaw = 0;
		if (FloatMath.sqrt(1.0F - matrix[6] * matrix[6]) >= 0.01F)
	    {
	       yaw = (float)Math.atan2(-matrix[2], matrix[10]);
	      //roll = (float)Math.atan2(-matrix[4], matrix[5]);
	    }
	    else
	    {
	      yaw = 0.0F;
//	      roll = (float)Math.atan2(this.headView[1], this.headView[0]);
	    }
		Log.d("FM", "Yaw angle: " +  yaw); */
	}

	public void onDrawFrame(GL10 gl) {
		for (KoreMoviePlayer player : KoreMoviePlayer.players) {
			player.update();
		}
		
		//updateViewAngles();
		
		//KoreLib.gaze(mQuaternion[0], mQuaternion[1], mQuaternion[2], mQuaternion[3]);
		KoreLib.step();
		
		if (KoreLib.keyboardShown()) {
			if (!keyboardShown) {
				keyboardShown = true;
				KoreActivity.getInstance().runOnUiThread(new Runnable() {
				    public void run() {
				        view.showKeyboard();
				    }
				});
			}
		}
		else {
			if (keyboardShown) {
				keyboardShown = false;
				KoreActivity.getInstance().runOnUiThread(new Runnable() {
				    public void run() {
				        view.hideKeyboard();
				    }
				});
			}
		}
	}

	public void onSurfaceChanged(GL10 gl, int width, int height) {
		KoreLib.init(width, height, context.getResources().getAssets(), context.getApplicationInfo().sourceDir, context.getFilesDir().toString());
	}
	
	public void key(int keyCode, boolean down) {
		switch (keyCode) {
		case 59: // shift
			if (down) KoreLib.keyDown(0x00000120);
			else KoreLib.keyUp(0x00000120);
			break;
		case 66: // return
			if (down) KoreLib.keyDown(0x00000104);
			else KoreLib.keyUp(0x00000104);
			break;
		case 67: // backspace
			if (down) KoreLib.keyDown(0x00000103);
			else KoreLib.keyUp(0x00000103);
			break;
		default:
			int code = keyMap.get(keyCode, MetaKeyKeyListener.META_SHIFT_ON);
			if (down) KoreLib.keyDown(code);
			else KoreLib.keyUp(code);
			break;
		}
	}
	
	public void touch(int index, int x, int y, int action) {
		KoreLib.touch(index, x, y, action);
	}
	
	public void accelerometer(float x, float y, float z) {
		KoreLib.accelerometerChanged(x, y, z);
	}
	
	public void gyro(float x, float y, float z) {
		KoreLib.gyroChanged(x, y, z);
	}
}
