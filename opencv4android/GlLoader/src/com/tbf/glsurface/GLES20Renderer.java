package com.tbf.glsurface;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView;

import com.tbf.glsurface.adapters.CameraProjectionAdapter;
import com.tbf.glsurface.filters.ar.ARFilter;

public class GLES20Renderer implements GLSurfaceView.Renderer {
	public ARFilter filter;
	public CameraProjectionAdapter cameraProjectionAdapter;
	public float scale = 1.0f;

	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		LibraryClass.init();
	}

	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height) {
		LibraryClass.resize(width, height);
	}

	@Override
	public void onDrawFrame(GL10 gl) {
		gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
        gl.glClearColor(0f, 0f, 0f, 0f); // transparent
        if (filter == null) {
            return;
        }
        
        if (cameraProjectionAdapter == null) {
            return;
        }
        
        float[] pose = filter.getGLPose();
        if (pose == null) {
            return;
        }
        
        //gl.glMatrixMode(GL10.GL_PROJECTION);
        float[] projection = cameraProjectionAdapter.getProjectionGL();
        //gl.glLoadMatrixf(projection, 0);
        
        //gl.glMatrixMode(GL10.GL_MODELVIEW);
        //gl.glLoadMatrixf(pose, 0);
    	
        LibraryClass.render(projection, pose);
	}

}
