package com.tbf.cameragl;

public class Native {
	public static void loadlibs() {
		System.loadLibrary("opencv_java"); // this library was manually copied to armeabi folder
		System.loadLibrary("NativeCamera");
	}

	public static native void initCamera(int width, int height);

	public static native void releaseCamera();

	public static native void renderBackground();

	public static native void surfaceChanged(int width, int height, int orientation);

	public static native void changeShownModel();

	public static native float getAngle();

	public static native void setAngle(float angle);

	public static native void setScale(float scale);

	public static native float getScale();

	public static native void setTranslate(float x, float y);

	public static native void changeIlluminationModel();
}
