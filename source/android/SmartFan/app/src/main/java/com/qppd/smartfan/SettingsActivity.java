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
import com.google.android.material.snackbar.Snackbar;
import com.google.android.material.switchmaterial.SwitchMaterial;
import com.google.android.material.textfield.TextInputEditText;
import com.google.android.material.textfield.TextInputLayout;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.messaging.FirebaseMessaging;
import java.util.Locale;

public class SettingsActivity extends AppCompatActivity {
    private MaterialToolbar toolbar;
    private SwitchMaterial switchTheme, switchNotifications;
    private TextInputEditText editTextDeviceId;
    private TextInputLayout textInputLayoutDeviceId;
    private MaterialButton buttonSaveDeviceId;
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
        editTextDeviceId = findViewById(R.id.editTextDeviceId);
        textInputLayoutDeviceId = findViewById(R.id.textInputLayoutDeviceId);
        buttonSaveDeviceId = findViewById(R.id.buttonSaveDeviceId);
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

        // Device ID save button
        buttonSaveDeviceId.setOnClickListener(v -> saveDeviceId());

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

        // Load current device ID from user settings
        loadCurrentDeviceId();

        // Save FCM token to database
        FirebaseMessaging.getInstance().getToken().addOnCompleteListener(task -> {
            if (task.isSuccessful()) {
                String token = task.getResult();
                dbRef.child("smartfan").child("users").child(uid).child("fcmToken").setValue(token);
            }
        });
    }

    private void loadCurrentDeviceId() {
        dbRef.child("smartfan").child("users").child(uid).child("deviceId")
            .addListenerForSingleValueEvent(new ValueEventListener() {
                @Override
                public void onDataChange(DataSnapshot snapshot) {
                    String currentDeviceId = snapshot.getValue(String.class);
                    if (currentDeviceId != null && !currentDeviceId.isEmpty()) {
                        editTextDeviceId.setText(currentDeviceId);
                    } else {
                        // Set default device ID
                        editTextDeviceId.setText("SmartFan_ESP8266_001");
                    }
                }

                @Override
                public void onCancelled(DatabaseError error) {
                    showSnackbar("Failed to load device ID: " + error.getMessage(), false);
                    editTextDeviceId.setText("SmartFan_ESP8266_001");
                }
            });
    }

    private void saveDeviceId() {
        String deviceId = editTextDeviceId.getText().toString().trim();
        
        if (deviceId.isEmpty()) {
            textInputLayoutDeviceId.setError(getString(R.string.error_device_id_empty));
            return;
        }

        if (!deviceId.startsWith("SmartFan_ESP")) {
            textInputLayoutDeviceId.setError(getString(R.string.error_device_id_invalid));
            return;
        }

        textInputLayoutDeviceId.setError(null);

        // Save device ID to user profile
        dbRef.child("smartfan").child("users").child(uid).child("deviceId").setValue(deviceId)
            .addOnSuccessListener(aVoid -> {
                showSnackbar(getString(R.string.message_device_id_saved), true);
                // Optionally restart MainActivity to pick up new device ID
                showSnackbar("Please restart the app to connect to the new device", true);
            })
            .addOnFailureListener(e -> {
                showSnackbar("Failed to save device ID: " + e.getMessage(), false);
            });
    }

    private void showSnackbar(String message, boolean isSuccess) {
        Snackbar snackbar = Snackbar.make(findViewById(android.R.id.content), message, Snackbar.LENGTH_SHORT);
        if (!isSuccess) {
            snackbar.setBackgroundTint(getColor(R.color.accent_red));
        }
        snackbar.show();
    }
}
