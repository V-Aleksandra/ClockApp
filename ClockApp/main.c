#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "helper_library/helper.h"
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ssd1315.h"
#include "driverlib/i2c.h"
#include "inc/hw_ints.h"

#define MINUTES_MAX                59
#define SECONDS_MAX                59
#define HOURS_MAX                  23
#define NULL 0
#define STACK_SIZE 1024
#define DHT20_SLAVE_ADDRESS           0x38
#define DELAY_COUNT                2000000
#define DHT20_MEASUREMENT_LENGTH         7
#define MAX_STR_LEN                     2

typedef struct
{
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} time_of_day_t;

time_of_day_t time;
time_of_day_t alarm;

void init_i2c()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);

    GPIOPinConfigure(GPIO_PA6_I2C1SCL);
    GPIOPinConfigure(GPIO_PA7_I2C1SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);

    I2CMasterEnable(I2C1_BASE);
    I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), false);
}
void init_lcd()
{
    SSD1315_Init(I2C1_BASE);
    SSD1315_Clear();
}

void I2C1Wait()
{
    while (I2CMasterBusy(I2C1_BASE))
    {
        ;
    }
}

void print_led_time()
{
    char hours_string[MAX_STR_LEN];
    char minutes_string[MAX_STR_LEN];
    char seconds_string[MAX_STR_LEN];
    char colon[] = ":";

    uint8_t hours_string_len;
    uint8_t minutes_string_len;
    uint8_t seconds_string_len;

    hours_string_len = sprintf(hours_string, "%02u", time.hours);
    minutes_string_len = sprintf(minutes_string, "%02u", time.minutes);
    seconds_string_len = sprintf(seconds_string, "%02u", time.seconds);

    SSD1315_SetCursorPosition(0, 0);
    SSD1315_WriteString((uint8_t*) hours_string, hours_string_len);

    SSD1315_SetCursorPosition(0, 2);
    SSD1315_WriteString((uint8_t*) colon, 1);

    SSD1315_SetCursorPosition(0, 3);
    SSD1315_WriteString((uint8_t*) minutes_string, minutes_string_len);

    SSD1315_SetCursorPosition(0, 5);
    SSD1315_WriteString((uint8_t*) colon, 1);

    SSD1315_SetCursorPosition(0, 6);
    SSD1315_WriteString((uint8_t*) seconds_string, seconds_string_len);

}

void Timer0Isr(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, false);

}

void init_buzzer()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, 5 * 16000000); /* 5 seconds */
    TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0Isr);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    GPIOPinConfigure(GPIO_PB6_M0PWM0);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0,
    PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 4000);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, 2000);
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, false);
}

void print_time()
{
    print("Current time: %02u:%02u:%02u\n", time.hours, time.minutes,
          time.seconds);
}

void update_time()
{
    time.seconds++;
    if (time.seconds == 60)
    {
        time.seconds = 0;
        time.minutes++;
        if (time.minutes == 60)
        {
            time.minutes = 0;
            time.hours++;
            if (time.hours == 24)
            {
                time.hours = 0;
            }
        }
    }
}

void check_alarm()
{
    if ((alarm.hours == time.hours) && (alarm.minutes == time.minutes)
            && (alarm.seconds == time.seconds))
    {
        PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);
        TimerEnable(TIMER0_BASE, TIMER_A);

    }

}
void timer_task(void *parameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        taskENTER_CRITICAL();
        update_time();
        print_time();
        print_led_time();
        check_alarm();
        taskEXIT_CRITICAL();
        vTaskDelayUntil(&xLastWakeTime, 1000);
    }

}

void inputTime()
{
    print("Enter the current time:\n");

    do
    {
        print("Enter hours 0-23:");
        scan("%u", &time.hours);
    }
    while (time.hours > 23);

    do
    {
        print("Enter minutes 0-59:");
        scan("%u", &time.minutes);
    }
    while (time.minutes > 59);

    do
    {

        print("Enter seconds 0-59:");
        scan("%u", &time.seconds);
    }
    while (time.seconds > 59);

}

void inputAlarm_task(void *parameters)
{
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    while (1)
    {

        do
        {
            scan("%u", &hours);
        }
        while (hours > 23);

        do
        {
            scan("%u", &minutes);
        }
        while (minutes > 59);

        do
        {
            scan("%u", &seconds);
        }
        while (seconds > 59);

        taskENTER_CRITICAL();
        alarm.hours = hours;
        alarm.minutes = minutes;
        alarm.seconds = seconds;

        print("Alarm time: %02u:%02u:%02u\n", alarm.hours, alarm.minutes,
              alarm.seconds);
        taskEXIT_CRITICAL();

        // vTaskDelay(1000);
    }

}

int main(void)
{
    helper_library_init();
    inputTime();
    init_buzzer();
    init_i2c();
    init_lcd();

    xTaskCreate(timer_task, "Timer task", STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(inputAlarm_task, "Input alarm task", STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();

    while (1)
    {
        ;
    }
}
