#include <stdio.h>  
#include "freertos/FreeRTOS.h"  
#include "freertos/task.h"  
#include "driver/gpio.h"  
#include "sdkconfig.h"  
#include "input_iot.h"
#include "output_iot.h"
#include "freertos/event_groups.h"
#include "freertos/portmacro.h"

#define BLINK_GPIO 2

/**
 * @brief Bit modifying is processing in interrupt function so setting bit is neeeded for changing bit modification department
 */
#define BIT_PRESS_SHORT  (1<<0)
#define BIT_PRESS_NORMAL (1<<1)
#define BIT_PRESS_LONG   (1<<2)


static EventGroupHandle_t xCreatedEventGroup;//Event Group creating

void input_event_callback(int pin, uint64_t tick)  
{  
    if(pin == GPIO_NUM_0)  
    {  
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        int press_ms = tick*portTICK_PERIOD_MS; //1 tick == 10ms
        if(press_ms < 1000)
        {
            //press short
            xEventGroupSetBitsFromISR(xCreatedEventGroup, BIT_PRESS_SHORT, &xHigherPriorityTaskWoken);
        }
        else if(press_ms < 3000)
        {
            //press normal
            xEventGroupSetBitsFromISR(xCreatedEventGroup, BIT_PRESS_NORMAL, &xHigherPriorityTaskWoken);
        }
        else if(press_ms > 3000)
        {
            //press long
            //xEventGroupSetBitsFromISR(xCreatedEventGroup, BIT_PRESS_LONG, &xHigherPriorityTaskWoken);
        }
    }  
}

void button_timeout_callback(int pin){
    if(pin == GPIO_NUM_0){
        printf("TIMEOUT\n");
    }
}

void vTaskCode( void * pvParameters )
{
    for( ;; )
    {   
         EventBits_t uxBits = xEventGroupWaitBits(
            xCreatedEventGroup,   /* The event group being tested. */
            BIT_PRESS_SHORT | BIT_PRESS_NORMAL | BIT_PRESS_LONG, /* The bits within the event group to wait for. */
            pdTRUE,        /* BIT_0 & BIT_4 should be cleared before returning. */
            pdFALSE,       /* Don't wait for both bits, either bit will do. */
            portMAX_DELAY );/* Wait a maximum of 100ms for either bit to be set. */

        if( uxBits & BIT_PRESS_SHORT)   
        {      
            printf("Press short\n");
        }
        else if( uxBits & BIT_PRESS_NORMAL)
        {   
            printf("Press Normal\n");
        }
        else if ( uxBits & BIT_PRESS_LONG )
        {
            printf("Press Long\n");
        }
    }
}

void app_main(void)  
{  
    xCreatedEventGroup = xEventGroupCreate();
    output_io_create(BLINK_GPIO);
    input_io_create(GPIO_NUM_0, ANY_EDLE);  
    input_set_callback(input_event_callback); 
    input_set_timeout_callback (button_timeout_callback);
    xTaskCreate(
                    vTaskCode,       /* Function that implements the task. */
                    "vTaskCode",          /* Text name for the task. */
                    1024*2,      /* Stack size in words, not bytes. */
                    NULL,    /* Parameter passed into the task. */
                    4,/* Priority at which the task is created. */
                    NULL);      /* Used to pass out the created task's handle. */

}


// Khi gap loi implicit declaration tuc la chua khai bao ham do o trong file.h