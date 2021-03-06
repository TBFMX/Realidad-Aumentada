package com.tbf.glsurface.filters.ar;

import java.io.IOException;

import org.opencv.core.Mat;
import org.opencv.imgproc.Imgproc;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;

import com.tbf.glsurface.adapters.CameraProjectionAdapter;

public class VideoDetectionFilter extends ImageDetectionFilter {
	public VideoDetectionFilter(Context context, int referenceImageResourceID,
			CameraProjectionAdapter cameraProjectionAdapter) throws IOException {
		super(context, referenceImageResourceID, cameraProjectionAdapter);
	}
    protected void draw(Mat src, Mat dst) {
        
        if (dst != src) {
            src.copyTo(dst);
        }
        
        if (!mTargetFound) {
            // The target has not been found.
            
            // Draw a thumbnail of the target in the upper-left
            // corner so that the user knows what it is.
            
            int height = mReferenceImage.height();
            int width = mReferenceImage.width();
            int maxDimension = Math.min(dst.width(), dst.height()) / 2;
            double aspectRatio = width / (double)height;
            if (height > width) {
                height = maxDimension;
                width = (int)(height * aspectRatio);
            } else {
                width = maxDimension;
                height = (int)(width / aspectRatio);
            }
            Mat dstROI = dst.submat(0, height, 0, width);
            Imgproc.resize(mReferenceImage, dstROI, dstROI.size(), 0.0, 0.0, Imgproc.INTER_AREA);

//			try {
//	            InputStream input;
//	            int size;
//				input = mAssetManager.open("someToast.txt");
//				size = input.available();
//				byte[] buffer = new byte[size];
//				input.read(buffer);
//	            input.close();
//	            final String someText = new String(buffer);
//	            
//	        	Handler handler = new Handler(Looper.getMainLooper());
//	        	handler.post(new Runnable() {
//	        	     public void run() {
//	        	    	 mToast.setText(someText);
//	        	    	 mToast.show();
//	        	     }
//	        	});
//			} catch (IOException e) {
//				e.printStackTrace();
//			}
        } else {
        	
        	Handler handler = new Handler(Looper.getMainLooper());
        	handler.post(new Runnable() {
        	     public void run() {
        	          // UI code goes here
        	    	 //Toast.makeText(mContext, "marcador detectado", Toast.LENGTH_SHORT).show();
        	    	 mToast.setText("marcador detectado");
        	    	 mToast.show();

        	     }
        	});
        }
    }
	
}
