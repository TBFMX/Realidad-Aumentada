package com.tbf.demotamarindo;

public class Native {
	public static void loadlibs() {
		System.loadLibrary("opencv_java"); // this library was manually copied to armeabi folder
		System.loadLibrary("NativeCamera");
	}

	public static native void initCamera(int width, int height);

	public static native void releaseCamera();

	public static native void renderBackground();
	
	public static native void renderFurnish();

	public static native void surfaceChanged(int width, int height, int orientation);
}
