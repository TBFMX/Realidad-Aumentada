package com.tbf.jnitester;

import org.opencv.core.Mat;

public class FrameProcessor {
		public FrameProcessor() {
	        mNativeObj = nativeCreateObject();
	    }

	    public void start() {
	        nativeStart(mNativeObj);
	    }

	    public void stop() {
	        nativeStop(mNativeObj);
	    }

	    public void findFeatures(Mat matAddrGr, Mat matAddrRgba) {
	        nativeFindFeatures(mNativeObj, matAddrGr.getNativeObjAddr(), matAddrRgba.getNativeObjAddr());
	    }
	    
	    public void cartoonify(Mat matAddrSrc, Mat matAddrDst) {
	        nativeCartoonify(mNativeObj,matAddrSrc.getNativeObjAddr(), matAddrDst.getNativeObjAddr());
	    }
	    
	    public void augment(Mat matAddrSrc, Mat matAddrDst, Mat matAddrMarker) {
	        nativeMarker(mNativeObj,matAddrSrc.getNativeObjAddr(), matAddrDst.getNativeObjAddr(), matAddrMarker.getNativeObjAddr());
	    }
	    
	    public void release() {
	        nativeDestroyObject(mNativeObj);
	        mNativeObj = 0;
	    }

	    private long mNativeObj = 0;

	    private static native long nativeCreateObject();
	    private static native void nativeDestroyObject(long thiz);
	    private static native void nativeStart(long thiz);
	    private static native void nativeStop(long thiz);
	    private static native void nativeFindFeatures(long thiz, long matAddrGr, long matAddrRgba);
	    private static native void nativeCartoonify(long thiz, long matAddrSrc, long matAddrDst);
	    private static native void nativeMarker(long thiz, long matAddrSrc, long matAddrDst, long matAddrMarker);
}
