package com.tbf.glsurface;

import java.io.IOException;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.NativeCameraView;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Core;
import org.opencv.core.Mat;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.graphics.PixelFormat;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;

import com.tbf.glsurface.adapters.CameraProjectionAdapter;
import com.tbf.glsurface.filters.ar.ARFilter;
import com.tbf.glsurface.filters.ar.ImageDetectionFilter;
import com.tbf.glsurface.filters.ar.NoneARFilter;

public final class CameraActivity extends Activity
        implements CvCameraViewListener2 {
    
    // A tag for log output.
    private static final String TAG = "CameraActivity";
    
    // A key for storing the index of the active camera.
    private static final String STATE_CAMERA_INDEX = "cameraIndex";
    
    // Keys for storing the indices of the active filters.
    private static final String STATE_IMAGE_DETECTION_FILTER_INDEX =
            "imageDetectionFilterIndex";
 
    // The filters.
    private ARFilter[] mImageDetectionFilters;
    
    // The indices of the active filters.
    private int mImageDetectionFilterIndex;
    
    // The index of the active camera.
    private int mCameraIndex;
    
    // Whether the active camera is front-facing.
    // If so, the camera view should be mirrored.
    private boolean mIsCameraFrontFacing;
    
    // The number of cameras on the device.
    private int mNumCameras;
    // The camera view.
    private CameraBridgeViewBase mCameraView;
    
    // An adapter between the video camera and projection matrix.
    private CameraProjectionAdapter mCameraProjectionAdapter;
    
    // The renderer for 3D augmentations.
    private ARCubeRenderer mARRenderer;
    
    // Whether an asynchronous menu action is in progress.
    // If so, menu interaction should be disabled.
    private boolean mIsMenuLocked;
    
    // The OpenCV loader callback.
    private BaseLoaderCallback mLoaderCallback =
    		new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(final int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                    Log.d(TAG, "OpenCV loaded successfully");
                    mCameraView.enableView();
                    
	                final ARFilter starryNight;
                    try {
                        starryNight = new ImageDetectionFilter(
                                CameraActivity.this,
                                R.drawable.starry_night,
                                mCameraProjectionAdapter);
                    } catch (IOException e) {
                        Log.e(TAG, "Failed to load drawable: " +
                                "starry_night");
                        e.printStackTrace();
                        break;
                    }
                    
                    final ARFilter akbarHunting;
                    try {
                        akbarHunting = new ImageDetectionFilter(
                                CameraActivity.this,
                                R.drawable.akbar_hunting_with_cheetahs,
                                mCameraProjectionAdapter);
                    } catch (IOException e) {
                        Log.e(TAG, "Failed to load drawable: " +
                                "akbar_hunting_with_cheetahs");
                        e.printStackTrace();
                        break;
                    }
                    
                    final ARFilter selfPortrait;
                    try {
                        selfPortrait = new ImageDetectionFilter(
                                CameraActivity.this,
                                R.drawable.self_portrait,
                                mCameraProjectionAdapter);
                    } catch (IOException e) {
                        Log.e(TAG, "Failed to load drawable: " +
                                "self_portrait");
                        e.printStackTrace();
                        break;
                    }
                    
                    mImageDetectionFilters = new ARFilter[] {
                            new NoneARFilter(),
                            starryNight,
                            akbarHunting,
                            selfPortrait
                    };

                    break;
                default:
                    super.onManagerConnected(status);
                    break;
            }
        }
    };
    
    @SuppressLint("NewApi")
    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        final Window window = getWindow();
        window.addFlags(
        		WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        
        if (savedInstanceState != null) {
            mCameraIndex = savedInstanceState.getInt(
                    STATE_CAMERA_INDEX, 0);
            mImageDetectionFilterIndex = savedInstanceState.getInt(
                    STATE_IMAGE_DETECTION_FILTER_INDEX, 0);
        } else {
            mCameraIndex = 0;
            mImageDetectionFilterIndex = 0;
        }
        
        FrameLayout layout = new FrameLayout(this);
        layout.setLayoutParams(new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT));
        setContentView(layout);
        
        mCameraView = new NativeCameraView(this, mCameraIndex);
        mCameraView.setCvCameraViewListener(this);
        mCameraView.setLayoutParams(new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT));
        layout.addView(mCameraView);
        
        GLSurfaceView glSurfaceView = new GLSurfaceView(this);
        glSurfaceView.getHolder().setFormat(
                PixelFormat.TRANSPARENT);
        glSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 0, 0);
        glSurfaceView.setZOrderOnTop(true);
        glSurfaceView.setLayoutParams(new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT));
        layout.addView(glSurfaceView);
        
        mCameraProjectionAdapter = new CameraProjectionAdapter();
        
        mARRenderer = new ARCubeRenderer();
        mARRenderer.cameraProjectionAdapter = mCameraProjectionAdapter;
        glSurfaceView.setRenderer(mARRenderer);
        
        final Camera camera;
        if (Build.VERSION.SDK_INT >=
                Build.VERSION_CODES.JELLY_BEAN) {
            CameraInfo cameraInfo = new CameraInfo();
            Camera.getCameraInfo(mCameraIndex, cameraInfo);
            mIsCameraFrontFacing = 
                    (cameraInfo.facing ==
                    CameraInfo.CAMERA_FACING_FRONT);
            mNumCameras = Camera.getNumberOfCameras();
            camera = Camera.open(mCameraIndex);
        } else { // pre-Gingerbread
            // Assume there is only 1 camera and it is rear-facing.
            mIsCameraFrontFacing = false;
            mNumCameras = 1;
            camera = Camera.open();
        }
        final Parameters parameters = camera.getParameters();
        mCameraProjectionAdapter.setCameraParameters(
                parameters);
        camera.release();
    }
    
    @Override
    public void onSaveInstanceState(Bundle savedInstanceState) {
        // Save the current camera index.
        savedInstanceState.putInt(STATE_CAMERA_INDEX, mCameraIndex);
        
        // Save the current filter indices.
        savedInstanceState.putInt(STATE_IMAGE_DETECTION_FILTER_INDEX,
                mImageDetectionFilterIndex);

        super.onSaveInstanceState(savedInstanceState);
    }
    
    @Override
    public void onPause() {
        if (mCameraView != null) {
            mCameraView.disableView();
        }
        super.onPause();
    }
    
    @Override
    public void onResume() {
        super.onResume();
        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3,
                this, mLoaderCallback);
        mIsMenuLocked = false;
    }
    
    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mCameraView != null) {
            mCameraView.disableView();
        }
    }
    
    @Override
    public boolean onCreateOptionsMenu(final Menu menu) {
        getMenuInflater().inflate(R.menu.camera, menu);
        if (mNumCameras < 2) {
            // Remove the option to switch cameras, since there is
            // only 1.
            menu.removeItem(R.id.menu_next_camera);
        }
        return true;
    }
    
    @Override
    public boolean onOptionsItemSelected(final MenuItem item) {
        if (mIsMenuLocked) {
            return true;
        }
        switch (item.getItemId()) {
        case R.id.menu_next_image_detection_filter:
            mImageDetectionFilterIndex++;
            if (mImageDetectionFilterIndex ==
                    mImageDetectionFilters.length) {
            	mImageDetectionFilterIndex = 0;
            }
            mARRenderer.filter = mImageDetectionFilters[
                    mImageDetectionFilterIndex];
            return true;
        case R.id.menu_next_camera:
            mIsMenuLocked = true;
            
            // With another camera index, recreate the activity.
            mCameraIndex++;
            if (mCameraIndex == mNumCameras) {
                mCameraIndex = 0;
            }
            recreate();
            
            return true;
            
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    @Override
    public void onCameraViewStarted(final int width,
    		final int height) {
    }

    @Override
    public void onCameraViewStopped() {
    }

    @Override
    public Mat onCameraFrame(final CvCameraViewFrame inputFrame) {
        final Mat rgba = inputFrame.rgba();
        
        // Apply the active filters.
        if (mImageDetectionFilters != null) {
            mImageDetectionFilters[mImageDetectionFilterIndex].apply(
                    rgba, rgba);
        }
        
        if (mIsCameraFrontFacing) {
            // Mirror (horizontally flip) the preview.
            Core.flip(rgba, rgba, 1);
        }
        
        return rgba;
    }
}
