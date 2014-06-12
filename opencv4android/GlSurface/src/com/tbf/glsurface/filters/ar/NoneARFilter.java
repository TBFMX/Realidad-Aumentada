package com.tbf.glsurface.filters.ar;

import com.tbf.glsurface.filters.NoneFilter;

public class NoneARFilter extends NoneFilter implements ARFilter {
    @Override
    public float[] getGLPose() {
        return null;
    }
}
