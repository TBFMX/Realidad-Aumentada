package com.tbf.demotamarindo;

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
		Native.surfaceChanged(width,height,context.getResources().getConfiguration().orientation);
	}

	@Override
	public void onDrawFrame(GL10 gl) {
		// TODO Auto-generated method stub
		//Native.renderFurnish();
		Native.renderBackground();
		
	}

}
