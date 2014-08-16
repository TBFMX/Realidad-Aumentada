package com.tbf.cameragl;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
//using org.opencv.core.Size instead of camera.size
import org.opencv.core.Size;
import android.opengl.GLSurfaceView.Renderer;

public class CameraRenderer implements Renderer {
	 private Context context;
	 private Size size;
	 public CameraRenderer(Context c,Size size) {
		  super();
		  context = c;
		  this.size = size;
	}

	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		// TODO Auto-generated method stub
		  Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
		  Native.initCamera((int)size.width,(int)size.height);
	}

	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height) {
		// TODO Auto-generated method stub
//		Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
		Native.surfaceChanged(width,height,context.getResources().getConfiguration().orientation);
	}

	@Override
	public void onDrawFrame(GL10 gl) {
		// TODO Auto-generated method stub
//		Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
		Native.renderBackground();
	}
	
	public void changeShownModel() {
		// TODO Auto-generated method stub
		Native.changeShownModel();
	}

	public float getAngle() {
		// TODO Auto-generated method stub
		return Native.getAngle();
	}

	public void setAngle(float angle) {
		// TODO Auto-generated method stub
		Native.setAngle(angle);
	}

	public float getWidth() {
		// TODO Auto-generated method stub
		return (float) size.width;
	}

	public float getHeight() {
		// TODO Auto-generated method stub
		return (float) size.height;
	}

	public void setScale(float scale) {
		// TODO Auto-generated method stub
		Native.setScale(scale);
	}

	public float getScale() {
		// TODO Auto-generated method stub
		return Native.getScale();
	}

	public void setTranslate(float x, float y) {
		// TODO Auto-generated method stub
		Native.setTranslate(x,y);
	}

	public void changeIlluminationModel() {
		// TODO Auto-generated method stub
		Native.changeIlluminationModel();
	}
}
