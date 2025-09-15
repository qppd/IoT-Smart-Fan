#include "PIDConfig.h"

PIDConfig::PIDConfig(double* input, double* output, double* setpoint, double Kp, double Ki, double Kd, int direction) {
    _input = input;
    _output = output;
    _setpoint = setpoint;
    _pid = new PID(_input, _output, _setpoint, Kp, Ki, Kd, direction);
}

void PIDConfig::begin() {
    _pid->SetMode(AUTOMATIC);
}

void PIDConfig::setTunings(double Kp, double Ki, double Kd) {
    _pid->SetTunings(Kp, Ki, Kd);
}

void PIDConfig::setOutputLimits(double min, double max) {
    _pid->SetOutputLimits(min, max);
}

void PIDConfig::setMode(int mode) {
    _pid->SetMode(mode);
}

void PIDConfig::compute() {
    _pid->Compute();
}
