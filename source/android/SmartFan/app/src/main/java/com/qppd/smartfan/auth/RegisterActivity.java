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

public class RegisterActivity extends AppCompatActivity {
    private TextInputEditText emailEditText, passwordEditText, confirmPasswordEditText;
    private TextInputLayout emailInputLayout, passwordInputLayout, confirmPasswordInputLayout;
    private MaterialButton registerButton, loginButton;
    private ProgressBar progressBar;
    private FirebaseAuth mAuth;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_register);

        // Initialize Material Design components
        emailEditText = findViewById(R.id.editTextEmail);
        passwordEditText = findViewById(R.id.editTextPassword);
        confirmPasswordEditText = findViewById(R.id.editTextConfirmPassword);
        
        // Get TextInputLayouts
        emailInputLayout = (TextInputLayout) emailEditText.getParent().getParent();
        passwordInputLayout = (TextInputLayout) passwordEditText.getParent().getParent();
        confirmPasswordInputLayout = (TextInputLayout) confirmPasswordEditText.getParent().getParent();
        
        registerButton = findViewById(R.id.buttonRegister);
        loginButton = findViewById(R.id.buttonLogin);
        progressBar = findViewById(R.id.progressBar);
        mAuth = FirebaseAuth.getInstance();

        registerButton.setOnClickListener(v -> registerUser());
        loginButton.setOnClickListener(v -> {
            finish();
        });
    }

    private void registerUser() {
        String email = emailEditText.getText().toString().trim();
        String password = passwordEditText.getText().toString().trim();
        String confirmPassword = confirmPasswordEditText.getText().toString().trim();
        
        // Clear previous errors
        emailInputLayout.setError(null);
        passwordInputLayout.setError(null);
        confirmPasswordInputLayout.setError(null);
        
        // Validation
        if (TextUtils.isEmpty(email)) {
            emailInputLayout.setError("Email is required");
            emailEditText.requestFocus();
            return;
        }
        
        if (!android.util.Patterns.EMAIL_ADDRESS.matcher(email).matches()) {
            emailInputLayout.setError("Please enter a valid email address");
            emailEditText.requestFocus();
            return;
        }
        
        if (TextUtils.isEmpty(password)) {
            passwordInputLayout.setError("Password is required");
            passwordEditText.requestFocus();
            return;
        }
        
        if (password.length() < 6) {
            passwordInputLayout.setError("Password must be at least 6 characters");
            passwordEditText.requestFocus();
            return;
        }
        
        if (TextUtils.isEmpty(confirmPassword)) {
            confirmPasswordInputLayout.setError("Please confirm your password");
            confirmPasswordEditText.requestFocus();
            return;
        }
        
        if (!password.equals(confirmPassword)) {
            confirmPasswordInputLayout.setError("Passwords do not match");
            confirmPasswordEditText.requestFocus();
            return;
        }
        
        // Show progress and disable button
        progressBar.setVisibility(View.VISIBLE);
        registerButton.setEnabled(false);
        
        mAuth.createUserWithEmailAndPassword(email, password)
                .addOnCompleteListener(task -> {
                    progressBar.setVisibility(View.GONE);
                    registerButton.setEnabled(true);
                    
                    if (task.isSuccessful()) {
                        Toast.makeText(RegisterActivity.this, "Account created successfully! Please sign in.", Toast.LENGTH_SHORT).show();
                        Intent intent = new Intent(RegisterActivity.this, LoginActivity.class);
                        intent.putExtra("email", email); // Pre-fill email in login
                        startActivity(intent);
                        finish();
                    } else {
                        String errorMessage = "Registration failed";
                        if (task.getException() != null) {
                            errorMessage = task.getException().getMessage();
                        }
                        Toast.makeText(RegisterActivity.this, errorMessage, Toast.LENGTH_LONG).show();
                    }
                });
    }
}
