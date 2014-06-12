package com.tbf.sample1java;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;

public class CameraTemplate extends Activity implements CvCameraViewListener2
{
    private Mat mRgba;
    private Mat mGray;
	private CameraBridgeViewBase mOpenCvCameraView;
	private String TAG="OCVSample::Activity";
	private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
		@Override
	    public void onManagerConnected(int status) {
	        switch (status) {
	            case LoaderCallbackInterface.SUCCESS:
	            {
	                Log.i(TAG, "OpenCV loaded successfully");
	             // Load native library after(!) OpenCV initialization
	                System.loadLibrary("cppLibrary");
	                mOpenCvCameraView.enableView();
	            } break;
	            default:
	            {
	                super.onManagerConnected(status);
	            } break;
	        }
	    }
	};
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "called onCreate");
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.fragment_main);

        mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.HelloOpenCvView);
        mOpenCvCameraView.setCvCameraViewListener(this);
    }	
	
	@Override
	public void onResume()
	{
	    super.onResume();
	    OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_6, this, mLoaderCallback);
	}
	
	 @Override
	 public void onPause()
	 {
	     super.onPause();
	     if (mOpenCvCameraView != null)
	         mOpenCvCameraView.disableView();
	 }

	 public void onDestroy() {
	     super.onDestroy();
	     if (mOpenCvCameraView != null)
	         mOpenCvCameraView.disableView();
	 }
	@Override
	public void onCameraViewStarted(int width, int height) {
		// TODO Auto-generated method stub
        mGray = new Mat();
        mRgba = new Mat();		
	}

	@Override
	public void onCameraViewStopped() {
		// TODO Auto-generated method stub
        mGray.release();
        mRgba.release();		
	}

	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		// TODO Auto-generated method stub
        mRgba = inputFrame.rgba();
        mGray = inputFrame.gray();
		return mRgba;
	}

}
