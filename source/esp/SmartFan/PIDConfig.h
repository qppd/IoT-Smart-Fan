#ifndef PIDCONFIG_H
#define PIDCONFIG_H

#include <Arduino.h>
#include <PID_v1.h>

class PIDConfig {
public:
    PIDConfig(double* input, double* output, double* setpoint, double Kp, double Ki, double Kd, int direction = DIRECT);
    void begin();
    void setTunings(double Kp, double Ki, double Kd);
    void setOutputLimits(double min, double max);
    void setMode(int mode);
    void compute();
private:
    PID* _pid;
    double* _input;
    double* _output;
    double* _setpoint;
};

#endif // PIDCONFIG_H
