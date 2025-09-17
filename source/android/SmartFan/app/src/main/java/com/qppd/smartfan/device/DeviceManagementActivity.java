package com.qppd.smartfan.device;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import com.google.android.material.card.MaterialCardView;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.database.*;
import com.qppd.smartfan.R;
import java.util.ArrayList;

public class DeviceManagementActivity extends AppCompatActivity {
    private RecyclerView recyclerViewDevices;
    private TextView textViewNoDevices, textViewSelectedDevice;
    private MaterialCardView cardDeviceActions;
    private DeviceAdapter adapter;
    private ArrayList<DeviceInfo> deviceList = new ArrayList<>();
    private DatabaseReference dbRef;
    private String uid;
    private EditText editTextRename;
    private Button buttonRename, buttonUnlink;
    private String selectedDeviceId;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_device_management);

        recyclerViewDevices = findViewById(R.id.recyclerViewDevices);
        textViewNoDevices = findViewById(R.id.textViewNoDevices);
        textViewSelectedDevice = findViewById(R.id.textViewSelectedDevice);
        cardDeviceActions = findViewById(R.id.cardDeviceActions);
        editTextRename = findViewById(R.id.editTextRename);
        buttonRename = findViewById(R.id.buttonRename);
        buttonUnlink = findViewById(R.id.buttonUnlink);
        
        // Setup RecyclerView
        recyclerViewDevices.setLayoutManager(new LinearLayoutManager(this));
        adapter = new DeviceAdapter();
        recyclerViewDevices.setAdapter(adapter);
        
        dbRef = FirebaseDatabase.getInstance().getReference();
        uid = FirebaseAuth.getInstance().getCurrentUser().getUid();

        loadDevices();

        buttonRename.setOnClickListener(v -> renameDevice());
        buttonUnlink.setOnClickListener(v -> unlinkDevice());
    }

    private void loadDevices() {
        dbRef.child("users").child(uid).child("devices").addListenerForSingleValueEvent(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot snapshot) {
                deviceList.clear();
                if (snapshot.exists()) {
                    for (DataSnapshot deviceSnap : snapshot.getChildren()) {
                        String deviceId = deviceSnap.getKey();
                        dbRef.child("devices").child(deviceId).child("name").addListenerForSingleValueEvent(new ValueEventListener() {
                            @Override
                            public void onDataChange(DataSnapshot nameSnap) {
                                String name = nameSnap.getValue(String.class);
                                deviceList.add(new DeviceInfo(deviceId, name != null ? name : "Unnamed Device"));
                                adapter.notifyDataSetChanged();
                                updateUI();
                            }
                            @Override public void onCancelled(DatabaseError error) {}
                        });
                    }
                } else {
                    updateUI();
                }
            }
            @Override public void onCancelled(DatabaseError error) {}
        });
    }

    private void updateUI() {
        if (deviceList.isEmpty()) {
            textViewNoDevices.setVisibility(View.VISIBLE);
            recyclerViewDevices.setVisibility(View.GONE);
        } else {
            textViewNoDevices.setVisibility(View.GONE);
            recyclerViewDevices.setVisibility(View.VISIBLE);
        }
    }

    private void renameDevice() {
        if (selectedDeviceId == null) {
            Toast.makeText(this, "Select a device first", Toast.LENGTH_SHORT).show();
            return;
        }
        String newName = editTextRename.getText().toString().trim();
        if (newName.isEmpty()) {
            Toast.makeText(this, "Enter a new name", Toast.LENGTH_SHORT).show();
            return;
        }
        dbRef.child("devices").child(selectedDeviceId).child("name").setValue(newName);
        Toast.makeText(this, "Device renamed", Toast.LENGTH_SHORT).show();
        loadDevices();
    }

    private void unlinkDevice() {
        if (selectedDeviceId == null) {
            Toast.makeText(this, "Select a device first", Toast.LENGTH_SHORT).show();
            return;
        }
        dbRef.child("users").child(uid).child("devices").child(selectedDeviceId).removeValue();
        dbRef.child("devices").child(selectedDeviceId).child("owner").removeValue();
        Toast.makeText(this, "Device unlinked", Toast.LENGTH_SHORT).show();
        selectedDeviceId = null;
        cardDeviceActions.setVisibility(View.GONE);
        loadDevices();
    }

    // Simple Device Info class
    private static class DeviceInfo {
        String id;
        String name;

        DeviceInfo(String id, String name) {
            this.id = id;
            this.name = name;
        }
    }

    // Simple RecyclerView Adapter
    private class DeviceAdapter extends RecyclerView.Adapter<DeviceAdapter.DeviceViewHolder> {

        @NonNull
        @Override
        public DeviceViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            View view = LayoutInflater.from(parent.getContext()).inflate(android.R.layout.simple_list_item_1, parent, false);
            return new DeviceViewHolder(view);
        }

        @Override
        public void onBindViewHolder(@NonNull DeviceViewHolder holder, int position) {
            DeviceInfo device = deviceList.get(position);
            holder.textView.setText(device.name + " (" + device.id + ")");
            holder.itemView.setOnClickListener(v -> {
                selectedDeviceId = device.id;
                textViewSelectedDevice.setText(device.name + " (" + device.id + ")");
                cardDeviceActions.setVisibility(View.VISIBLE);
            });
        }

        @Override
        public int getItemCount() {
            return deviceList.size();
        }

        class DeviceViewHolder extends RecyclerView.ViewHolder {
            TextView textView;

            DeviceViewHolder(@NonNull View itemView) {
                super(itemView);
                textView = itemView.findViewById(android.R.id.text1);
            }
        }
    }
}
