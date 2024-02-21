#include <Arduino.h>
#include <SPI.h>
// #include <temp.h>
#include <hmi.h>

#include <PID_v1.h>
// define water level schematic
#define MCU_ANALOG_WSS_OUT0 20 // A2
#define MCU_ANALOG_WSS_OUT1 21 // A3
#define MCU_ANALOG_WSS_OUT2 22 // A4
#define MCU_ANALOG_WSS_OUT3 23 // A5
// define water level in use
#define WATER_MIN_LEVEL 21    // A2
#define WATER_ACTIVE_LEVEL 22 // A3
#define WATER_MAX_LEVEL 23    // A4

// define analog input
#define A_IN_0 29 // A11
#define A_IN_1 26 // A8
#define A_IN_2 25 // A7
#define A_IN_3 24 // A6
// define analog input of sensor
#define PRESSURE_10V_4BAR 29 // A11
#define PT100_10V_150DC 26   // A7
// define pwm output
#define PWM0 10
#define PWM1 9
// define analog output
#define ANALOG_OUT0 10
#define HEAT_CONTROL_PWM 9
// define expander gpio output
#define SPI_CLK 15
#define SPI_MISO 14
#define SPI_MOSI 16
#define LATCH_595 19

// define led_7segment digits
#define NUM0 0B10111110;
#define NUM1 0B00001100;
#define NUM2 0B01110110;
#define NUM3 0B01011110;
#define NUM4 0B11001100;
#define NUM5 0B11011010;
#define NUM6 0B11111010;
#define NUM7 0B00001110;
#define NUM8 0B11111110;
#define NUM9 0B11011110;
#define DIGIT_A 0B11101110;
#define DIGIT_B 0B11111000;
#define DIGIT_C 0B10110010;
#define DIGIT_D 0B01111100;
#define DIGIT_E 0B11110010;
#define DIGIT_F 0B11100010;

// define relay output
#define RELAY_PUMP_SOLENOID 5

uint8_t REGISTER_595_OUTPUT; // REGISTER_595_0
uint8_t REGISTER_595_LED;    // REGISTER_595_1
// 0B    0    0    0    0    0    0    0    0
//       F    G    E    D    C    B    A   NOT USED

uint8_t REGISTER_595_RELAY; // REGISTER_595_2
// 0B    0    0    0    0    0    0    0    0
//      RL6  RL5  RL4  RL3  RL2  RL1  RL0   NOT USED
uint32_t ms;
uint32_t start1000_ms;
uint32_t start500_ms;
uint32_t start250_ms;
double KP_10, KI_10, KD_10;
PID myPID(&Input, &Output, &Setpoint, 2, 5, 1, DIRECT);
// uint32_t transfer_ms;
int i;

bool relayLatch;
uint8_t volt2pwm(float volt)
{
  return uint8_t(volt / 5 * 255);
}

void setup()
{
  read_kp_eeprom();
  read_ki_eeprom();
  read_kd_eeprom();
  read_temp_eeprom();
  read_pressure_eeprom();
  read_water_sensor_timming_eeprom();
  read_control_type_eeprom();
  KP_10 = kp / 10.0;
  KI_10 = ki / 10.0;
  KD_10 = kd / 10.0;
  pinMode(LATCH_595, OUTPUT);
  pinMode(ANALOG_OUT0, OUTPUT);
  pinMode(HEAT_CONTROL_PWM, OUTPUT);
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();

  digitalWrite(LATCH_595, 0);
  SPI.transfer(REGISTER_595_OUTPUT);
  SPI.transfer(REGISTER_595_LED);
  SPI.transfer(REGISTER_595_RELAY);
  digitalWrite(LATCH_595, 1);

  REGISTER_595_OUTPUT = 0B11111111;
  REGISTER_595_LED = NUM0;
  REGISTER_595_RELAY = 0B00000001;

  // Print temperature sensor details.
  Serial1.begin(115200);
  myPID.SetMode(AUTOMATIC);
  delay(10000);
  Serial1.print("page 0");
  Serial1.print("\xFF\xFF\xFF");
}

void loop()
{
  ms = millis();
  // if (millis() - transfer_ms > 40)
  // {
  digitalWrite(LATCH_595, 0);
  SPI.transfer(REGISTER_595_OUTPUT);
  SPI.transfer(REGISTER_595_LED);
  SPI.transfer(REGISTER_595_RELAY);
  // transfer_ms = millis();
  // }
  digitalWrite(LATCH_595, 1);

  pressure_now = readPressure(PRESSURE_10V_4BAR);
  temp_now = readTemp(PT100_10V_150DC);
  analog_start_button = analogRead(MCU_ANALOG_WSS_OUT0);
  analog_water_sensor_low = analogRead(MCU_ANALOG_WSS_OUT1);
  analog_water_sensor_mid = analogRead(MCU_ANALOG_WSS_OUT2);
  analog_water_sensor_high = analogRead(MCU_ANALOG_WSS_OUT3);
  water_level = (528.0 - (analog_water_sensor_low + analog_water_sensor_mid + analog_water_sensor_high * 5)) + 200 / 528.0 * 100.0;
  if (temp_now < 2)
  {
    error = 5; // Lỗi không có cảm biến nhiệt độ!
  }
  if (temp_now > 1050 && pressure_now == 0)
  {
    error = 6; // Lỗi không có cảm biến áp suất!
  }
  if (temp_now >= 1450)
  {
   // error = 7; // Lỗi quá nhiệt!
  }
  if (control_type == 1)
  {
    Setpoint = temp_set;
    Input = double(temp_now);
  }
  else
  {
    Setpoint = pressure_set;
    Input = double(pressure_now);
  }
  if (is_started)
  {
    myPID.Compute();
    analogWrite(HEAT_CONTROL_PWM, Output);
    REGISTER_595_LED = 0B01100000;
  }
  else
  {
    analogWrite(HEAT_CONTROL_PWM, 0);
    REGISTER_595_RELAY &= ~(1 << 6);
  }
  // switch (error)
  // {
  // case 1:
  //   /* code */
  //   REGISTER_595_LED = NUM1;
  //   Serial1.print("page thong_bao");
  //   Serial1.print("\xFF\xFF\xFF");
  //   delay(10);
  //   Serial1.print("thong_bao.t1.txt=\"lỗi không bơm được nước vào mức thấp nhất!\"");
  //   Serial1.print("\xFF\xFF\xFF");
  //   delay(1000);
  //   break;
  // case 2:
  //   /* code */
  //   REGISTER_595_LED = NUM2;
  //   Serial1.print("page thong_bao");
  //   Serial1.print("\xFF\xFF\xFF");
  //   delay(10);
  //   Serial1.print("thong_bao.t1.txt=\"lỗi không bơm được nước vào mức trung bình \nsau 4 tiếng!\"");
  //   Serial1.print("\xFF\xFF\xFF");
  //   delay(1000);
  //   break;
  // case 3:
  //   /* code */
  //   REGISTER_595_LED = NUM3;
  //   Serial1.print("page thong_bao");
  //   Serial1.print("\xFF\xFF\xFF");
  //   delay(10);
  //   Serial1.print("thong_bao.t1.txt=\"lỗi nước quá đầy!\"");
  //   Serial1.print("\xFF\xFF\xFF");
  //   delay(1000);
  //   break;
  // case 4:
  //   /* code */
  //   REGISTER_595_LED = NUM4;
  //   Serial1.print("page thong_bao");
  //   Serial1.print("\xFF\xFF\xFF");
  //   delay(10);
  //   Serial1.print("thong_bao.t1.txt=\"Lỗi Khởi động khi chưa đủ nước!\"");
  //   Serial1.print("\xFF\xFF\xFF");
  //   delay(1000);
  // case 5:
  //   /* code */
  //   REGISTER_595_LED = NUM5;
  //   Serial1.print("page thong_bao");
  //   Serial1.print("\xFF\xFF\xFF");
  //   delay(10);
  //   Serial1.print("thong_bao.t1.txt=\"Lỗi không có cảm biến nhiệt độ!!\"");
  //   Serial1.print("\xFF\xFF\xFF");
  //   delay(1000);
  //   break;
  // case 6:
  //   /* code */
  //   REGISTER_595_LED = NUM6;
  //   Serial1.print("page thong_bao");
  //   Serial1.print("\xFF\xFF\xFF");
  //   delay(10);
  //   Serial1.print("thong_bao.t1.txt=\"Lỗi không có cảm biến áp suất!!\"");
  //   Serial1.print("\xFF\xFF\xFF");
  //   delay(1000);
  //   break;
  // case 7:
  //   /* code */
  //   REGISTER_595_LED = NUM7;
  //   Serial1.print("page thong_bao");
  //   Serial1.print("\xFF\xFF\xFF");
  //   delay(10);
  //   Serial1.print("thong_bao.t1.txt=\"Lỗi quá nhiệt!!\"");
  //   Serial1.print("\xFF\xFF\xFF");
  //   delay(1000);
  //   break;
  // default:
  //   break;
  // }
  // analogWrite(ANALOG_OUT0, 255);
  //  test analog water sensor
  //   Serial.print("A_IN0:");
  //   Serial.println(analogRead(MCU_ANALOG_WSS_OUT0));
  //   Serial.print("A_IN1:");
  //   Serial.println(analogRead(MCU_ANALOG_WSS_OUT1));
  //   Serial.print("A_IN2:");
  //   Serial.println(analogRead(MCU_ANALOG_WSS_OUT2));
  //   Serial.print("A_IN3:");
  //   Serial.println(analogRead(MCU_ANALOG_WSS_OUT3));
  //   delay(500);
  //  test relay output
  //  if (i > 7)
  //  {
  //    i = 0;
  //    REGISTER_595_RELAY = 0;
  //  }
  //  else
  //  {
  //    REGISTER_595_RELAY |=1 << i;
  //    i++;
  //  }
  //  delay(200);
  //  if ((millis() - start_ms) > 1000)
  //  {
  //    if (relayLatch)
  //    {
  //      relayLatch = false;
  //      REGISTER_595_RELAY |= 1 << RELAY_PUMP_SOLENOID;
  //    }
  //    else
  //    {
  //      REGISTER_595_RELAY &= ~(1 << RELAY_PUMP_SOLENOID);
  //      relayLatch = true;
  //    }
  //    start_ms = millis();
  //  }

  // test sensor
  // if (millis() - start_ms > 500)
  // {
  //   start_ms = millis();
  //   Serial.print("PRESURE:");
  //   Serial.println(analogRead(A_IN_0));
  //   Serial.print("TEMP:");
  //   Serial.println(analogRead(A_IN_1));
  //   Serial.print("A2:");
  //   Serial.println(analogRead(A_IN_2));
  //   Serial.print("A3:");
  //   Serial.println(analogRead(A_IN_3));

  // }
  if (ms - start250_ms > 250)
  {
    start250_ms = ms;

    if (error && is_started)
    {
      // buzzer error
      if (REGISTER_595_RELAY & (1 << 7))
      {
        REGISTER_595_RELAY &= ~(1 << 7);
      }
      else
      {
        REGISTER_595_RELAY |= 1 << 7;
      }
    }
  }
  if (ms - start500_ms > 500)
  {
    start500_ms = ms;
    // led running
    if (is_started)
    {
      if (REGISTER_595_RELAY & (1 << 6))
      {
        REGISTER_595_RELAY &= ~(1 << 6);
      }
      else
      {
        REGISTER_595_RELAY |= 1 << 6;
      }
    }
    // Serial.println(readTemp(PT100_10V_150DC));
    // Serial1.print("x_temp_now.val=");
    // Serial1.print((int)readTemp(PT100_10V_150DC) * 100 / 10);
    // Serial1.print("\xFF\xFF\xFF");
  }
  if (ms - start1000_ms > 1000)
  {
    start1000_ms = ms;
    // check start button
    if (error == 0)
    {
      if (analog_start_button > 10)
      {
        if (analog_water_sensor_low < 30 && analog_water_sensor_mid < 30 && analog_water_sensor_high > 0)
        {
          is_started = true;
        }
        else
        {
          // if (check_water == false)
          // {
          //   error = 4; // lỗi không đủ nước mà nhấn khởi động
          // }
        }
      }
      else
      {
        is_started = false;
      }
      // check water level sensor
      if (analog_water_sensor_low < 30)
      {
        if (analog_water_sensor_mid < 30)
        {
          if (analog_water_sensor_high <= 0)
          {

            // error = 3; // lỗi nước quá đầy
            REGISTER_595_RELAY &= ~(1 << 5);
          }
          if (!is_pump_timming)
          {
            is_pump_timming = true;
            start_pump_timming = ms;
          }

          // REGISTER_595_RELAY &= ~(1 << 5);
        }
        else
        {
          if (is_started == false)
          {
            check_water = true;
          }
          if (start_pump == 0)
          {
            start_pump = ms;
            REGISTER_595_RELAY |= 1 << 5;
          }
          else if (ms - start_pump > 14400000)
          {
            error = 2; // lỗi không bơm được nước vào mức trung bình
          }
        }
      }
      else
      {
        if (start_pump == 0)
        {
          start_pump = ms;
          REGISTER_595_RELAY |= 1 << 5;
        }
        else if (ms - start_pump > 600000)
        {
          error = 1; // lỗi không bơm được nước vào mức thấp nhất
        }
      }
    }
    else
    {
      REGISTER_595_RELAY &= ~(1 << 5);
    }

    switch (page_now)
    {
    case 0:
      is_send_setting_hmi = false;
      Serial1.print("x_temp_now.val=");
      Serial1.print((int)(readTemp(PT100_10V_150DC)));
      Serial1.print("\xFF\xFF\xFF");
      Serial1.print("x_pressnow.val=");
      Serial1.print((int)(readPressure(PRESSURE_10V_4BAR) * 100.0));
      Serial1.print("\xFF\xFF\xFF");
      Serial1.print("j0.val=");
      Serial1.print((int)water_level);
      Serial1.print("\xFF\xFF\xFF");
      break;
    case 1:
      if (!is_send_setting_hmi)
      {
        Serial1.print("x_set_temp.val=");
        Serial1.print((int)(temp_set));
        Serial1.print("\xFF\xFF\xFF");
        Serial1.print("x_set_press.val=");
        Serial1.print((int)(pressure_set));
        Serial1.print("\xFF\xFF\xFF");
        if (control_type > 0)
        {
          Serial1.print("r_temp.val=");
          Serial1.print(1);
          Serial1.print("\xFF\xFF\xFF");
          Serial1.print("r_press.val=");
          Serial1.print(0);
          Serial1.print("\xFF\xFF\xFF");
        }
        else
        {
          Serial1.print("r_temp.val=");
          Serial1.print(0);
          Serial1.print("\xFF\xFF\xFF");
          Serial1.print("r_press.val=");
          Serial1.print(1);
          Serial1.print("\xFF\xFF\xFF");
        }
        is_send_setting_hmi = true;
      }
    case 4:
      break;
    default:
      // is_send_setting_hmi = false;
      break;
    }
    // Serial.println("Setpoint");
    // Serial.println(Setpoint);
    // Serial.println(temp_set);
    // Serial.println(pressure_set);
    // Serial.println(Input);
    // Serial.println("Output");
    // Serial.println(Output);
    // Serial.println(kp);
    // Serial.println(ki);
    // Serial.println(kd);
    // Serial.println("sensor");
    // Serial.println(temp_now);
    // Serial.println(pressure_now);
    // Serial.println(water_sensor_timming);
    Serial.print("A_IN0:");
    Serial.println(analogRead(MCU_ANALOG_WSS_OUT0));
    Serial.print("A_IN1:");
    Serial.println(analogRead(MCU_ANALOG_WSS_OUT1));
    Serial.print("A_IN2:");
    Serial.println(analogRead(MCU_ANALOG_WSS_OUT2));
    Serial.print("A_IN3:");
    Serial.println(analogRead(MCU_ANALOG_WSS_OUT3));
    Serial.print("error:");
    Serial.println(error);
  }
  if (is_pump_timming)
  {

    if ((ms - start_pump_timming) >= 5000)
    {
      is_pump_timming = false;
      REGISTER_595_RELAY &= ~(1 << 5);
      start_pump = 0;
    }
  }
  HMI_listen();
}
