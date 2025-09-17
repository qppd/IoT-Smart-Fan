package com.qppd.smartfan.device;

import android.animation.ObjectAnimator;
import android.animation.ValueAnimator;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.text.TextUtils;
import android.view.View;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import com.google.android.material.button.MaterialButton;
import com.google.android.material.card.MaterialCardView;
import com.google.android.material.progressindicator.CircularProgressIndicator;
import com.google.android.material.snackbar.Snackbar;
import com.google.android.material.textfield.TextInputEditText;
import com.google.android.material.textfield.TextInputLayout;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;
import com.qppd.smartfan.MainActivity;
import com.qppd.smartfan.R;

public class SetupDeviceActivity extends AppCompatActivity {
    private MaterialButton buttonWiFiSetup;
    private CircularProgressIndicator progressBar;
    private LinearLayout layoutProgress;
    private TextView textViewProgress;
    private MaterialCardView cardWiFiSetup;
    private ImageView imageViewLogo;
    
    private DatabaseReference dbRef;
    private FirebaseUser user;
    private Handler animationHandler = new Handler();

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_setup_device);

        initializeViews();
        initializeFirebase();
        setupClickListeners();
        startEntryAnimations();
    }

    private void initializeViews() {
        buttonWiFiSetup = findViewById(R.id.buttonWiFiSetup);
        progressBar = findViewById(R.id.progressBar);
        layoutProgress = findViewById(R.id.layoutProgress);
        textViewProgress = findViewById(R.id.textViewProgress);
        cardWiFiSetup = findViewById(R.id.cardWiFiSetup);
        imageViewLogo = findViewById(R.id.imageViewLogo);
    }

    private void initializeFirebase() {
        dbRef = FirebaseDatabase.getInstance().getReference();
        user = FirebaseAuth.getInstance().getCurrentUser();
    }

    private void setupClickListeners() {
        buttonWiFiSetup.setOnClickListener(v -> {
            animateButtonPress(buttonWiFiSetup);
            startWiFiSetup();
        });

        findViewById(R.id.textViewHelp).setOnClickListener(v -> showHelpDialog());
    }

    private void startEntryAnimations() {
        // Fade in logo with rotation
        imageViewLogo.setAlpha(0f);
        imageViewLogo.setScaleX(0.5f);
        imageViewLogo.setScaleY(0.5f);
        imageViewLogo.animate()
                .alpha(1f)
                .scaleX(1f)
                .scaleY(1f)
                .setDuration(800)
                .setInterpolator(new AccelerateDecelerateInterpolator())
                .start();

        // Slide in cards from bottom with stagger
        animateCardEntry(cardWiFiSetup, 200);
    }

    private void animateCardEntry(MaterialCardView card, long delay) {
        card.setTranslationY(300f);
        card.setAlpha(0f);
        card.animate()
                .translationY(0f)
                .alpha(1f)
                .setDuration(600)
                .setStartDelay(delay)
                .setInterpolator(new AccelerateDecelerateInterpolator())
                .start();
    }

    private void animateButtonPress(MaterialButton button) {
        button.animate()
                .scaleX(0.95f)
                .scaleY(0.95f)
                .setDuration(100)
                .withEndAction(() -> {
                    button.animate()
                            .scaleX(1f)
                            .scaleY(1f)
                            .setDuration(100)
                            .start();
                })
                .start();
    }

    private void showProgress(String message) {
        textViewProgress.setText(message);
        layoutProgress.setVisibility(View.VISIBLE);
        layoutProgress.setAlpha(0f);
        layoutProgress.animate()
                .alpha(1f)
                .setDuration(300)
                .start();
    }

    private void hideProgress() {
        layoutProgress.animate()
                .alpha(0f)
                .setDuration(300)
                .withEndAction(() -> layoutProgress.setVisibility(View.GONE))
                .start();
    }

    private void showSuccessSnackbar(String message) {
        Snackbar snackbar = Snackbar.make(findViewById(android.R.id.content), message, Snackbar.LENGTH_LONG);
        snackbar.setBackgroundTint(getColor(R.color.accent_green));
        snackbar.setTextColor(getColor(R.color.neutral_white));
        snackbar.show();
    }

    private void showErrorSnackbar(String message) {
        Snackbar snackbar = Snackbar.make(findViewById(android.R.id.content), message, Snackbar.LENGTH_LONG);
        snackbar.setBackgroundTint(getColor(R.color.accent_red));
        snackbar.setTextColor(getColor(R.color.neutral_white));
        snackbar.show();
    }

    private void showHelpDialog() {
        new androidx.appcompat.app.AlertDialog.Builder(this)
                .setTitle("Device Setup Help")
                .setMessage("To set up your Smart Fan:\n\n" +
                           "1. Make sure your Smart Fan is powered on\n" +
                           "2. Put the device in setup mode (hold reset button for 5 seconds)\n" +
                           "3. Tap 'Setup WiFi' to configure network settings\n" +
                           "4. Follow the on-screen instructions\n\n" +
                           "Your device will appear as 'SmartFan-XXXXXX' in the WiFi list.")
                .setPositiveButton("Got it", null)
                .setIcon(R.drawable.ic_info)
                .show();
    }

    private void startWiFiSetup() {
        Intent intent = new Intent(this, WiFiSetupActivity.class);
        startActivity(intent);
    }
}