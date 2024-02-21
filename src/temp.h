#include <Arduino.h>
float readTemp(uint8_t sensor_pin)
{
    // const int analog=analogRead(sensor_pin);
    // float voltage=(float)analog*10.0/1024.0;
    // float temp=voltage*150.0/10.0;
    // Serial.print("Analog:");
    // Serial.println(analog);
    // Serial.print("voltage:");
    // Serial.println(voltage);
    // Serial.print("temp:");
    // Serial.println(temp);
    // return (float)analogRead(sensor_pin) * 10.0 / 1024.0 * 150.0 / 10.0 ;
    return (float)analogRead(sensor_pin) * 10.0 / 1024.0 * 150.0+26; //=>102 độ C -> 1020 để up lên hmi và eeprom +pid
}
float readPressure(uint8_t sensor_pin)
{
    // const int analog=analogRead(sensor_pin);
    // float voltage=(float)analog*10.0/1024.0;
    // float pre=voltage*40.0/10.0;
    // Serial.print("Analog:");
    // Serial.println(analog);
    // Serial.print("voltage:");
    // Serial.println(voltage);
    // Serial.print("press:");
    // Serial.println(pre);
    //return (float)analogRead(sensor_pin) * 10.0 / 1024.0 * 4.0 / 10.0;
    return (float)analogRead(sensor_pin) / 1024.0 * 4.0 ; //=>1.2bar độ C -> 120 để up lên hmi và eeprom +pid
}