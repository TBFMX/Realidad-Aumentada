package com.tbf.cameragl;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import org.opencv.core.Size;

import android.content.Context;
import android.graphics.Matrix;
import android.graphics.PointF;
import android.opengl.GLSurfaceView;
import android.util.FloatMath;
import android.view.MotionEvent;
import android.widget.Toast;

public class ResponsiveGLSurfaceView extends GLSurfaceView {

	private CameraRenderer mRenderer;
    // these matrices will be used to move and zoom image
    //private Matrix matrix = new Matrix();
    //private Matrix savedMatrix = new Matrix();
    private float mAngle = 0;
    private float mCumulativeAngle = 0;
    private float mScale = 1;
    private float mCumulativeScale = 1;
    private float mTranslateX = 0;
    private float mTranslateY = 0;
    private float mCumulativeTranslateX = 0;
    private float mCumulativeTranslateY = 0;
    // we can be in one of these 3 states
    private static final int NONE = 0;
    private static final int DRAG = 1;
    private static final int ZOOM = 2;
    private int mode = NONE;
    // remember some things for zooming
    private PointF start = new PointF();
    //private PointF mid = new PointF();
    private float oldDist = 1f;
    private float d = 0f;
    private float newRot = 0f;
    private float[] lastEvent = null;
    private boolean isNewDrag = false;

	public ResponsiveGLSurfaceView(Context context,Size size) {
		super(context);
		mRenderer = new CameraRenderer(context, size);
		setRenderer(mRenderer);
	}

	public boolean onTouchEvent(MotionEvent event) {
        // handle touch events here
        
        switch (event.getAction() & MotionEvent.ACTION_MASK) {
            case MotionEvent.ACTION_DOWN:
                //savedMatrix.set(matrix);
                start.set(event.getX(), event.getY());
                mode = DRAG;
                lastEvent = null;
                isNewDrag = true;
                mCumulativeTranslateX = mCumulativeTranslateX + mTranslateX;
                mCumulativeTranslateY = mCumulativeTranslateY + mTranslateY;
                //Toast.makeText(getContext(), "down action", Toast.LENGTH_SHORT).show();;
                break;
            case MotionEvent.ACTION_POINTER_DOWN:
                oldDist = spacing(event);
                mCumulativeScale = mCumulativeScale * mScale;
                if (oldDist > 10f) {
                    //savedMatrix.set(matrix);
                    //midPoint(mid, event);
                    mode = ZOOM;
                }
                lastEvent = new float[4];
                lastEvent[0] = event.getX(0);
                lastEvent[1] = event.getX(1);
                lastEvent[2] = event.getY(0);
                lastEvent[3] = event.getY(1);
                d = rotation(event);
                mCumulativeAngle = mCumulativeAngle + mAngle;
                //d = rotationDirection(event);
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_POINTER_UP:
                mode = NONE;
                lastEvent = null;
                //Toast.makeText(getContext(), "pointer up up action", Toast.LENGTH_SHORT).show();
                break;
            case MotionEvent.ACTION_MOVE:
                if (mode == DRAG) {
                   
                    float dx = (event.getX() - start.x)/mRenderer.getWidth();
                    float dy = (event.getY() - start.y)/mRenderer.getHeight();

                    mTranslateX = dx;
	                mTranslateY = dy;
  
                    mRenderer.setTranslate(mCumulativeTranslateX + mTranslateX, mCumulativeTranslateY + mTranslateY);
                    //String aPosition = "start " + start.x +", " + start.y + " final " +event.getX() + ", " + event.getY();
//                    String aPosition = "dx dy " + dx +", " + dy;
//                    Toast.makeText(getContext(), aPosition, Toast.LENGTH_SHORT).show();
                    
                } else if (mode == ZOOM) {
                    float newDist = spacing(event);
                    if (newDist > 10f) {
                        //matrix.set(savedMatrix);
                        mScale = (newDist / oldDist);
                        //matrix.postScale(scale, scale, mid.x, mid.y);
                        mRenderer.setScale(mCumulativeScale*mScale);
                        //String aScale = "scale " + mRenderer.getScale();
                        //Toast.makeText(getContext(), aScale, Toast.LENGTH_SHORT).show();
                    }
//                    if (lastEvent != null && event.getPointerCount() == 3) {
                    if (lastEvent != null){
                        newRot = rotation(event);
                    	//newRot = rotationDirection(event);
                        mAngle = newRot - d;
                        mRenderer.setAngle(mCumulativeAngle + mAngle);
                        //String aAngle = "angle " + newRot;
                        //Toast.makeText(getContext(), aAngle, Toast.LENGTH_SHORT).show();
                    }
                }
                //Toast.makeText(getContext(), "move action", Toast.LENGTH_SHORT).show();
                break;
        }

        // //view.setImageMatrix(matrix);
        //String aMode = "mode " + mode;
        //Toast.makeText(getContext(), aMode, Toast.LENGTH_SHORT).show();
        return true;
    }

    /**
* Determine the space between the first two fingers
*/
    private float spacing(MotionEvent event) {
        float x = event.getX(0) - event.getX(1);
        float y = event.getY(0) - event.getY(1);
        return FloatMath.sqrt(x * x + y * y);
    }

    /**
* Calculate the mid point of the first two fingers
*/
    private void midPoint(PointF point, MotionEvent event) {
        float x = event.getX(0) + event.getX(1);
        float y = event.getY(0) + event.getY(1);
        point.set(x / 2, y / 2);
    }

    /**
* Calculate the degree to be rotated by.
*
* @param event
* @return Degrees
*/
    private float rotation(MotionEvent event) {
        double delta_x = (event.getX(0) - event.getX(1));
        double delta_y = (event.getY(0) - event.getY(1));
        double radians = Math.atan2(delta_y, delta_x);
//        String aAngle = "dx dy " + delta_x + ", " +delta_y;
//        Toast.makeText(getContext(), aAngle, Toast.LENGTH_SHORT).show();
        return (float) Math.toDegrees(radians);
    }
    
    private float rotationDirection(MotionEvent event) {
        double x0 = event.getX(0);
        double x1 = event.getX(1);
        double y0 = event.getY(0);
        double y1 = event.getY(1);
        float height = mRenderer.getHeight();
        float width = mRenderer.getWidth();
        float midX = width/2;
        float midY = height/2;
        float degrees = 0;
        // right rotation
//        if(x1 < midX){
//        	if(y1 < midY){
//        		if(y0 < midY )
//        			degrees = (float )(90.0 * (x1 - x0) / width); // negative
//        		else if(y0 > midY)
//        			degrees = (float )(90.0 * (y1 - y0) / height); // positive		
//        	}else if(y1 > midY){
//        		if(x0 < midX )
//        			degrees = (float )(90.0 * (y1 - y0) / height); // positive
//        		else if(x0 > midX)
//        			degrees = (float )(90.0 * (x1 - x0) / width); // negative
//        	}
//        } else if(x1 > midX){
//        	if(y1 < midY){
//        		if(y0 > midY)
//        			degrees = (float )(90.0 * (y1 - y0) / height); // negative
//        		else if(x0 < midX)
//        			degrees = (float )(90.0 * (x0 - x1) / width);  // positive      			
//        	}else if(y1 > midY){
//        		if(x0 < midX )
//        			degrees = (float )(90.0 * (x0 - x1) / width);
//        		else if(x0 > midX)
//        			degrees = (float )(90.0 * (y0 - y1) / height); 
//        	}       	
//        }
        
        if(x1 > midX || x0 > midX)
        	degrees = 1;
        else if (x1 < midX || x0 < midX)
        	degrees = -1;
        	
        String aAngle = "degrees " + degrees;
        Toast.makeText(getContext(), aAngle, Toast.LENGTH_SHORT).show();
        return degrees;
    }
	
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		mRenderer.onSurfaceCreated(gl, config);
	}

	public void onSurfaceChanged(GL10 gl, int width, int height) {
		mRenderer.onSurfaceChanged(gl, width, height);
	}

	public void onDrawFrame(GL10 gl) {
		// TODO Auto-generated method stub
		mRenderer.onDrawFrame(gl);
	}

	public void changeShownModel() {
		// TODO Auto-generated method stub
		mRenderer.changeShownModel();
	}
	
	public CameraRenderer getCameraRenderer(){
		return mRenderer;
	}

	public void backToDefaults() {
	    mAngle = 0;
	    mCumulativeAngle = 0;
	    mScale = 1;
	    mCumulativeScale = 1;
	    mTranslateX = 0;
	    mTranslateY = 0;
	    mCumulativeTranslateX = 0;
	    mCumulativeTranslateY = 0;
	    mRenderer.setScale(mCumulativeScale*mScale);
	    mRenderer.setTranslate(mCumulativeTranslateX + mTranslateX, mCumulativeTranslateY + mTranslateY);
	    mRenderer.setAngle(mCumulativeAngle + mAngle);
	}

	public void changeIlluminationModel() {
		// TODO Auto-generated method stub
		mRenderer.changeIlluminationModel();
	}
}
