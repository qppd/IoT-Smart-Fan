package com.qppd.smartfan.device;

import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.qppd.smartfan.R;

import java.util.List;

public class WiFiNetworkAdapter extends RecyclerView.Adapter<WiFiNetworkAdapter.NetworkViewHolder> {
    
    private List<ScanResult> networks;
    private OnNetworkSelectedListener listener;
    
    public interface OnNetworkSelectedListener {
        void onNetworkSelected(ScanResult network);
    }
    
    public WiFiNetworkAdapter(List<ScanResult> networks, OnNetworkSelectedListener listener) {
        this.networks = networks;
        this.listener = listener;
    }
    
    @NonNull
    @Override
    public NetworkViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.item_wifi_network, parent, false);
        return new NetworkViewHolder(view);
    }
    
    @Override
    public void onBindViewHolder(@NonNull NetworkViewHolder holder, int position) {
        ScanResult network = networks.get(position);
        holder.bind(network);
    }
    
    @Override
    public int getItemCount() {
        return networks.size();
    }
    
    class NetworkViewHolder extends RecyclerView.ViewHolder {
        
        private TextView textViewSSID, textViewSecurity, textViewFrequency;
        private ImageView imageViewWiFiStrength, imageViewLockIcon;
        
        public NetworkViewHolder(@NonNull View itemView) {
            super(itemView);
            
            textViewSSID = itemView.findViewById(R.id.textViewSSID);
            textViewSecurity = itemView.findViewById(R.id.textViewSecurity);
            textViewFrequency = itemView.findViewById(R.id.textViewFrequency);
            imageViewWiFiStrength = itemView.findViewById(R.id.imageViewWiFiStrength);
            imageViewLockIcon = itemView.findViewById(R.id.imageViewLockIcon);
            
            itemView.setOnClickListener(v -> {
                if (listener != null) {
                    listener.onNetworkSelected(networks.get(getAdapterPosition()));
                }
            });
        }
        
        public void bind(ScanResult network) {
            textViewSSID.setText(network.SSID);
            
            // Set security type
            String security = getSecurityType(network);
            textViewSecurity.setText(security);
            
            // Set frequency
            int frequency = network.frequency;
            if (frequency > 4900 && frequency < 5900) {
                textViewFrequency.setText("5 GHz");
            } else {
                textViewFrequency.setText("2.4 GHz");
            }
            
            // Set WiFi strength icon
            int level = WifiManager.calculateSignalLevel(network.level, 4);
            int strengthIcon;
            switch (level) {
                case 0:
                    strengthIcon = R.drawable.ic_wifi_strength_1;
                    break;
                case 1:
                    strengthIcon = R.drawable.ic_wifi_strength_2;
                    break;
                case 2:
                    strengthIcon = R.drawable.ic_wifi_strength_3;
                    break;
                default:
                    strengthIcon = R.drawable.ic_wifi_strength_4;
                    break;
            }
            imageViewWiFiStrength.setImageResource(strengthIcon);
            
            // Show lock icon for secured networks
            boolean isSecured = !security.equals("Open");
            imageViewLockIcon.setVisibility(isSecured ? View.VISIBLE : View.GONE);
        }
        
        private String getSecurityType(ScanResult network) {
            String capabilities = network.capabilities;
            
            if (capabilities.contains("WPA3")) {
                return "WPA3";
            } else if (capabilities.contains("WPA2")) {
                return "WPA2";
            } else if (capabilities.contains("WPA")) {
                return "WPA";
            } else if (capabilities.contains("WEP")) {
                return "WEP";
            } else {
                return "Open";
            }
        }
    }
}