package com.tbf.glsurface;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView;

import com.tbf.glsurface.adapters.CameraProjectionAdapter;
import com.tbf.glsurface.filters.ar.ARFilter;

public class VideoRenderer  implements GLSurfaceView.Renderer {
	public ARFilter filter;
	public CameraProjectionAdapter cameraProjectionAdapter;

	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		
	}

	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height) {
		
	}

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

        float[] projection = cameraProjectionAdapter.getProjectionGL();

        LibraryClass.videorender(projection,pose);

//		try {
//        InputStream input;
//        int size;
//		input = mAssetManager.open("someToast.txt");
//		size = input.available();
//		byte[] buffer = new byte[size];
//		input.read(buffer);
//        input.close();
//        final String someText = new String(buffer);
//        
//    	Handler handler = new Handler(Looper.getMainLooper());
//    	handler.post(new Runnable() {
//    	     public void run() {
//    	    	 mToast.setText(someText);
//    	    	 mToast.show();
//    	     }
//    	});
//	} catch (IOException e) {
//		e.printStackTrace();
//	}        
	}

}
