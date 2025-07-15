#include "motor.h"
#include <string.h>
//#include "ssd1306.h"



#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_vfs_semihost.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "sdmmc_cmd.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/apps/netbiosns.h"
//#include "protocol_examples_common.h"
//#include "ssd1306.h"
#include "esp_netif.h"
#include "esp_event.h" // Add this line for event loop
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sound.h"
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO_L0          (18) // Define the output GPIO
#define LEDC_OUTPUT_IO_L1          (19) // Define the output GPIO
#define LEDC_OUTPUT_IO_R0          (7) // Define the output GPIO
#define LEDC_OUTPUT_IO_R1          (8) // Define the output GPIO
#define LEDC_CHANNEL_L0            LEDC_CHANNEL_0
#define LEDC_CHANNEL_L1            LEDC_CHANNEL_1
#define LEDC_CHANNEL_R0            LEDC_CHANNEL_2
#define LEDC_CHANNEL_R1            LEDC_CHANNEL_3
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4095) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz
//SSD1306_t ssd1306Dev;
static const char *TAG = "example";
void initMotors(void) {
       /**
     * @brief L0
     * 
     */
    int duty =0;
   
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channelL0 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_L0,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO_L0,
        .duty           = duty, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channelL0));
   // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channelL1 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_L1,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO_L1,
        .duty           = duty, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channelL1));
    ledc_channel_config_t ledc_channelR0 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_R0,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO_R0,
        .duty           = duty, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channelR0));
    ledc_channel_config_t ledc_channelR1 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_R1,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO_R1,
        .duty           = duty, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channelR1));



    //displayIP(&ssd1306Dev);
   // playMusicLoop(1);
}
void setLeftMotor(int speed) {
    if(speed<0) {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_L0, -speed);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_L1, 10);
    }
    else if(speed==0) {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_L0, 0);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_L1, 0);
    }
    else {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_L0, 0);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_L1, speed);
    }
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_L0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_L1);
}

void setRightMotor(int speed) {
    if(speed<0) {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_R0, -speed);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_R1, 10);
    }
    else if(speed==0) {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_R0, 0);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_R1, 0);
    }
    else {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_R0, 0);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_R1, speed);
    }
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_R0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_R1);
}
char speed_buffer[1024];
char* extractBetweenHashes(const char* input) {
    if (input == NULL) {
        return NULL;
    }

    const char* first_hash = strchr(input, '#');
    if (first_hash == NULL) {
        return NULL;
    }

    const char* second_hash = strchr(first_hash + 1, '#');
    if (second_hash == NULL) {
        return NULL;
    }

    // 计算两个#之间的字符数
    size_t length = second_hash - first_hash - 1;
    if (length <= 0) {
        return NULL;
    }

    // 分配内存并复制子字符串
   /* char* result = (char*)malloc(length + 1);
    if (result == NULL) {
        return NULL;
    }*/

    strncpy(speed_buffer, first_hash + 1, length);
    speed_buffer[length] = '\0';
    return speed_buffer;
}

void paraseMotor(char *str){
    //ssd1306_display_text(&ssd1306Dev,2,str,strlen(str),0);
    if(strstr(str, "<#")!=NULL) {
        int speed_val = atoi(extractBetweenHashes(str));
        if(speed_val>=1000||speed_val<0)
        {
            speed_val = 0;
        }
        int speed_set = 8*speed_val;
        printf("set speed %d\n",speed_set);
        setLeftMotor(speed_set);
       // setRightMotor(speed_set);
       // playMusicLoop(mp3_data_start_engine,mp3_data_end_engine);
    }
     else if(strstr(str, ">#")!=NULL) {
         int speed_val = atoi(extractBetweenHashes(str));
        if(speed_val>=1000||speed_val<0)
        {
            speed_val = 0;
        }
        int speed_set = -8*speed_val;
        setLeftMotor(speed_set);
        printf("set speed %d\n",speed_set);
       // setRightMotor(speed_set);
       // playMusicLoop(mp3_data_start_engine,mp3_data_end_engine);
    }
    else if(strstr(str, "MOTOR_P3")!=NULL) {
        setLeftMotor(7100);
       // setRightMotor(7100);
       // playMusicLoop(mp3_data_start_engine,mp3_data_end_engine);
    }
    else if (strstr(str, "MOTOR_P2")!=NULL)
    {
       setLeftMotor(6000);
      // setRightMotor(6000);
       // playMusicLoop(mp3_data_start_engine,mp3_data_end_engine);
    }
    else if (strstr(str, "MOTOR_P1")!=NULL)
    {
       setLeftMotor(4096);
     //  setRightMotor(4096);
       // playMusicLoop(mp3_data_start_engine,mp3_data_end_engine);
    }
    else if (strstr(str, "MOTOR_S")!=NULL)
    {
       setLeftMotor(0);
      // setRightMotor(0);
       playMusicLoop(mp3_data_start_idel,mp3_data_end_idel);
    }

    else if (strstr(str, "MOTOR_N1")!=NULL)
    {
       setLeftMotor(-6000);
     //  setRightMotor(-6000);
      //  playMusicLoop(mp3_data_start_engine,mp3_data_end_engine);
    }
    else if (strstr(str, "MOTOR_L")!=NULL)
    {
       setRightMotor(6000);
       //setRightMotor(-6000);
        //playMusicLoop(mp3_data_start_engine,mp3_data_end_engine);
    }
    else if (strstr(str, "MOTOR_R")!=NULL)
    {
     
       setRightMotor(0);
     // setRightMotor(6000);
     //   playMusicLoop(mp3_data_start_engine,mp3_data_end_engine);
    }



}