#include <Arduino.h>
int error;
float temp_now;
float temp_set;
float pressure_now;
float pressure_set;
int water_sensor_timming;
double kp, ki, kd;
double Setpoint, Input, Output;
int analog_start_button;
bool is_started;
bool check_water;
int analog_water_sensor_low;
int analog_water_sensor_mid;
int analog_water_sensor_high;
float water_level;
int control_type;
uint32_t start_pump;
uint32_t start_pump_timming;
bool is_pump_timming;

bool is_send_setting_hmi;