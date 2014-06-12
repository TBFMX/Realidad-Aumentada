package com.tbf.glsurface;

import android.content.res.AssetManager;

public class LibraryClass {
	static 
	{
		System.loadLibrary("demo");
	}

	public static native void init();
	public static native void resize(int width, int height);
	public static native void render(float[] projection, float[] pose);
	public static native void videorender(float[] projection, float[] pose);
	public static native void initializeAssetManager(AssetManager assetManager);
}
