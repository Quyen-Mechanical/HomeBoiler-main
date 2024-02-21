#include <Arduino.h>
#include <variable.h>
#include <string.h>
#include <temp.h>
#include <eepromsave.h>
// #include <EasyNextionLibrary.h>
// EasyNex myNex(Serial1);
int page_now;
void HMI_listen()
{
    while (Serial1.available())
    {
        String buffer = Serial1.readStringUntil('\n');
        int buff_length = buffer.length();
        char array_buffer[buff_length];
        buffer.toCharArray(array_buffer, buff_length);
        // Serial.print("buffer:");
        // Serial.println(array_buffer);
        int i = buffer.length();
        // Serial.print("i length:");
        Serial.println(i);
        // int startValueIndex = buffer.indexOf("[");
        // int endValueIndex = buffer.indexOf("]");
        // Serial.print("startValueIndex:");
        // Serial.println(startValueIndex);
        // Serial.print("endValueIndex:");
        // Serial.println(endValueIndex);

        char command[4];
        // for (int i = 0; i <= buff_length; i++)
        // {
        //     if (array_buffer[i] != '[')
        //     {
        //         command[i] = array_buffer[i];
        //     }
        //     else
        //         break;
        // }
        memcpy(command, array_buffer, 3);
        Serial.print("command:");
        Serial.println(command);
        int i_command = atoi(command);
        Serial.println(i_command);
        switch (i_command)
        {
        case 100:
            // Serial1.print("x_temp_now.val=");
            // Serial1.print((int)readTemp(26) * 100 / 10);
            // Serial1.print("\xFF\xFF\xFF");
            char page_buff[2];
            memcpy(page_buff, array_buffer + 4, 2);
            page_now = atoi(page_buff);
            Serial.print("page_now:");
            Serial.println(page_now);
            break;
        case 101:
            char temp[5];
            char pressure[6];
            char water_sensor_timming_char[11];
            char control[2];
            int i_temp;
            int i_pressure;
            int i_water_sensor_timming;
            int i_control;
            memcpy(temp, array_buffer + 4, 4);
            memcpy(pressure, array_buffer + 9, 4);
            memcpy(water_sensor_timming_char, array_buffer + 14, 10);
            memcpy(control, array_buffer + 25, 2);
            i_temp = atoi(temp);
            i_pressure = atoi(pressure);
            i_water_sensor_timming = atoi(water_sensor_timming_char);
            i_control = atoi(control);
            // Serial.print("temp:");
            // Serial.println(temp);
            // Serial.println(i_temp);
            // Serial.print("pressure:");
            // Serial.println(pressure);
            // Serial.println(i_pressure);
            // Serial.print("pressure:");
            // Serial.println(water_sensor_timming);
            // Serial.println(i_water_sensor_timming);
            // Serial.print("control:");
            // Serial.println(control);
            // Serial.println(i_control);
            if (i_temp <= 0 || i_pressure <= 0 || i_water_sensor_timming <= 0)
            {
                Serial1.print("page thong_bao");
                Serial1.print("\xFF\xFF\xFF");
                delay(10);
                Serial1.print("thong_bao.t1.txt=\"lỗi giá trị không thể bằng không!\"");
                Serial1.print("\xFF\xFF\xFF");
                delay(10);
            }
            else
            {
                temp_set = i_temp;
                pressure_set = i_pressure;
                water_sensor_timming = i_water_sensor_timming;
                control_type = i_control;
                if (temp_set > 1450)
                {
                    Serial1.print("page thong_bao");
                    Serial1.print("\xFF\xFF\xFF");
                    delay(10);
                    Serial1.print("thong_bao.t1.txt=\"nhiệt độ < 145 C!!!\"");
                    Serial1.print("\xFF\xFF\xFF");
                    delay(10);
                }
                else if (pressure_set > 320)
                {
                    Serial1.print("page thong_bao");
                    Serial1.print("\xFF\xFF\xFF");
                    delay(10);
                    Serial1.print("thong_bao.t1.txt=\"áp suất < 3.2 bar !!!\"");
                    Serial1.print("\xFF\xFF\xFF");
                    delay(10);
                }
                else
                {
                    write_temp_eeprom();
                    write_pressure_eeprom();
                    write_water_sensor_timming_eeprom();
                    write_control_type_eeprom();
                    Serial1.print("page thong_bao");
                    Serial1.print("\xFF\xFF\xFF");
                    delay(10);
                    Serial1.print("thong_bao.t1.txt=\"lưu thành công\"");
                    Serial1.print("\xFF\xFF\xFF");
                    delay(10);
                }
            }
            is_send_setting_hmi = false;
            break;
        case 102:

            break;
        default:
            return;
            break;
        }
    }
}