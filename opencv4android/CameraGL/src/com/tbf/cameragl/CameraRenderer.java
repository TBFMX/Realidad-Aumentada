package com.tbf.cameragl;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
//using org.opencv.core.Size instead of camera.size
import org.opencv.core.Size;
import android.opengl.GLSurfaceView.Renderer;

public class CameraRenderer implements Renderer {
	 private Context mContext;
	 private Size mSize;
	 public CameraRenderer(Context c,Size size) {
		  super();
		  mContext = c;
		  this.mSize = size;
	}

	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		// TODO Auto-generated method stub
		  Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
		  Native.initCamera((int)mSize.width,(int)mSize.height);
	}

	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height) {
		// TODO Auto-generated method stub
//		Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
		Native.surfaceChanged(width,height,mContext.getResources().getConfiguration().orientation);
	}

	@Override
	public void onDrawFrame(GL10 gl) {
		// TODO Auto-generated method stub
		Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
		Native.renderBackground();
	}
	public float getWidth() {
		// TODO Auto-generated method stub
		return (float) mSize.width;
	}

	public float getHeight() {
		// TODO Auto-generated method stub
		return (float) mSize.height;
	}
}
