package com.qppd.smartfan;

import android.os.Build;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import com.google.android.material.appbar.MaterialToolbar;
import com.google.android.material.chip.ChipGroup;
import com.google.android.material.button.MaterialButton;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.database.*;
import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.charts.BarChart;
import com.github.mikephil.charting.components.Description;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.*;
import com.github.mikephil.charting.formatter.IndexAxisValueFormatter;
import com.github.mikephil.charting.formatter.ValueFormatter;
import com.github.mikephil.charting.interfaces.datasets.ILineDataSet;
import com.github.mikephil.charting.interfaces.datasets.IBarDataSet;
import android.graphics.Color;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;
import java.util.List;

public class HistoryActivity extends AppCompatActivity {
    private RecyclerView recyclerViewLogs;
    private LogAdapter adapter;
    private ArrayList<LogEntry> logsList = new ArrayList<>();
    private DatabaseReference dbRef;
    private String uid;
    private MaterialToolbar toolbar;
    
    // Chart components
    private LineChart chartTemperature;
    private BarChart chartFanSpeed;
    private ChipGroup chipGroupFilter;
    private MaterialButton buttonExport;
    
    // Time filter options (in milliseconds)
    private static final long FILTER_24H = 24 * 60 * 60 * 1000L;
    private static final long FILTER_7DAYS = 7 * 24 * 60 * 60 * 1000L;
    private static final long FILTER_30DAYS = 30 * 24 * 60 * 60 * 1000L;
    
    private long currentTimeFilter = FILTER_24H; // Default to 24 hours

    // Data class to hold log entry information - Updated to match ESP8266 Firebase structure
    public static class LogEntry {
        public Long timestamp;        // Unix timestamp (int64 from ESP8266)
        public String datetime;       // Human-readable datetime string from ESP8266 (YYYY-MM-DD HH:MM:SS)
        public Double temperature;    // Temperature reading
        public Long fanSpeed;         // Fan speed setting
        public Double voltage;        // Voltage measurement
        public Double current;        // Current measurement  
        public Double watt;           // Power consumption in watts
        public Double kwh;            // Energy consumption in kWh
        
        public LogEntry() {}
        
        public LogEntry(Long timestamp, String datetime, Double temperature, Long fanSpeed, 
                       Double voltage, Double current, Double watt, Double kwh) {
            this.timestamp = timestamp;
            this.datetime = datetime;
            this.temperature = temperature;
            this.fanSpeed = fanSpeed;
            this.voltage = voltage;
            this.current = current;
            this.watt = watt;
            this.kwh = kwh;
        }
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_history);

        // Setup toolbar
        toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
            getSupportActionBar().setDisplayShowHomeEnabled(true);
            getSupportActionBar().setTitle("Device History");
        }

        // Initialize components
        recyclerViewLogs = findViewById(R.id.recyclerViewLogs);
        recyclerViewLogs.setLayoutManager(new LinearLayoutManager(this));
        adapter = new LogAdapter();
        recyclerViewLogs.setAdapter(adapter);

        // Initialize charts
        chartTemperature = findViewById(R.id.chartTemperature);
        chartFanSpeed = findViewById(R.id.chartFanSpeed);
        
        // Initialize filter components
        chipGroupFilter = findViewById(R.id.chipGroupFilter);
        buttonExport = findViewById(R.id.buttonExport);

        dbRef = FirebaseDatabase.getInstance().getReference();
        uid = FirebaseAuth.getInstance().getCurrentUser().getUid();

        // Setup charts
        setupTemperatureChart();
        setupFanSpeedChart();
        
        // Setup filter listeners
        setupFilterListeners();

        // Load data from the known device
        loadDeviceLogs();
        
        // Also try to load from general data path as fallback
        loadGeneralData();
    }
    
    private void setupTemperatureChart() {
        // Configure temperature line chart
        chartTemperature.setDragEnabled(true);
        chartTemperature.setScaleEnabled(true);
        chartTemperature.setPinchZoom(true);
        chartTemperature.setDrawGridBackground(false);
        
        // Description
        Description desc = new Description();
        desc.setText("Temperature over time (°C)");
        desc.setTextSize(12f);
        chartTemperature.setDescription(desc);
        
        // X-axis
        XAxis xAxis = chartTemperature.getXAxis();
        xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
        xAxis.setDrawGridLines(true);
        xAxis.setGranularity(1f);
        xAxis.setLabelCount(5);
        xAxis.setValueFormatter(new ValueFormatter() {
            @Override
            public String getFormattedValue(float value) {
                long timestamp = (long) value;
                return new SimpleDateFormat("HH:mm", Locale.getDefault()).format(new Date(timestamp));
            }
        });
        
        // Y-axis
        YAxis leftAxis = chartTemperature.getAxisLeft();
        leftAxis.setDrawGridLines(true);
        leftAxis.setAxisMinimum(0f);
        
        YAxis rightAxis = chartTemperature.getAxisRight();
        rightAxis.setEnabled(false);
        
        // Legend
        chartTemperature.getLegend().setEnabled(true);
    }
    
    private void setupFanSpeedChart() {
        // Configure fan speed bar chart
        chartFanSpeed.setDragEnabled(true);
        chartFanSpeed.setScaleEnabled(true);
        chartFanSpeed.setPinchZoom(true);
        chartFanSpeed.setDrawGridBackground(false);
        
        // Description
        Description desc = new Description();
        desc.setText("Fan speed over time (%)");
        desc.setTextSize(12f);
        chartFanSpeed.setDescription(desc);
        
        // X-axis
        XAxis xAxis = chartFanSpeed.getXAxis();
        xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
        xAxis.setDrawGridLines(true);
        xAxis.setGranularity(1f);
        xAxis.setLabelCount(5);
        xAxis.setValueFormatter(new ValueFormatter() {
            @Override
            public String getFormattedValue(float value) {
                long timestamp = (long) value;
                return new SimpleDateFormat("HH:mm", Locale.getDefault()).format(new Date(timestamp));
            }
        });
        
        // Y-axis
        YAxis leftAxis = chartFanSpeed.getAxisLeft();
        leftAxis.setDrawGridLines(true);
        leftAxis.setAxisMinimum(0f);
        leftAxis.setAxisMaximum(100f);
        
        YAxis rightAxis = chartFanSpeed.getAxisRight();
        rightAxis.setEnabled(false);
        
        // Legend
        chartFanSpeed.getLegend().setEnabled(true);
    }
    
    private void setupFilterListeners() {
        chipGroupFilter.setOnCheckedStateChangeListener((group, checkedIds) -> {
            if (!checkedIds.isEmpty()) {
                int checkedId = checkedIds.get(0);
                if (checkedId == R.id.chip24h) {
                    currentTimeFilter = FILTER_24H;
                } else if (checkedId == R.id.chip7days) {
                    currentTimeFilter = FILTER_7DAYS;
                } else if (checkedId == R.id.chip30days) {
                    currentTimeFilter = FILTER_30DAYS;
                }
                
                // Update charts with new filter
                updateCharts();
            }
        });
        
        buttonExport.setOnClickListener(v -> exportData());
    }
    
    private List<LogEntry> getFilteredData() {
        List<LogEntry> filteredList = new ArrayList<>();
        long currentTime = System.currentTimeMillis();
        long cutoffTime = currentTime - currentTimeFilter;
        
        for (LogEntry entry : logsList) {
            if (entry.timestamp != null) {
                long entryTime = entry.timestamp * 1000; // Convert to milliseconds
                if (entryTime >= cutoffTime) {
                    filteredList.add(entry);
                }
            }
        }
        
        return filteredList;
    }
    
    private void updateCharts() {
        List<LogEntry> filteredData = getFilteredData();
        
        if (filteredData.isEmpty()) {
            // Clear charts if no data
            chartTemperature.clear();
            chartFanSpeed.clear();
            chartTemperature.invalidate();
            chartFanSpeed.invalidate();
            return;
        }
        
        updateTemperatureChart(filteredData);
        updateFanSpeedChart(filteredData);
        updatePowerChart(filteredData);
    }
    
    private void updatePowerChart(List<LogEntry> data) {
        // We'll use the fan speed chart to also show power data by creating a combined view
        // or we can add power data as a secondary dataset to the fan speed chart
        
        ArrayList<Entry> powerEntries = new ArrayList<>();
        
        for (LogEntry entry : data) {
            if (entry.timestamp != null && entry.watt != null && entry.watt > 0) {
                float timestamp = entry.timestamp * 1000f; // Convert to milliseconds for X-axis
                powerEntries.add(new Entry(timestamp, entry.watt.floatValue()));
            }
        }
        
        if (powerEntries.isEmpty()) {
            return; // No power data to add
        }
        
        // Add power data as a secondary line to the temperature chart
        LineDataSet powerDataSet = new LineDataSet(powerEntries, "Power (W)");
        powerDataSet.setColor(Color.parseColor("#4CAF50")); // Green color
        powerDataSet.setCircleColor(Color.parseColor("#4CAF50"));
        powerDataSet.setLineWidth(2f);
        powerDataSet.setCircleRadius(3f);
        powerDataSet.setDrawCircleHole(false);
        powerDataSet.setValueTextSize(9f);
        powerDataSet.setAxisDependency(YAxis.AxisDependency.RIGHT);
        
        // Enable right Y-axis for power data
        YAxis rightAxis = chartTemperature.getAxisRight();
        rightAxis.setEnabled(true);
        rightAxis.setAxisMinimum(0f);
        rightAxis.setDrawGridLines(false);
        
        // Get existing data and add power data
        LineData existingData = chartTemperature.getLineData();
        if (existingData != null) {
            existingData.addDataSet(powerDataSet);
            chartTemperature.setData(existingData);
        }
    }
    
    private void updateTemperatureChart(List<LogEntry> data) {
        ArrayList<Entry> temperatureEntries = new ArrayList<>();
        
        for (LogEntry entry : data) {
            if (entry.timestamp != null && entry.temperature != null) {
                float timestamp = entry.timestamp * 1000f; // Convert to milliseconds for X-axis
                temperatureEntries.add(new Entry(timestamp, entry.temperature.floatValue()));
            }
        }
        
        if (temperatureEntries.isEmpty()) {
            chartTemperature.clear();
            chartTemperature.invalidate();
            return;
        }
        
        LineDataSet dataSet = new LineDataSet(temperatureEntries, "Temperature (°C)");
        dataSet.setColor(Color.parseColor("#FF5722")); // Orange color
        dataSet.setCircleColor(Color.parseColor("#FF5722"));
        dataSet.setLineWidth(2f);
        dataSet.setCircleRadius(3f);
        dataSet.setDrawCircleHole(false);
        dataSet.setValueTextSize(9f);
        dataSet.setDrawFilled(true);
        dataSet.setFillColor(Color.parseColor("#FFCCBC")); // Light orange fill
        
        ArrayList<ILineDataSet> dataSets = new ArrayList<>();
        dataSets.add(dataSet);
        
        LineData lineData = new LineData(dataSets);
        chartTemperature.setData(lineData);
        chartTemperature.invalidate(); // Refresh chart
    }
    
    private void updateFanSpeedChart(List<LogEntry> data) {
        ArrayList<BarEntry> fanSpeedEntries = new ArrayList<>();
        ArrayList<String> labels = new ArrayList<>();
        
        int index = 0;
        for (LogEntry entry : data) {
            if (entry.timestamp != null && entry.fanSpeed != null) {
                fanSpeedEntries.add(new BarEntry(index, entry.fanSpeed.floatValue()));
                
                // Create time label
                String timeLabel = new SimpleDateFormat("HH:mm", Locale.getDefault())
                    .format(new Date(entry.timestamp * 1000));
                labels.add(timeLabel);
                index++;
            }
        }
        
        if (fanSpeedEntries.isEmpty()) {
            chartFanSpeed.clear();
            chartFanSpeed.invalidate();
            return;
        }
        
        BarDataSet dataSet = new BarDataSet(fanSpeedEntries, "Fan Speed (%)");
        dataSet.setColor(Color.parseColor("#2196F3")); // Blue color
        dataSet.setValueTextSize(9f);
        
        ArrayList<IBarDataSet> dataSets = new ArrayList<>();
        dataSets.add(dataSet);
        
        BarData barData = new BarData(dataSets);
        barData.setBarWidth(0.9f);
        
        chartFanSpeed.setData(barData);
        
        // Update X-axis labels
        XAxis xAxis = chartFanSpeed.getXAxis();
        xAxis.setValueFormatter(new IndexAxisValueFormatter(labels));
        xAxis.setLabelCount(Math.min(labels.size(), 5));
        
        chartFanSpeed.invalidate(); // Refresh chart
    }
    
    private void exportData() {
        List<LogEntry> filteredData = getFilteredData();
        
        if (filteredData.isEmpty()) {
            Toast.makeText(this, "No data to export", Toast.LENGTH_SHORT).show();
            return;
        }
        
        StringBuilder csvData = new StringBuilder();
        csvData.append("Timestamp,DateTime,Temperature(°C),Fan Speed(%),Voltage(V),Current(A),Power(W),Energy(kWh)\n");
        
        for (LogEntry entry : filteredData) {
            csvData.append(entry.timestamp != null ? entry.timestamp : "")
                   .append(",")
                   .append(entry.datetime != null ? entry.datetime : "")
                   .append(",")
                   .append(entry.temperature != null ? entry.temperature : "")
                   .append(",")
                   .append(entry.fanSpeed != null ? entry.fanSpeed : "")
                   .append(",")
                   .append(entry.voltage != null ? entry.voltage : "")
                   .append(",")
                   .append(entry.current != null ? entry.current : "")
                   .append(",")
                   .append(entry.watt != null ? entry.watt : "")
                   .append(",")
                   .append(entry.kwh != null ? entry.kwh : "")
                   .append("\n");
        }
        
        // For now, just show the CSV data in a toast (in a real app, save to file)
        Toast.makeText(this, "CSV data generated (" + filteredData.size() + " entries)", Toast.LENGTH_LONG).show();
        
        // TODO: Implement actual file saving using Storage Access Framework
        System.out.println("CSV Export:\n" + csvData.toString());
    }
    
    private void loadGeneralData() {
        // ESP8266 also writes to /smartfan/data path
        DatabaseReference generalDataRef = dbRef.child("smartfan").child("data");
        
        generalDataRef.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                if (snapshot.exists()) {
                    try {
                        // Parse the general data entry
                        LogEntry entry = snapshot.getValue(LogEntry.class);
                        if (entry != null && (entry.timestamp != null || (entry.datetime != null && !entry.datetime.trim().isEmpty()))) {
                            // Only add if not already in logs (check for duplicate timestamps)
                            boolean isDuplicate = false;
                            if (entry.timestamp != null) {
                                for (LogEntry existingEntry : logsList) {
                                    if (existingEntry.timestamp != null && existingEntry.timestamp.equals(entry.timestamp)) {
                                        isDuplicate = true;
                                        break;
                                    }
                                }
                            }
                            
                            if (!isDuplicate) {
                                logsList.add(0, entry); // Add to beginning as it's likely the most recent
                                adapter.notifyDataSetChanged();
                                updateCharts(); // Update charts when new data is added
                            }
                        }
                    } catch (Exception e) {
                        System.err.println("Error parsing general data entry: " + e.getMessage());
                    }
                }
            }
            
            @Override
            public void onCancelled(@NonNull DatabaseError error) {
                System.err.println("Failed to load general data: " + error.getMessage());
            }
        });
    }
    
    private void loadDeviceLogs() {
        String deviceId = "SmartFan_ESP8266_001";
        DatabaseReference logsRef = dbRef.child("smartfan").child("devices").child(deviceId).child("logs");
        
        logsRef.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                logsList.clear();
                
                if (snapshot.exists()) {
                    for (DataSnapshot logSnapshot : snapshot.getChildren()) {
                        try {
                            LogEntry entry = logSnapshot.getValue(LogEntry.class);
                            if (entry != null) {
                                // Validate that we have at least timestamp or datetime
                                if (entry.timestamp != null || (entry.datetime != null && !entry.datetime.trim().isEmpty())) {
                                    logsList.add(entry);
                                }
                            }
                        } catch (Exception e) {
                            // Log parsing error, skip this entry
                            System.err.println("Error parsing log entry: " + e.getMessage());
                        }
                    }
                    
                    // Sort by timestamp (newest first) - Handle both timestamp and datetime fields
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                        logsList.sort((a, b) -> {
                            // Primary sort by timestamp if both have it
                            if (a.timestamp != null && b.timestamp != null) {
                                return Long.compare(b.timestamp, a.timestamp);
                            }
                            // If one lacks timestamp, sort by datetime string comparison
                            if (a.datetime != null && b.datetime != null) {
                                return b.datetime.compareTo(a.datetime);
                            }
                            // Handle null cases
                            if (a.timestamp == null && b.timestamp == null && a.datetime == null && b.datetime == null) return 0;
                            if ((a.timestamp == null && a.datetime == null)) return 1;
                            if ((b.timestamp == null && b.datetime == null)) return -1;
                            return 0;
                        });
                    }

                    adapter.notifyDataSetChanged();
                    recyclerViewLogs.setVisibility(View.VISIBLE);
                    
                    // Update charts with loaded data
                    updateCharts();
                    
                    if (logsList.isEmpty()) {
                        Toast.makeText(HistoryActivity.this, "No valid history data available yet.", Toast.LENGTH_LONG).show();
                    } else {
                        Toast.makeText(HistoryActivity.this, 
                            "Loaded " + logsList.size() + " history entries", Toast.LENGTH_SHORT).show();
                    }
                } else {
                    // No logs found
                    recyclerViewLogs.setVisibility(View.GONE);
                    Toast.makeText(HistoryActivity.this, "No history data available yet.", Toast.LENGTH_LONG).show();
                }
            }
            
            @Override
            public void onCancelled(@NonNull DatabaseError error) {
                Toast.makeText(HistoryActivity.this, "Failed to load history: " + error.getMessage(), Toast.LENGTH_LONG).show();
                recyclerViewLogs.setVisibility(View.GONE);
            }
        });
    }
    
    @Override
    public boolean onSupportNavigateUp() {
        onBackPressed();
        return true;
    }

    // RecyclerView Adapter for logs
    private class LogAdapter extends RecyclerView.Adapter<LogAdapter.LogViewHolder> {

        @NonNull
        @Override
        public LogViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_history_log, parent, false);
            return new LogViewHolder(view);
        }

        @Override
        public void onBindViewHolder(@NonNull LogViewHolder holder, int position) {
            LogEntry entry = logsList.get(position);
            
            // Set timestamp - Use datetime string from ESP8266 if available, otherwise format timestamp
            if (entry.datetime != null && !entry.datetime.trim().isEmpty()) {
                // Use the formatted datetime string directly from ESP8266 (already in GMT+8)
                String displayTime = entry.datetime;
                // Remove "(EST)" suffix if present for cleaner display
                if (displayTime.contains(" (EST)")) {
                    displayTime = displayTime.replace(" (EST)", " (Est)");
                }
                holder.textViewTimestamp.setText(displayTime);
            } else if (entry.timestamp != null) {
                // Fallback: Convert Unix timestamp to readable format
                // ESP8266 timestamps are in GMT+8, so we display them as-is
                String timeStr = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault())
                    .format(new Date(entry.timestamp * 1000));
                holder.textViewTimestamp.setText(timeStr);
            } else {
                holder.textViewTimestamp.setText("-");
            }
            
            // Set temperature
            if (entry.temperature != null) {
                holder.textViewLogTemp.setText(String.format(Locale.getDefault(), "%.1f°C", entry.temperature));
            } else {
                holder.textViewLogTemp.setText("-");
            }
            
            // Set fan speed
            if (entry.fanSpeed != null) {
                holder.textViewLogFan.setText(String.format(Locale.getDefault(), "Speed: %d", entry.fanSpeed));
            } else {
                holder.textViewLogFan.setText("Speed: -");
            }
            
            // Handle power monitoring data
            boolean hasPowerData = entry.voltage != null || entry.current != null || entry.watt != null || entry.kwh != null;
            
            if (hasPowerData) {
                holder.layoutPowerData.setVisibility(View.VISIBLE);
                holder.layoutEnergyData.setVisibility(View.VISIBLE);
                
                // Voltage
                if (entry.voltage != null) {
                    holder.textViewLogVoltage.setText(String.format(Locale.getDefault(), "%.1fV", entry.voltage));
                } else {
                    holder.textViewLogVoltage.setText("-");
                }
                
                // Current
                if (entry.current != null) {
                    holder.textViewLogCurrent.setText(String.format(Locale.getDefault(), "%.3fA", entry.current));
                } else {
                    holder.textViewLogCurrent.setText("-");
                }
                
                // Watt
                if (entry.watt != null) {
                    holder.textViewLogWatt.setText(String.format(Locale.getDefault(), "%.2fW", entry.watt));
                } else {
                    holder.textViewLogWatt.setText("-");
                }
                
                // kWh
                if (entry.kwh != null && entry.kwh > 0) {
                    if (entry.kwh < 1.0) {
                        holder.textViewLogKwh.setText(String.format(Locale.getDefault(), "%.0fWh", entry.kwh * 1000));
                    } else {
                        holder.textViewLogKwh.setText(String.format(Locale.getDefault(), "%.3fkWh", entry.kwh));
                    }
                } else {
                    holder.textViewLogKwh.setText("-");
                }
            } else {
                holder.layoutPowerData.setVisibility(View.GONE);
                holder.layoutEnergyData.setVisibility(View.GONE);
            }
        }

        @Override
        public int getItemCount() {
            return logsList.size();
        }

        class LogViewHolder extends RecyclerView.ViewHolder {
            TextView textViewTimestamp, textViewLogTemp, textViewLogFan;
            TextView textViewLogVoltage, textViewLogCurrent, textViewLogWatt, textViewLogKwh;
            LinearLayout layoutPowerData, layoutEnergyData;

            LogViewHolder(@NonNull View itemView) {
                super(itemView);
                textViewTimestamp = itemView.findViewById(R.id.textViewTimestamp);
                textViewLogTemp = itemView.findViewById(R.id.textViewLogTemp);
                textViewLogFan = itemView.findViewById(R.id.textViewLogFan);
                textViewLogVoltage = itemView.findViewById(R.id.textViewLogVoltage);
                textViewLogCurrent = itemView.findViewById(R.id.textViewLogCurrent);
                textViewLogWatt = itemView.findViewById(R.id.textViewLogWatt);
                textViewLogKwh = itemView.findViewById(R.id.textViewLogKwh);
                layoutPowerData = itemView.findViewById(R.id.layoutPowerData);
                layoutEnergyData = itemView.findViewById(R.id.layoutEnergyData);
            }
        }
    }
}
