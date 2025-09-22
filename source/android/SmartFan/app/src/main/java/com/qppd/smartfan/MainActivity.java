package com.qppd.smartfan;

import androidx.appcompat.app.AppCompatActivity;
import android.animation.ObjectAnimator;
import android.animation.ValueAnimator;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import androidx.appcompat.app.AlertDialog;
import androidx.coordinatorlayout.widget.CoordinatorLayout;
import com.google.android.material.appbar.MaterialToolbar;
import com.google.android.material.bottomnavigation.BottomNavigationView;
import com.google.android.material.button.MaterialButton;
import com.google.android.material.card.MaterialCardView;
import com.google.android.material.chip.Chip;
import com.google.android.material.slider.Slider;
import com.google.android.material.snackbar.Snackbar;
import com.google.android.material.switchmaterial.SwitchMaterial;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.database.*;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class MainActivity extends AppCompatActivity {
    private FirebaseAuth mAuth;
    private DatabaseReference dbRef;
    private String uid;

    // UI Components
    private CoordinatorLayout coordinatorLayout;
    private MaterialToolbar toolbar;
    private BottomNavigationView bottomNavigation;
    
    // Dashboard Components
    private MaterialCardView cardDeviceStatus, cardTemperature, cardFanControl, cardQuickActions;
    private MaterialCardView cardPowerMonitoring, cardEnergyConsumption; // New power monitoring cards
    private ProgressBar progressBarTemperature;
    private TextView textViewGaugeTemp;
    private TextView textViewDeviceStatus, textViewLastUpdated, textViewCurrentTemp, textViewFanStatus, textViewFanSpeedLabel;
    private TextView textViewVoltage, textViewCurrent, textViewWatt, textViewKwh; // New power monitoring TextViews
    private ImageView imageViewDeviceStatus, imageViewTempIcon, imageViewFanIcon;
    private ImageView imageViewVoltage, imageViewCurrent, imageViewWatt, imageViewKwh; // New power monitoring Icons
    private SwitchMaterial switchAutoMode;
    private Slider sliderFanSpeed;
    private Chip chipTemperatureStatus, chipPowerStatus; // New power status chip
    private MaterialButton buttonHistory, buttonSettings;
    
    // Animation
    private ObjectAnimator fanRotationAnimator;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initializeAuth();
        if (!checkUserAuthentication()) return;
        
        initializeViews();
        setupToolbar();
        setupBottomNavigation();
        setupDashboard();
    }

    private void initializeAuth() {
        mAuth = FirebaseAuth.getInstance();
        dbRef = FirebaseDatabase.getInstance().getReference();
        if (mAuth.getCurrentUser() != null) {
            uid = mAuth.getCurrentUser().getUid();
        }
    }

    private boolean checkUserAuthentication() {
        if (mAuth.getCurrentUser() == null) {
            startActivity(new Intent(MainActivity.this, com.qppd.smartfan.auth.LoginActivity.class));
            finish();
            return false;
        }
        return true;
    }

    private void initializeViews() {
        coordinatorLayout = findViewById(R.id.coordinatorLayout);
        toolbar = findViewById(R.id.toolbar);
        bottomNavigation = findViewById(R.id.bottomNavigation);
        
        // Cards
        cardDeviceStatus = findViewById(R.id.cardDeviceStatus);
        cardTemperature = findViewById(R.id.cardTemperature);
        cardFanControl = findViewById(R.id.cardFanControl);
        cardQuickActions = findViewById(R.id.cardQuickActions);
        cardPowerMonitoring = findViewById(R.id.cardPowerMonitoring);
        cardEnergyConsumption = findViewById(R.id.cardEnergyConsumption);
        
        // Dashboard components
        progressBarTemperature = findViewById(R.id.progressBarTemperature);
        textViewGaugeTemp = findViewById(R.id.textViewGaugeTemp);
        textViewDeviceStatus = findViewById(R.id.textViewDeviceStatus);
        textViewLastUpdated = findViewById(R.id.textViewLastUpdated);
        textViewCurrentTemp = findViewById(R.id.textViewCurrentTemp);
        textViewFanStatus = findViewById(R.id.textViewFanStatus);
        textViewFanSpeedLabel = findViewById(R.id.textViewFanSpeedLabel);
        
        // Power monitoring components
        textViewVoltage = findViewById(R.id.textViewVoltage);
        textViewCurrent = findViewById(R.id.textViewCurrent);
        textViewWatt = findViewById(R.id.textViewWatt);
        textViewKwh = findViewById(R.id.textViewKwh);
        
        imageViewDeviceStatus = findViewById(R.id.imageViewDeviceStatus);
        imageViewTempIcon = findViewById(R.id.imageViewTempIcon);
        imageViewFanIcon = findViewById(R.id.imageViewFanIcon);
        imageViewVoltage = findViewById(R.id.imageViewVoltage);
        imageViewCurrent = findViewById(R.id.imageViewCurrent);
        imageViewWatt = findViewById(R.id.imageViewWatt);
        imageViewKwh = findViewById(R.id.imageViewKwh);
        
        switchAutoMode = findViewById(R.id.switchAutoMode);
        sliderFanSpeed = findViewById(R.id.sliderFanSpeed);
        chipTemperatureStatus = findViewById(R.id.chipTemperatureStatus);
        chipPowerStatus = findViewById(R.id.chipPowerStatus);
        
        buttonHistory = findViewById(R.id.buttonHistory);
        buttonSettings = findViewById(R.id.buttonSettings);
    }

    private void setupToolbar() {
        setSupportActionBar(toolbar);
        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayShowTitleEnabled(true);
        }
    }

    private void setupBottomNavigation() {
        bottomNavigation.setSelectedItemId(R.id.nav_dashboard);
        bottomNavigation.setOnItemSelectedListener(item -> {
            int itemId = item.getItemId();
            if (itemId == R.id.nav_dashboard) {
                return true;
            } else if (itemId == R.id.nav_history) {
                startActivity(new Intent(MainActivity.this, HistoryActivity.class));
                return true;
            } else if (itemId == R.id.nav_settings) {
                startActivity(new Intent(MainActivity.this, SettingsActivity.class));
                return true;
            } else if (itemId == R.id.nav_profile) {
                showProfileDialog();
                return true;
            }
            return false;
        });
    }

    private void setupDashboard() {
        animateCardsEntry();
        setupDeviceDataListener();
        setupControls();
        setupQuickActions();
    }

    private void animateCardsEntry() {
        // Animate cards entering with staggered animation
        MaterialCardView[] cards = {cardDeviceStatus, cardTemperature, cardFanControl, cardPowerMonitoring, cardEnergyConsumption, cardQuickActions};
        
        for (int i = 0; i < cards.length; i++) {
            MaterialCardView card = cards[i];
            if (card != null) { // Add null check for safety
                card.setAlpha(0f);
                card.setTranslationY(100f);
                
                card.animate()
                    .alpha(1f)
                    .translationY(0f)
                    .setDuration(500)
                    .setStartDelay(i * 100)
                    .setInterpolator(new AccelerateDecelerateInterpolator())
                    .start();
            }
        }
    }

    private void setupDeviceDataListener() {
        // Set default/demo values since no device is linked
        updateDeviceStatus(false); // Show device as offline
        updateTemperatureDisplay(25.0); // Default temperature
        updateFanDisplay(0); // Fan off
        updateModeDisplay("manual"); // Manual mode
        
        // Default power monitoring values
        updatePowerDisplay(220.0, 0.0); // Voltage with no current
        updateWattDisplay(0.0); // No power consumption
        updateEnergyDisplay(0.0); // No energy consumption
        
        updateLastSeenTime();
    }

    private void updateDeviceStatus(boolean isOnline) {
        if (isOnline) {
            textViewDeviceStatus.setText(getString(R.string.dashboard_device_online));
            textViewDeviceStatus.setTextColor(getColor(R.color.status_online));
            imageViewDeviceStatus.setImageResource(R.drawable.ic_device_online);
        } else {
            textViewDeviceStatus.setText(getString(R.string.dashboard_device_offline));
            textViewDeviceStatus.setTextColor(getColor(R.color.status_offline));
            imageViewDeviceStatus.setImageResource(R.drawable.ic_device_online); // You might want an offline icon
        }
    }

    private void updateTemperatureDisplay(double temperature) {
        // Update the progress bar (0-50°C range)
        int progress = (int) Math.min(Math.max(temperature, 0), 50);
        ObjectAnimator progressAnimator = ObjectAnimator.ofInt(progressBarTemperature, "progress", progress);
        progressAnimator.setDuration(1000);
        progressAnimator.setInterpolator(new AccelerateDecelerateInterpolator());
        progressAnimator.start();
        
        // Update the center temperature display with animation
        String currentTempText = textViewGaugeTemp.getText().toString();
        try {
            int currentTemp = Integer.parseInt(currentTempText);
            ValueAnimator textAnimator = ValueAnimator.ofInt(currentTemp, (int) temperature);
            textAnimator.setDuration(1000);
            textAnimator.addUpdateListener(animation -> {
                int animatedValue = (int) animation.getAnimatedValue();
                textViewGaugeTemp.setText(String.valueOf(animatedValue));
            });
            textAnimator.start();
        } catch (NumberFormatException e) {
            // If parsing fails, just set the value directly
            textViewGaugeTemp.setText(String.valueOf((int) temperature));
        }
        
        // Update temperature text
        textViewCurrentTemp.setText(String.format(Locale.getDefault(), "%.1f°C", temperature));
        
        // Update temperature status chip
        updateTemperatureStatus(temperature);
    }

    private void updateTemperatureStatus(double temperature) {
        String status;
        int backgroundColor;
        int iconResource;
        
        if (temperature < 20) {
            status = "Cold";
            backgroundColor = R.color.temp_cold;
            iconResource = R.drawable.ic_check_circle;
        } else if (temperature < 30) {
            status = "Normal";
            backgroundColor = R.color.accent_green;
            iconResource = R.drawable.ic_check_circle;
        } else if (temperature < 40) {
            status = "Warm";
            backgroundColor = R.color.accent_yellow;
            iconResource = R.drawable.ic_check_circle;
        } else {
            status = "Hot";
            backgroundColor = R.color.accent_red;
            iconResource = R.drawable.ic_check_circle;
        }
        
        chipTemperatureStatus.setText(status);
        chipTemperatureStatus.setChipBackgroundColorResource(backgroundColor);
        chipTemperatureStatus.setChipIcon(getDrawable(iconResource));
    }

    private void updateFanDisplay(int fanSpeed) {
        String fanText = fanSpeed == 0 ? "Off" : "Speed " + fanSpeed;
        textViewFanStatus.setText("Fan: " + fanText);
        sliderFanSpeed.setValue(fanSpeed);
        textViewFanSpeedLabel.setText("Fan Speed: " + fanSpeed);
        
        // Animate fan icon rotation based on speed
        animateFanIcon(fanSpeed);
    }

    private void animateFanIcon(int fanSpeed) {
        if (fanRotationAnimator != null) {
            fanRotationAnimator.cancel();
        }
        
        if (fanSpeed > 0) {
            fanRotationAnimator = ObjectAnimator.ofFloat(imageViewFanIcon, "rotation", 0f, 360f);
            fanRotationAnimator.setDuration(2000 / fanSpeed); // Faster rotation for higher speeds
            fanRotationAnimator.setRepeatCount(ObjectAnimator.INFINITE);
            fanRotationAnimator.setInterpolator(new AccelerateDecelerateInterpolator());
            fanRotationAnimator.start();
        }
    }

    private void updateModeDisplay(String mode) {
        boolean isAutoMode = "auto".equals(mode);
        switchAutoMode.setChecked(isAutoMode);
        sliderFanSpeed.setEnabled(!isAutoMode);
    }

    private void updateLastSeenTime() {
        String timeStr = new SimpleDateFormat("HH:mm", Locale.getDefault()).format(new Date());
        textViewLastUpdated.setText(timeStr);
    }

    private void setupControls() {
        // Auto mode switch
        switchAutoMode.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (isChecked) {
                showAutoModeConfirmDialog();
            } else {
                updateDeviceMode("manual");
                sliderFanSpeed.setEnabled(true);
                showSnackbar("Manual mode enabled", true);
            }
        });

        // Fan speed slider
        sliderFanSpeed.addOnChangeListener((slider, value, fromUser) -> {
            if (fromUser && !switchAutoMode.isChecked()) {
                int fanSpeed = (int) value;
                textViewFanSpeedLabel.setText("Fan Speed: " + fanSpeed);
                updateDeviceFanSpeed(fanSpeed);
            }
        });
    }

    private void showAutoModeConfirmDialog() {
        new AlertDialog.Builder(this)
            .setTitle("Enable Auto Mode")
            .setMessage(getString(R.string.dialog_enable_auto_mode))
            .setPositiveButton(getString(R.string.dialog_yes), (dialog, which) -> {
                updateDeviceMode("auto");
                sliderFanSpeed.setEnabled(false);
                showSnackbar("Auto mode enabled", true);
            })
            .setNegativeButton(getString(R.string.dialog_no), (dialog, which) -> {
                switchAutoMode.setChecked(false);
            })
            .show();
    }

    private void updateDeviceMode(String mode) {
        // Device mode update removed - no device linked
        showSnackbar("No device connected to update mode", false);
    }

    private void updateDeviceFanSpeed(int fanSpeed) {
        // Fan speed update removed - no device linked 
        showSnackbar("No device connected to update fan speed", false);
    }

    private void setupQuickActions() {
        buttonHistory.setOnClickListener(v -> {
            startActivity(new Intent(MainActivity.this, HistoryActivity.class));
        });

        buttonSettings.setOnClickListener(v -> {
            startActivity(new Intent(MainActivity.this, SettingsActivity.class));
        });
    }

    private void showProfileDialog() {
        new AlertDialog.Builder(this)
            .setTitle("Profile")
            .setMessage("User: " + (mAuth.getCurrentUser() != null ? mAuth.getCurrentUser().getEmail() : "Unknown"))
            .setPositiveButton("Logout", (dialog, which) -> showLogoutConfirmDialog())
            .setNegativeButton("Cancel", null)
            .show();
    }

    private void updatePowerDisplay(double voltage, double current) {
        if (textViewVoltage != null) {
            textViewVoltage.setText(String.format(Locale.getDefault(), "%.1fV", voltage));
        }
        if (textViewCurrent != null) {
            textViewCurrent.setText(String.format(Locale.getDefault(), "%.3fA", current));
        }
    }

    private void updateWattDisplay(double watt) {
        if (textViewWatt != null) {
            textViewWatt.setText(String.format(Locale.getDefault(), "%.2fW", watt));
        }
        
        // Update power status chip based on power consumption
        updatePowerStatus(watt);
        
        // Check for high power consumption alerts
        checkPowerConsumptionAlert(watt);
    }

    private void updateEnergyDisplay(double kwh) {
        if (textViewKwh != null) {
            if (kwh < 1.0) {
                // Show in Wh for values less than 1 kWh
                textViewKwh.setText(String.format(Locale.getDefault(), "%.0fWh", kwh * 1000));
            } else {
                textViewKwh.setText(String.format(Locale.getDefault(), "%.3fkWh", kwh));
            }
        }
    }

    private void updatePowerStatus(double watt) {
        if (chipPowerStatus == null) return;
        
        String status;
        int backgroundColor;
        int iconResource;
        
        if (watt < 10) {
            status = "Low";
            backgroundColor = R.color.accent_green;
            iconResource = R.drawable.ic_check_circle;
        } else if (watt < 50) {
            status = "Normal";
            backgroundColor = R.color.accent_blue;
            iconResource = R.drawable.ic_check_circle;
        } else if (watt < 100) {
            status = "High";
            backgroundColor = R.color.accent_yellow;
            iconResource = R.drawable.ic_check_circle;
        } else {
            status = "Very High";
            backgroundColor = R.color.accent_red;
            iconResource = R.drawable.ic_check_circle;
        }
        
        chipPowerStatus.setText(status);
        chipPowerStatus.setChipBackgroundColorResource(backgroundColor);
        chipPowerStatus.setChipIcon(getDrawable(iconResource));
    }

    private void checkPowerConsumptionAlert(double watt) {
        // Define power consumption thresholds
        final double HIGH_POWER_THRESHOLD = 75.0; // Watts
        final double VERY_HIGH_POWER_THRESHOLD = 100.0; // Watts
        
        if (watt >= VERY_HIGH_POWER_THRESHOLD) {
            showSnackbar("⚠️ Very high power consumption: " + String.format(Locale.getDefault(), "%.2fW", watt), false);
        } else if (watt >= HIGH_POWER_THRESHOLD) {
            showSnackbar("⚡ High power consumption: " + String.format(Locale.getDefault(), "%.2fW", watt), false);
        }
    }

    private void showLogoutConfirmDialog() {
        new AlertDialog.Builder(this)
            .setTitle("Logout")
            .setMessage(getString(R.string.dialog_logout))
            .setPositiveButton(getString(R.string.dialog_yes), (dialog, which) -> {
                mAuth.signOut();
                startActivity(new Intent(MainActivity.this, com.qppd.smartfan.auth.LoginActivity.class));
                finish();
            })
            .setNegativeButton(getString(R.string.dialog_no), null)
            .show();
    }

    private void showSnackbar(String message, boolean isSuccess) {
        Snackbar snackbar = Snackbar.make(coordinatorLayout, message, Snackbar.LENGTH_SHORT);
        if (!isSuccess) {
            snackbar.setBackgroundTint(getColor(R.color.accent_red));
        }
        snackbar.show();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, 1, Menu.NONE, "Settings").setIcon(R.drawable.ic_settings);
        menu.add(0, 2, Menu.NONE, "History").setIcon(R.drawable.ic_history);
        menu.add(0, 3, Menu.NONE, "Devices").setIcon(R.drawable.ic_device_online);
        menu.add(0, 4, Menu.NONE, "Logout").setIcon(R.drawable.ic_profile);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int itemId = item.getItemId();
        if (itemId == 1) {
            startActivity(new Intent(MainActivity.this, SettingsActivity.class));
            return true;
        } else if (itemId == 2) {
            startActivity(new Intent(MainActivity.this, HistoryActivity.class));
            return true;
        } else if (itemId == 3) {
            showLogoutConfirmDialog();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (fanRotationAnimator != null) {
            fanRotationAnimator.cancel();
        }
    }
}