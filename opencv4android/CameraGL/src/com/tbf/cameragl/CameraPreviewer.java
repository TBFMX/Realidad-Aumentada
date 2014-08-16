package com.tbf.cameragl;

import java.util.List;

import org.opencv.android.CameraBridgeViewBase.ListItemAccessor;
import org.opencv.android.NativeCameraView.OpenCvSizeAccessor;
import org.opencv.highgui.Highgui;
import org.opencv.highgui.VideoCapture;
// using org.opencv.core.Size instead of camera.size
import org.opencv.core.Size;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Display;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;

public class CameraPreviewer extends Activity {

    GLSurfaceView mView;
    //CameraRenderer mCameraRenderer;
    ResponsiveGLSurfaceView mCameraRenderer;
    
    @Override 
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        Native.loadlibs();
        //VideoCapture mCamera = new VideoCapture(Highgui.CV_CAP_ANDROID);
        VideoCapture mCamera = new VideoCapture(Highgui.CV_CAP_ANDROID);
        List<Size> sizes = mCamera.getSupportedPreviewSizes();
        mCamera.release();
 
        mView = new GLSurfaceView(getApplication()){
         @Override
         public void onPause() {
          // TODO Auto-generated method stub
        	 super.onPause();
        	 Native.releaseCamera();
         }
        };
        Size size = calculateCameraFrameSize(sizes,new OpenCvSizeAccessor());
        //mCameraRenderer = new CameraRenderer(this,size);
        mCameraRenderer = new ResponsiveGLSurfaceView(this,size);
        mView.setRenderer(mCameraRenderer.getCameraRenderer());
        setContentView(mView);
    }

@SuppressWarnings("deprecation")
protected Size calculateCameraFrameSize( List<Size> supportedSizes, ListItemAccessor accessor) {
	int calcWidth = Integer.MAX_VALUE;
	int calcHeight = Integer.MAX_VALUE;

	Display display = getWindowManager().getDefaultDisplay();

	int maxAllowedWidth = 1920;
	maxAllowedWidth = 1024;
	int maxAllowedHeight = 1536;
	maxAllowedHeight = 1024;

	for (Object size : supportedSizes) {
		int width = accessor.getWidth(size);
		int height = accessor.getHeight(size);

		if (width <= maxAllowedWidth && height <= maxAllowedHeight) {
			if ( width <= calcWidth
					&& width >= (maxAllowedWidth/2)
					&& (display.getWidth() % width ==0||display.getHeight() % height==0)
			){
				calcWidth = (int) width;
				calcHeight = (int) height;
			}
		}
	}

	return new Size(calcWidth, calcHeight);
}
    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
    }
    
    @Override 
    public boolean onCreateOptionsMenu(Menu menu){
    	getMenuInflater().inflate(R.menu.camera_previewer, menu);
		return true;
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
	    // Handle item selection
	    switch (item.getItemId()) {
	    case R.id.next_model:
	    //this.startActivity(new Intent(android.provider.Settings.ACTION_SETTINGS));
	    	mCameraRenderer.changeShownModel();
	    	return true;
	    case R.id.next_illum:
	    //this.startActivity(new Intent(android.provider.Settings.ACTION_SETTINGS));
	    	mCameraRenderer.changeIlluminationModel();
	    	return true;
	    case R.id.reset:
		    //this.startActivity(new Intent(android.provider.Settings.ACTION_SETTINGS));
		    mCameraRenderer.backToDefaults();
		    return true;
	    default:
	    return super.onOptionsItemSelected(item);
	    }
    }
    
    public boolean onTouchEvent(MotionEvent event){
    	return mCameraRenderer.onTouchEvent(event);
    }
    
}