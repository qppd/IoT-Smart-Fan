package com.qppd.smartfan;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.widget.Button;
import android.content.Intent;
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AppCompatDelegate;
import com.google.android.material.appbar.MaterialToolbar;
import com.google.android.material.button.MaterialButton;
import com.google.android.material.slider.Slider;
import com.google.android.material.snackbar.Snackbar;
import com.google.android.material.switchmaterial.SwitchMaterial;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.messaging.FirebaseMessaging;
import java.util.Locale;

public class SettingsActivity extends AppCompatActivity {
    private MaterialToolbar toolbar;
    private SwitchMaterial switchTheme, switchNotifications;
    private Slider sliderMinTemp, sliderMaxTemp;
    private TextView textViewMinTemp, textViewMaxTemp;
    private MaterialButton buttonSave, buttonManageDevices;
    private DatabaseReference dbRef;
    private String uid;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        initializeViews();
        setupToolbar();
        initializeData();
        setupListeners();
        loadSettings();
    }

    private void initializeViews() {
        toolbar = findViewById(R.id.toolbar);
        switchTheme = findViewById(R.id.switchTheme);
        switchNotifications = findViewById(R.id.switchNotifications);
        sliderMinTemp = findViewById(R.id.sliderMinTemp);
        sliderMaxTemp = findViewById(R.id.sliderMaxTemp);
        textViewMinTemp = findViewById(R.id.textViewMinTemp);
        textViewMaxTemp = findViewById(R.id.textViewMaxTemp);
        buttonSave = findViewById(R.id.buttonSaveSettings);
        buttonManageDevices = findViewById(R.id.buttonManageDevices);
    }

    private void setupToolbar() {
        setSupportActionBar(toolbar);
        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
            getSupportActionBar().setDisplayShowHomeEnabled(true);
        }
        toolbar.setNavigationOnClickListener(v -> onBackPressed());
    }

    private void initializeData() {
        dbRef = FirebaseDatabase.getInstance().getReference();
        uid = FirebaseAuth.getInstance().getCurrentUser().getUid();
    }

    private void setupListeners() {
        // Theme switch
        switchTheme.setOnCheckedChangeListener((buttonView, isChecked) -> {
            AppCompatDelegate.setDefaultNightMode(isChecked ? AppCompatDelegate.MODE_NIGHT_YES : AppCompatDelegate.MODE_NIGHT_NO);
            SharedPreferences prefs = getSharedPreferences("settings", MODE_PRIVATE);
            prefs.edit().putBoolean("dark_mode", isChecked).apply();
            showSnackbar("Theme updated", true);
        });

        // Temperature sliders
        sliderMinTemp.addOnChangeListener((slider, value, fromUser) -> {
            textViewMinTemp.setText(String.format(Locale.getDefault(), "%.0f°C", value));
            if (value >= sliderMaxTemp.getValue()) {
                sliderMaxTemp.setValue(value + 1);
            }
        });

        sliderMaxTemp.addOnChangeListener((slider, value, fromUser) -> {
            textViewMaxTemp.setText(String.format(Locale.getDefault(), "%.0f°C", value));
            if (value <= sliderMinTemp.getValue()) {
                sliderMinTemp.setValue(value - 1);
            }
        });

        // Save button
        buttonSave.setOnClickListener(v -> saveSettings());

        // Manage devices button
        buttonManageDevices.setOnClickListener(v -> {
            startActivity(new Intent(SettingsActivity.this, com.qppd.smartfan.device.DeviceManagementActivity.class));
        });

        // Notifications switch
        switchNotifications.setOnCheckedChangeListener((buttonView, isChecked) -> {
            // Handle notification preferences
            showSnackbar(isChecked ? "Notifications enabled" : "Notifications disabled", true);
        });
    }

    private void loadSettings() {
        // Load theme preference
        SharedPreferences prefs = getSharedPreferences("settings", MODE_PRIVATE);
        boolean darkMode = prefs.getBoolean("dark_mode", false);
        switchTheme.setChecked(darkMode);
        AppCompatDelegate.setDefaultNightMode(darkMode ? AppCompatDelegate.MODE_NIGHT_YES : AppCompatDelegate.MODE_NIGHT_NO);

        // Load temperature settings from Firebase or set defaults
        sliderMinTemp.setValue(20f);
        sliderMaxTemp.setValue(35f);
        textViewMinTemp.setText("20°C");
        textViewMaxTemp.setText("35°C");

        // Save FCM token to database
        FirebaseMessaging.getInstance().getToken().addOnCompleteListener(task -> {
            if (task.isSuccessful()) {
                String token = task.getResult();
                dbRef.child("users").child(uid).child("fcmToken").setValue(token);
            }
        });
    }

    private void saveSettings() {
        int minTemp = (int) sliderMinTemp.getValue();
        int maxTemp = (int) sliderMaxTemp.getValue();

        if (minTemp >= maxTemp) {
            showSnackbar("Minimum temperature must be less than maximum", false);
            return;
        }

        if (minTemp < 0 || maxTemp > 100) {
            showSnackbar("Temperature thresholds must be between 0°C and 100°C", false);
            return;
        }

        // Save to Firebase
        dbRef.child("users").child(uid).child("settings").child("tempMin").setValue(minTemp);
        dbRef.child("users").child(uid).child("settings").child("tempMax").setValue(maxTemp);
        
        showSnackbar(getString(R.string.message_settings_saved), true);
    }

    private void showSnackbar(String message, boolean isSuccess) {
        Snackbar snackbar = Snackbar.make(findViewById(android.R.id.content), message, Snackbar.LENGTH_SHORT);
        if (!isSuccess) {
            snackbar.setBackgroundTint(getColor(R.color.accent_red));
        }
        snackbar.show();
    }
}
