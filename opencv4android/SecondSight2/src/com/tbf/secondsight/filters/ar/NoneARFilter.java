package com.tbf.secondsight.filters.ar;

import com.tbf.secondsight.filters.NoneFilter;

public class NoneARFilter extends NoneFilter implements ARFilter {
    @Override
    public float[] getGLPose() {
        return null;
    }
}
