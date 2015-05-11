package com.ktxsoftware.kore;

import java.util.ArrayList;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.text.method.MetaKeyKeyListener;
import android.util.FloatMath;
import android.util.Log;
import android.view.KeyCharacterMap;

import com.google.vrtoolkit.cardboard.HeadTransform;


public class KoreRenderer implements GLSurfaceView.Renderer {
	private Context context;
	private boolean keyboardShown = false;
	private ArrayList<KoreTouchEvent> touchEvents;
	private ArrayList<KoreKeyEvent> keyEvents;
	private KeyCharacterMap keyMap;
	//private boolean shift = false;
	private float lastAccelerometerX, lastAccelerometerY, lastAccelerometerZ;
	private float lastGyroX, lastGyroY, lastGyroZ;
	
	
	// Orientation for cardboard
	/* private HeadTransform mHeadTransform;
	private float[] mQuaternion;
	private float[] matrix;
	
	public KoreRenderer(Context context) {
		this.context = context;
		touchEvents = new ArrayList<KoreTouchEvent>();
		keyEvents = new ArrayList<KoreKeyEvent>();
		keyMap = KeyCharacterMap.load(-1);
		
		mQuaternion = new float[4];
		matrix = new float[16];
		mHeadTransform = new HeadTransform();
	} */
	
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
	    
	    if (FloatMath.sqrt(1.0F - m[6] * m[6]) >= 0.01F)
	    {
	      yaw = (float)Math.atan2(-m[2], m[10]);
	      roll = (float)Math.atan2(m[4], m[5]);
	    }
	    else
	    {
	      yaw = 0.0F;
	      roll = (float)Math.atan2(m[1], m[0]);
	    }
	    angles[0] = (-pitch);
	    angles[1] = (-yaw);
	    angles[2] = (-roll);
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
		
		synchronized(KoreView.inputLock) {
			touchEvents.addAll(KoreView.touchEvents);
			KoreView.touchEvents.clear();
			keyEvents.addAll(KoreView.keyEvents);
			KoreView.keyEvents.clear();
		}
		
		//updateViewAngles();
		
		//KoreLib.gaze(mQuaternion[0], mQuaternion[1], mQuaternion[2], mQuaternion[3]);
				
		for (int i = 0; i < touchEvents.size(); ++i) {
			KoreTouchEvent e = touchEvents.get(i);
			KoreLib.touch(e.index, e.x, e.y, e.action);
		}
		touchEvents.clear();
		
		for (int i = 0; i < keyEvents.size(); ++i) {
			KoreKeyEvent e = keyEvents.get(i);
			int original = e.code;
			if (original == 59) { // shift
				if (e.down) KoreLib.keyDown(0x00000120);
				else KoreLib.keyUp(0x00000120);
				continue;
			}
			if (original == 67) { // backspace
				if (e.down) KoreLib.keyDown(0x00000103);
				else KoreLib.keyUp(0x00000103);
				continue;
			}
			if (original == 66) { // return
				if (e.down) KoreLib.keyDown(0x00000104);
				else KoreLib.keyUp(0x00000104);
				continue;
			}
			int code = keyMap.get(original, MetaKeyKeyListener.META_SHIFT_ON);
			//System.out.println("Key: " + code + " from " + original);
			if (e.down) KoreLib.keyDown(code);
			else KoreLib.keyUp(code);
		}
		keyEvents.clear();
		
		float accelerometerX, accelerometerY, accelerometerZ;
		float gyroX, gyroY, gyroZ;
		synchronized(KoreActivity.sensorLock) {
			accelerometerX = KoreActivity.accelerometerX;
			accelerometerY = KoreActivity.accelerometerY;
			accelerometerZ = KoreActivity.accelerometerZ;
			gyroX = KoreActivity.gyroX;
			gyroY = KoreActivity.gyroY;
			gyroZ = KoreActivity.gyroZ;
		}
		if (accelerometerX != lastAccelerometerX || accelerometerY != lastAccelerometerY || accelerometerZ != lastAccelerometerZ) {
			KoreLib.accelerometerChanged(accelerometerX, accelerometerY, accelerometerZ);
			lastAccelerometerX = accelerometerX;
			lastAccelerometerY = accelerometerY;
			lastAccelerometerZ = accelerometerZ;
		}
		if (gyroX != lastGyroX || gyroY != lastGyroY || gyroZ != lastGyroZ) {
			KoreLib.gyroChanged(gyroX, gyroY, gyroZ);
			lastGyroX = gyroX;
			lastGyroY = gyroY;
			lastGyroZ = gyroZ;
		}
		
		KoreLib.step();
		
		if (KoreLib.keyboardShown()) {
			if (!keyboardShown) {
				keyboardShown = true;
				KoreActivity.getInstance().runOnUiThread(new Runnable(){
				    public void run(){
				        KoreView.getInstance().showKeyboard();
				    }
				});
			}
		}
		else {
			if (keyboardShown) {
				keyboardShown = false;
				KoreActivity.getInstance().runOnUiThread(new Runnable(){
				    public void run(){
				        KoreView.getInstance().hideKeyboard();
				    }
				});
			}
		}
	}

	public void onSurfaceChanged(GL10 gl, int width, int height) {
		KoreLib.init(width, height, context.getResources().getAssets(), context.getApplicationInfo().sourceDir, context.getFilesDir().toString());
	}
}
