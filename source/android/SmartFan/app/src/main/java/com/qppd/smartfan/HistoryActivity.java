package com.qppd.smartfan;

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
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.database.*;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;

public class HistoryActivity extends AppCompatActivity {
    private RecyclerView recyclerViewLogs;
    private LogAdapter adapter;
    private ArrayList<LogEntry> logsList = new ArrayList<>();
    private DatabaseReference dbRef;
    private String uid, linkedDeviceId;

    // Data class to hold log entry information
    public static class LogEntry {
        public Long timestamp;
        public Double temperature;
        public Long fanSpeed;
        public Double voltage;
        public Double current;
        public Double watt;
        public Double kwh;
        
        public LogEntry() {}
        
        public LogEntry(Long timestamp, Double temperature, Long fanSpeed, 
                       Double voltage, Double current, Double watt, Double kwh) {
            this.timestamp = timestamp;
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

        recyclerViewLogs = findViewById(R.id.recyclerViewLogs);
        recyclerViewLogs.setLayoutManager(new LinearLayoutManager(this));
        adapter = new LogAdapter();
        recyclerViewLogs.setAdapter(adapter);

        dbRef = FirebaseDatabase.getInstance().getReference();
        uid = FirebaseAuth.getInstance().getCurrentUser().getUid();

        // Get linked device
        dbRef.child("users").child(uid).child("devices").addListenerForSingleValueEvent(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot snapshot) {
                if (snapshot.exists()) {
                    for (DataSnapshot deviceSnap : snapshot.getChildren()) {
                        linkedDeviceId = deviceSnap.getKey();
                        break;
                    }
                    loadLogs();
                } else {
                    Toast.makeText(HistoryActivity.this, "No device linked.", Toast.LENGTH_SHORT).show();
                    finish();
                }
            }
            @Override
            public void onCancelled(DatabaseError error) {}
        });
    }

    private void loadLogs() {
        dbRef.child("devices").child(linkedDeviceId).child("logs").limitToLast(50).addListenerForSingleValueEvent(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot snapshot) {
                logsList.clear();
                for (DataSnapshot logSnap : snapshot.getChildren()) {
                    Long timestamp = logSnap.child("timestamp").getValue(Long.class);
                    Double temp = logSnap.child("temperature").getValue(Double.class);
                    Long fanSpeed = logSnap.child("fanSpeed").getValue(Long.class);
                    Double voltage = logSnap.child("voltage").getValue(Double.class);
                    Double current = logSnap.child("current").getValue(Double.class);
                    Double watt = logSnap.child("watt").getValue(Double.class);
                    Double kwh = logSnap.child("kwh").getValue(Double.class);
                    
                    LogEntry entry = new LogEntry(timestamp, temp, fanSpeed, voltage, current, watt, kwh);
                    logsList.add(entry);
                }
                adapter.notifyDataSetChanged();
            }
            @Override
            public void onCancelled(DatabaseError error) {}
        });
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
            
            // Set timestamp
            if (entry.timestamp != null) {
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
