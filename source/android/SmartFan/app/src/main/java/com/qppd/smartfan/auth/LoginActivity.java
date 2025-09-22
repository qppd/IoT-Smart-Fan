package com.qppd.smartfan.auth;

import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.Toast;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import com.google.android.material.button.MaterialButton;
import com.google.android.material.textfield.TextInputEditText;
import com.google.android.material.textfield.TextInputLayout;
import com.google.firebase.auth.FirebaseAuth;
import com.qppd.smartfan.R;

public class LoginActivity extends AppCompatActivity {
    private TextInputEditText emailEditText, passwordEditText;
    private TextInputLayout emailInputLayout, passwordInputLayout;
    private MaterialButton loginButton, registerButton;
    private ProgressBar progressBar;
    private FirebaseAuth mAuth;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        mAuth = FirebaseAuth.getInstance();
        if (mAuth.getCurrentUser() != null) {
            // User already logged in, go to dashboard
            startActivity(new Intent(LoginActivity.this, com.qppd.smartfan.MainActivity.class));
            finish();
            return;
        }

        // Initialize Material Design components
        emailEditText = findViewById(R.id.editTextEmail);
        passwordEditText = findViewById(R.id.editTextPassword);
        emailInputLayout = (TextInputLayout) emailEditText.getParent().getParent();
        passwordInputLayout = (TextInputLayout) passwordEditText.getParent().getParent();
        loginButton = findViewById(R.id.buttonLogin);
        registerButton = findViewById(R.id.buttonRegister);
        progressBar = findViewById(R.id.progressBar);

        // Pre-fill email if coming from registration
        String prefilledEmail = getIntent().getStringExtra("email");
        if (prefilledEmail != null) {
            emailEditText.setText(prefilledEmail);
            passwordEditText.requestFocus();
        }

        loginButton.setOnClickListener(v -> loginUser());
        registerButton.setOnClickListener(v -> {
            Intent intent = new Intent(LoginActivity.this, RegisterActivity.class);
            startActivity(intent);
        });
    }

    private void loginUser() {
        String email = emailEditText.getText().toString().trim();
        String password = passwordEditText.getText().toString().trim();
        
        // Clear previous errors
        emailInputLayout.setError(null);
        passwordInputLayout.setError(null);
        
        if (TextUtils.isEmpty(email)) {
            emailInputLayout.setError("Email is required");
            emailEditText.requestFocus();
            return;
        }
        if (TextUtils.isEmpty(password)) {
            passwordInputLayout.setError("Password is required");
            passwordEditText.requestFocus();
            return;
        }
        
        // Show progress and disable button
        progressBar.setVisibility(View.VISIBLE);
        loginButton.setEnabled(false);
        
        mAuth.signInWithEmailAndPassword(email, password)
                .addOnCompleteListener(task -> {
                    progressBar.setVisibility(View.GONE);
                    loginButton.setEnabled(true);
                    
                    if (task.isSuccessful()) {
                        // Check if user has configured device ID
                        com.google.firebase.database.DatabaseReference dbRef = com.google.firebase.database.FirebaseDatabase.getInstance().getReference();
                        String uid = mAuth.getCurrentUser().getUid();
                        dbRef.child("smartfan").child("users").child(uid).child("deviceId").addListenerForSingleValueEvent(new com.google.firebase.database.ValueEventListener() {
                            @Override
                            public void onDataChange(com.google.firebase.database.DataSnapshot snapshot) {
                                String deviceId = snapshot.getValue(String.class);
                                
                                if (deviceId == null || deviceId.isEmpty()) {
                                    // No device ID configured, set default and navigate
                                    dbRef.child("smartfan").child("users").child(uid).child("deviceId").setValue("SmartFan_ESP8266_000");
                                }
                                
                                Toast.makeText(LoginActivity.this, "Welcome back!", Toast.LENGTH_SHORT).show();
                                startActivity(new Intent(LoginActivity.this, com.qppd.smartfan.MainActivity.class));
                                finish();
                            }
                            @Override
                            public void onCancelled(com.google.firebase.database.DatabaseError error) {
                                // On error, still proceed to main activity with default device ID
                                Toast.makeText(LoginActivity.this, "Welcome back!", Toast.LENGTH_SHORT).show();
                                startActivity(new Intent(LoginActivity.this, com.qppd.smartfan.MainActivity.class));
                                finish();
                            }
                        });
                    } else {
                        String errorMessage = "Login failed";
                        if (task.getException() != null) {
                            errorMessage = task.getException().getMessage();
                        }
                        Toast.makeText(LoginActivity.this, errorMessage, Toast.LENGTH_LONG).show();
                    }
                });
    }
}
