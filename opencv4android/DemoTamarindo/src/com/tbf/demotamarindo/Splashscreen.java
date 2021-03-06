package com.tbf.demotamarindo;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.widget.ImageView;

public class Splashscreen extends Activity {
	  private ImageView splashImageView;
	    boolean splashloading = false;

	    @Override
	    protected void onCreate(Bundle savedInstanceState) {
	        super.onCreate(savedInstanceState);
	        splashImageView = new ImageView(this);
	        //splashImageView.setScaleType(ScaleType.FIT_XY);
	        splashImageView.setImageResource(R.drawable.tamarindo);
	        setContentView(splashImageView);
	        splashloading = true;
	        Handler h = new Handler();
	        h.postDelayed(new Runnable() {
	            public void run() {
	            	Intent i = new Intent(Splashscreen.this, CameraPreviewer.class);
	                startActivity(i);
	                finish();
	            }

	        }, 5000);

	    }
}
