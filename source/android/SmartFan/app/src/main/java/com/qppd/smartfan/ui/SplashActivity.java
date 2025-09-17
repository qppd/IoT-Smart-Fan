package com.qppd.smartfan.ui;

import android.animation.ObjectAnimator;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.widget.ImageView;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.splashscreen.SplashScreen;
import com.qppd.smartfan.R;
import com.qppd.smartfan.auth.LoginActivity;

public class SplashActivity extends AppCompatActivity {
    
    private static final int SPLASH_DELAY = 2000; // 2 seconds
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // Install splash screen
        SplashScreen splashScreen = SplashScreen.installSplashScreen(this);
        
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_splash);
        
        // Animate the logo
        ImageView logoImage = findViewById(R.id.imageViewLogo);
        animateLogo(logoImage);
        
        // Navigate to login after delay
        new Handler(Looper.getMainLooper()).postDelayed(() -> {
            Intent intent = new Intent(SplashActivity.this, LoginActivity.class);
            startActivity(intent);
            finish();
            overridePendingTransition(android.R.anim.fade_in, android.R.anim.fade_out);
        }, SPLASH_DELAY);
    }
    
    private void animateLogo(ImageView logoImage) {
        // Rotate animation for fan logo
        ObjectAnimator rotateAnimator = ObjectAnimator.ofFloat(logoImage, "rotation", 0f, 360f);
        rotateAnimator.setDuration(1500);
        rotateAnimator.setInterpolator(new AccelerateDecelerateInterpolator());
        rotateAnimator.setRepeatCount(1);
        rotateAnimator.start();
        
        // Scale animation
        ObjectAnimator scaleXAnimator = ObjectAnimator.ofFloat(logoImage, "scaleX", 0.8f, 1.1f, 1.0f);
        ObjectAnimator scaleYAnimator = ObjectAnimator.ofFloat(logoImage, "scaleY", 0.8f, 1.1f, 1.0f);
        scaleXAnimator.setDuration(1000);
        scaleYAnimator.setDuration(1000);
        scaleXAnimator.setInterpolator(new AccelerateDecelerateInterpolator());
        scaleYAnimator.setInterpolator(new AccelerateDecelerateInterpolator());
        scaleXAnimator.start();
        scaleYAnimator.start();
    }
}
