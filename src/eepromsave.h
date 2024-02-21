#include <Arduino.h>
#include <eeprom.h>
void writeIntIntoEEPROM(int address, int number)
{
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}
int readIntFromEEPROM(int address)
{
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}
void read_kp_eeprom()
{
  int KP = readIntFromEEPROM(0);
  delay(5);
  if (KP > 0)
  {
    kp = (double)(KP / 100.0);
  }
  else
  {
    writeIntIntoEEPROM(0, 12000);
    delay(5);
    kp = 120;
  }
}
void write_kp_eeprom()
{
  writeIntIntoEEPROM(0, (int)(kp * 100));
  delay(5);
}
void read_ki_eeprom()
{
  int KI = readIntFromEEPROM(4);
  delay(5);
  if (KI > 0)
  {
    ki = (double)(KI / 100.0);
  }
  else
  {
    writeIntIntoEEPROM(4, 1000);
    delay(5);
    ki = 10;
  }
}
void write_ki_eeprom()
{
  writeIntIntoEEPROM(4, (int)(ki * 100));
  delay(5);
}

void read_kd_eeprom()
{
  int KD = readIntFromEEPROM(8);
  delay(5);
  if (KD > 0)
  {
    kd = (double)(KD / 100.0);
  }
  else
  {
    writeIntIntoEEPROM(8, 4000);
    delay(5);
    kd = 40;
  }
}
void write_kd_eeprom()
{
  writeIntIntoEEPROM(8, (int)(kd * 100));
  delay(5);
}
void read_temp_eeprom()
{
  int TEMP = readIntFromEEPROM(12);
  delay(5);
  if (TEMP >= 0)
  {
    temp_set = (float)TEMP;
  }
  else
  {
    writeIntIntoEEPROM(12, 10200);
    delay(5);
    temp_set = 10200;
  }
}
void write_temp_eeprom()
{
  writeIntIntoEEPROM(12, (int)(temp_set));
  delay(5);
}
void read_pressure_eeprom()
{
  int PRESS = readIntFromEEPROM(16);
  delay(5);
  if (PRESS >= 0)
  {
    pressure_set = (float)PRESS;
  }
  else
  {
    writeIntIntoEEPROM(16, 10);
    delay(5);
    temp_set = 10;
  }
}
void write_pressure_eeprom()
{
  writeIntIntoEEPROM(16, (int)pressure_set);
  delay(5);
}
void read_water_sensor_timming_eeprom()
{
  int WLST = readIntFromEEPROM(20);
  delay(5);
  if (WLST >= 0)
  {
    water_sensor_timming = WLST;
  }
  else
  {
    writeIntIntoEEPROM(20, 5000);
    delay(5);
    water_sensor_timming = 5000;
  }
}
void write_water_sensor_timming_eeprom()
{
  writeIntIntoEEPROM(20, water_sensor_timming);
  delay(5);
}

void read_control_type_eeprom()
{
  int CTP = readIntFromEEPROM(24);
  delay(5);
  if (CTP >= 0)
  {
    control_type = CTP;
  }
  else
  {
    writeIntIntoEEPROM(24, 1);
    delay(5);
    control_type = 1;
  }
}
void write_control_type_eeprom()
{
  writeIntIntoEEPROM(24, control_type);
  delay(5);
}