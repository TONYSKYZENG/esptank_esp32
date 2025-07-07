#ifndef _MOTOR_H_
#define _MOTOR_H_
#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"

void initMotors(void);
void initScreen(void);
void setLeftMotor(int speed);
void setRightMotor(int speed);
void paraseMotor(char *str);

#endif