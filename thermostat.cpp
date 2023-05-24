#include "mbed.h"
#include "board_freedom.h"
#include "oled_ssd1322.h"
#include "adc.h"
#include <cstdio>

#define MESSAGE_MAX_SIZE 50

// main() runs in its own thread in the OS
int main()
{
    board_init();
    u8g2_ClearBuffer(&oled);
    // Ensure a font is set (only required once)
    u8g2_SetFont(&oled, u8g2_font_nokiafc22_tr);
    u8g2_SendBuffer(&oled);
    

    // Ready a single reusable buffer for writing text to.
    char message[MESSAGE_MAX_SIZE + 1];
    message[MESSAGE_MAX_SIZE] = '\0';

    // 
    DigitalOut redLed(PTB2);
    redLed = 0;
    DigitalOut greenLed(PTB3);
    greenLed = 0;

    PwmOut heater_power(PTC2);
    heater_power = 0;

    while (true) {
        // FR-READ-TEMPERATURE
        uint16_t analog_in_value = adc_read(0);
        float voltage = (analog_in_value / 65535.0) * 3.0;
        float temperature = (voltage * 1000 - 400) / 19.5;

        // prevent overheating
        if(temperature > 38) {
            break;
        } 
        // FR-LIGHT-GREEN
        if(temperature <= 35 && temperature >= 30) {
            greenLed = !greenLed;
            heater_power = 0;
        } 
        // FR-HEATER and FR-LIGHT-RED
        if(temperature  < 30 ){
            heater_power = 1;
            redLed = 1;
        }


        // FR-DISPLAY-TEMPERATURE
        snprintf(message, MESSAGE_MAX_SIZE, "Temperature is %5.02f", temperature);
        // Clear the buffer
        u8g2_ClearBuffer(&oled);
        u8g2_DrawUTF8(&oled, 10, 10, message);
        u8g2_SendBuffer(&oled);

        // FR-HEATER 
        if(temperature  < 30 ){
            heater_power = 1;
            redLed = 1;
        }

        // Print mesaage to serial monitor via USB
        printf("%s\n", message);

        ThisThread::sleep_for(200ms);

        // Turn off LED lights for next loop
        greenLed = 0;
        redLed = 0;
    }
}


