package com.tbf.cameragl;

import org.opencv.core.Size;

import android.content.Context;
import android.graphics.PointF;
import android.view.MotionEvent;
import android.view.SurfaceView;

public class ResponsiveSurfaceView extends SurfaceView {
	private Size mSize;
	//private CameraRenderer mRenderer;
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
    
    public ResponsiveSurfaceView(Context context,Size size) {
		super(context);
		this.mSize = size;
		//mRenderer = new CameraRenderer(context, size);
		//setRenderer(mRenderer);
	}

	public boolean onTouchEvent(MotionEvent event) {
        // handle touch events here
		Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
        switch (event.getAction() & MotionEvent.ACTION_MASK) {
            case MotionEvent.ACTION_DOWN:
                //savedMatrix.set(matrix);
                start.set(event.getX(), event.getY());
                mode = DRAG;
                lastEvent = null;
                mCumulativeTranslateX = mCumulativeTranslateX + mTranslateX;
                mCumulativeTranslateY = mCumulativeTranslateY + mTranslateY;
                //Toast.makeText(getContext(), "down action", Toast.LENGTH_SHORT).show();;
                break;
            case MotionEvent.ACTION_POINTER_DOWN:
                oldDist = spacing(event);
                mCumulativeScale = mCumulativeScale * mScale;
                if(mCumulativeScale < 1.0)
                	mCumulativeScale = (float) 1.0;
                if (oldDist > 40f) {
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
                if(mCumulativeAngle > 360)
                	mCumulativeAngle = mCumulativeAngle - 360;
                else if(mCumulativeAngle < -360)
                	mCumulativeAngle = mCumulativeAngle + 360;
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
                   
                    float dx = (float) ((event.getX() - start.x)/mSize.width);
                    float dy = (float) ((event.getY() - start.y)/mSize.height);

                    mTranslateX = dx;
	                mTranslateY = dy;
  
                    setTranslate(mCumulativeTranslateX + mTranslateX, mCumulativeTranslateY + mTranslateY);
                    //String aPosition = "start " + start.x +", " + start.y + " final " +event.getX() + ", " + event.getY();
//                    String aPosition = "dx dy " + dx +", " + dy;
//                    Toast.makeText(getContext(), aPosition, Toast.LENGTH_SHORT).show();
                    
                } else if (mode == ZOOM) {
                    float newDist = spacing(event);
                    if (newDist > 40f) {
                        //matrix.set(savedMatrix);
                        mScale = (newDist / oldDist);
                        //matrix.postScale(scale, scale, mid.x, mid.y);
                        setScale(mCumulativeScale * mScale);
                        //String aScale = "scale " + mRenderer.getScale();
                        //Toast.makeText(getContext(), aScale, Toast.LENGTH_SHORT).show();
                    }
//                    if (lastEvent != null && event.getPointerCount() == 3) {
                    if (lastEvent != null){
                        newRot = rotation(event);
                    	//newRot = rotationDirection(event);
                        mAngle = newRot - d;
                        setAngle(mCumulativeAngle + mAngle);
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
        return (float) Math.sqrt(x * x + y * y);
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

	public void backToDefaults() {
	    mAngle = 0;
	    mCumulativeAngle = 0;
	    mScale = 1;
	    mCumulativeScale = 1;
	    mTranslateX = 0;
	    mTranslateY = 0;
	    mCumulativeTranslateX = 0;
	    mCumulativeTranslateY = 0;
	    setScale(mCumulativeScale*mScale);
	    setTranslate(mCumulativeTranslateX + mTranslateX, mCumulativeTranslateY + mTranslateY);
	    setAngle(mCumulativeAngle + mAngle);
	}
	
	public void changeShownModel() {
		// TODO Auto-generated method stub
		Native.changeShownModel();
	}

	public float getAngle() {
		// TODO Auto-generated method stub
		return Native.getAngle();
	}

	public void setAngle(float angle) {
		// TODO Auto-generated method stub
		Native.setAngle(angle);
	}

	public void setScale(float scale) {
		// TODO Auto-generated method stub
		Native.setScale(scale);
	}

	public float getScale() {
		// TODO Auto-generated method stub
		return Native.getScale();
	}

	public void setTranslate(float x, float y) {
		// TODO Auto-generated method stub
		Native.setTranslate(x,y);
	}

	public void changeIlluminationModel() {
		// TODO Auto-generated method stub
		Native.changeIlluminationModel();
	}
}