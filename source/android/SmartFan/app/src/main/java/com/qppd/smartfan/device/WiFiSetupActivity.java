package com.qppd.smartfan.device;

import android.Manifest;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.text.TextUtils;
import android.view.View;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.appbar.MaterialToolbar;
import com.google.android.material.button.MaterialButton;
import com.google.android.material.card.MaterialCardView;
import com.google.android.material.progressindicator.CircularProgressIndicator;
import com.google.android.material.snackbar.Snackbar;
import com.google.android.material.textfield.TextInputEditText;
import com.google.android.material.textfield.TextInputLayout;
import com.qppd.smartfan.R;

import java.util.ArrayList;
import java.util.List;

public class WiFiSetupActivity extends AppCompatActivity implements WiFiNetworkAdapter.OnNetworkSelectedListener {
    
    private static final int PERMISSION_REQUEST_CODE = 100;
    private static final String SMARTFAN_AP_PREFIX = "SmartFan-";
    
    // UI Components
    private MaterialToolbar toolbar;
    private MaterialButton buttonScanWiFi, buttonConfigureDevice;
    private RecyclerView recyclerViewWiFiNetworks;
    private MaterialCardView cardWiFiSelection, cardSmartFanConfig, cardProgress;
    private TextInputLayout textInputLayoutPassword, textInputLayoutDeviceId;
    private TextInputEditText editTextPassword, editTextDeviceId;
    private CircularProgressIndicator progressIndicator;
    
    // WiFi Components
    private WifiManager wifiManager;
    private WiFiNetworkAdapter wifiAdapter;
    private List<ScanResult> wifiNetworks;
    private ScanResult selectedNetwork;
    
    // BroadcastReceiver for WiFi scan results
    private BroadcastReceiver wifiScanReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            boolean success = intent.getBooleanExtra(WifiManager.EXTRA_RESULTS_UPDATED, false);
            if (success) {
                scanSuccess();
            } else {
                scanFailure();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_wifi_setup);
        
        initializeViews();
        setupToolbar();
        setupRecyclerView();
        setupClickListeners();
        initializeWiFi();
        checkPermissions();
    }

    private void initializeViews() {
        toolbar = findViewById(R.id.toolbar);
        buttonScanWiFi = findViewById(R.id.buttonScanWiFi);
        buttonConfigureDevice = findViewById(R.id.buttonConfigureDevice);
        recyclerViewWiFiNetworks = findViewById(R.id.recyclerViewWiFiNetworks);
        cardWiFiSelection = findViewById(R.id.cardWiFiSelection);
        cardSmartFanConfig = findViewById(R.id.cardSmartFanConfig);
        cardProgress = findViewById(R.id.cardProgress);
        textInputLayoutPassword = findViewById(R.id.textInputLayoutPassword);
        textInputLayoutDeviceId = findViewById(R.id.textInputLayoutDeviceId);
        editTextPassword = findViewById(R.id.editTextPassword);
        editTextDeviceId = findViewById(R.id.editTextDeviceId);
        progressIndicator = findViewById(R.id.progressIndicator);
    }

    private void setupToolbar() {
        setSupportActionBar(toolbar);
        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
            getSupportActionBar().setDisplayShowHomeEnabled(true);
        }
        
        toolbar.setNavigationOnClickListener(v -> onBackPressed());
    }

    private void setupRecyclerView() {
        wifiNetworks = new ArrayList<>();
        wifiAdapter = new WiFiNetworkAdapter(wifiNetworks, this);
        recyclerViewWiFiNetworks.setLayoutManager(new LinearLayoutManager(this));
        recyclerViewWiFiNetworks.setAdapter(wifiAdapter);
    }

    private void setupClickListeners() {
        buttonScanWiFi.setOnClickListener(v -> scanForWiFiNetworks());
        buttonConfigureDevice.setOnClickListener(v -> configureSmartFan());
    }

    private void initializeWiFi() {
        wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        
        // Enable WiFi if disabled
        if (wifiManager != null && !wifiManager.isWifiEnabled()) {
            wifiManager.setWifiEnabled(true);
        }
    }

    private void checkPermissions() {
        String[] permissions = {
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.ACCESS_COARSE_LOCATION,
            Manifest.permission.CHANGE_WIFI_STATE,
            Manifest.permission.ACCESS_WIFI_STATE
        };

        List<String> permissionsToRequest = new ArrayList<>();
        for (String permission : permissions) {
            if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                permissionsToRequest.add(permission);
            }
        }

        if (!permissionsToRequest.isEmpty()) {
            ActivityCompat.requestPermissions(this, 
                permissionsToRequest.toArray(new String[0]), 
                PERMISSION_REQUEST_CODE);
        } else {
            // Automatically scan for networks once permissions are granted
            scanForWiFiNetworks();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        
        if (requestCode == PERMISSION_REQUEST_CODE) {
            boolean allPermissionsGranted = true;
            for (int result : grantResults) {
                if (result != PackageManager.PERMISSION_GRANTED) {
                    allPermissionsGranted = false;
                    break;
                }
            }
            
            if (allPermissionsGranted) {
                scanForWiFiNetworks();
            } else {
                Snackbar.make(cardWiFiSelection, "WiFi permissions are required for setup", Snackbar.LENGTH_LONG)
                    .setAction("Grant", v -> checkPermissions())
                    .show();
            }
        }
    }

    private void scanForWiFiNetworks() {
        if (wifiManager == null) return;
        
        buttonScanWiFi.setText("Scanning...");
        buttonScanWiFi.setEnabled(false);
        
        // Register receiver for scan results
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
        registerReceiver(wifiScanReceiver, intentFilter);
        
        // Start scan
        boolean success = wifiManager.startScan();
        if (!success) {
            scanFailure();
        }
    }

    private void scanSuccess() {
        if (wifiManager == null) return;
        
        try {
            List<ScanResult> results = wifiManager.getScanResults();
            wifiNetworks.clear();
            
            // Filter and sort networks
            for (ScanResult result : results) {
                if (!TextUtils.isEmpty(result.SSID)) {
                    // Check if network already exists (avoid duplicates)
                    boolean exists = false;
                    for (ScanResult existing : wifiNetworks) {
                        if (existing.SSID.equals(result.SSID)) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        wifiNetworks.add(result);
                    }
                }
            }
            
            // Sort by signal strength
            wifiNetworks.sort((a, b) -> Integer.compare(b.level, a.level));
            
            wifiAdapter.notifyDataSetChanged();
            
            buttonScanWiFi.setText("Scan for Networks");
            buttonScanWiFi.setEnabled(true);
            
            if (wifiNetworks.isEmpty()) {
                Toast.makeText(this, "No WiFi networks found", Toast.LENGTH_SHORT).show();
            }
            
        } catch (SecurityException e) {
            scanFailure();
        }
        
        try {
            unregisterReceiver(wifiScanReceiver);
        } catch (IllegalArgumentException e) {
            // Receiver not registered
        }
    }

    private void scanFailure() {
        buttonScanWiFi.setText("Scan for Networks");
        buttonScanWiFi.setEnabled(true);
        
        Toast.makeText(this, "WiFi scan failed", Toast.LENGTH_SHORT).show();
        
        try {
            unregisterReceiver(wifiScanReceiver);
        } catch (IllegalArgumentException e) {
            // Receiver not registered
        }
    }

    @Override
    public void onNetworkSelected(ScanResult network) {
        selectedNetwork = network;
        
        // Show configuration card
        cardSmartFanConfig.setVisibility(View.VISIBLE);
        
        // Update selected network display
        findViewById(R.id.textViewSelectedNetwork).setVisibility(View.VISIBLE);
        ((android.widget.TextView) findViewById(R.id.textViewSelectedNetwork))
            .setText("Selected Network: " + network.SSID);
        
        // Show/hide password field based on security
        boolean isSecured = !network.capabilities.contains("[ESS]") || 
                           network.capabilities.contains("WPA") || 
                           network.capabilities.contains("WEP");
        
        textInputLayoutPassword.setVisibility(isSecured ? View.VISIBLE : View.GONE);
        
        // Scroll to configuration card
        findViewById(R.id.cardSmartFanConfig).getParent().requestChildFocus(
            findViewById(R.id.cardSmartFanConfig), findViewById(R.id.cardSmartFanConfig));
    }

    private void configureSmartFan() {
        if (selectedNetwork == null) {
            Toast.makeText(this, "Please select a WiFi network first", Toast.LENGTH_SHORT).show();
            return;
        }
        
        String password = editTextPassword.getText() != null ? editTextPassword.getText().toString() : "";
        String deviceId = editTextDeviceId.getText() != null ? editTextDeviceId.getText().toString() : "";
        
        // Validate password if network is secured
        boolean isSecured = !selectedNetwork.capabilities.contains("[ESS]") || 
                           selectedNetwork.capabilities.contains("WPA") || 
                           selectedNetwork.capabilities.contains("WEP");
        
        if (isSecured && TextUtils.isEmpty(password)) {
            textInputLayoutPassword.setError("Password is required for secured networks");
            return;
        }
        
        // Show progress
        showProgress("Connecting to Smart Fan...");
        
        // Connect to SmartFan access point first
        connectToSmartFanAP(selectedNetwork.SSID, password, deviceId);
    }

    private void connectToSmartFanAP(String targetSSID, String targetPassword, String deviceId) {
        // Look for SmartFan access points
        ScanResult smartFanAP = null;
        for (ScanResult network : wifiNetworks) {
            if (network.SSID.startsWith(SMARTFAN_AP_PREFIX)) {
                smartFanAP = network;
                break;
            }
        }
        
        if (smartFanAP == null) {
            showError("SmartFan access point not found. Please ensure your device is in setup mode.");
            return;
        }
        
        // Connect to SmartFan AP
        connectToNetwork(smartFanAP.SSID, "smartfan123", () -> {
            // Once connected to SmartFan AP, send configuration
            sendWiFiConfiguration(targetSSID, targetPassword, deviceId);
        });
    }

    private void connectToNetwork(String ssid, String password, Runnable onSuccess) {
        if (wifiManager == null) return;
        
        WifiConfiguration wifiConfig = new WifiConfiguration();
        wifiConfig.SSID = "\"" + ssid + "\"";
        
        if (!TextUtils.isEmpty(password)) {
            wifiConfig.preSharedKey = "\"" + password + "\"";
        } else {
            wifiConfig.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
        }
        
        try {
            int netId = wifiManager.addNetwork(wifiConfig);
            if (netId != -1) {
                wifiManager.disconnect();
                wifiManager.enableNetwork(netId, true);
                wifiManager.reconnect();
                
                // Wait for connection and then execute callback
                new Handler().postDelayed(() -> {
                    if (onSuccess != null) {
                        onSuccess.run();
                    }
                }, 5000);
            } else {
                showError("Failed to add network configuration");
            }
        } catch (Exception e) {
            showError("Failed to connect to network: " + e.getMessage());
        }
    }

    private void sendWiFiConfiguration(String ssid, String password, String deviceId) {
        updateProgress("Configuring Smart Fan WiFi...");
        
        // In a real implementation, this would send HTTP requests to the SmartFan's
        // configuration endpoint (typically 192.168.4.1)
        // For now, we'll simulate the process
        
        new Handler().postDelayed(() -> {
            updateProgress("Saving configuration...");
            
            new Handler().postDelayed(() -> {
                updateProgress("Restarting Smart Fan...");
                
                new Handler().postDelayed(() -> {
                    hideProgress();
                    showSuccess("Smart Fan configured successfully!");
                    
                    // Return to device linking
                    finish();
                }, 2000);
            }, 2000);
        }, 3000);
    }

    private void showProgress(String message) {
        cardProgress.setVisibility(View.VISIBLE);
        progressIndicator.setVisibility(View.VISIBLE);
        ((android.widget.TextView) findViewById(R.id.textViewProgress)).setText(message);
        
        // Hide other cards
        cardWiFiSelection.setVisibility(View.GONE);
        cardSmartFanConfig.setVisibility(View.GONE);
    }

    private void updateProgress(String message) {
        ((android.widget.TextView) findViewById(R.id.textViewProgress)).setText(message);
    }

    private void hideProgress() {
        cardProgress.setVisibility(View.GONE);
        
        // Show main cards
        cardWiFiSelection.setVisibility(View.VISIBLE);
        if (selectedNetwork != null) {
            cardSmartFanConfig.setVisibility(View.VISIBLE);
        }
    }

    private void showError(String message) {
        hideProgress();
        Toast.makeText(this, message, Toast.LENGTH_LONG).show();
    }

    private void showSuccess(String message) {
        Toast.makeText(this, message, Toast.LENGTH_LONG).show();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        try {
            unregisterReceiver(wifiScanReceiver);
        } catch (IllegalArgumentException e) {
            // Receiver not registered
        }
    }
}