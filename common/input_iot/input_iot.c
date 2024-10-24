#include <stdio.h>  
#include "esp_log.h"
#include <driver/gpio.h>  
#include "input_iot.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/portmacro.h"

input_callback_t input_callback = NULL;
static uint64_t _start, _stop, _pressTick;
/**
 * @brief Khi nao bat dau an se tao mot timer. 
 * @example Gia su Time out la 5s. Sau 5s neu van con giu thi timer se dat nguong va bao su kien de xu ly ngay sau do. 
 * @details Timer duoc tao ra de bao hieu cho nguoi dung su kien da xay ra . 
            Vi du nhu khi nhan giu nut qua mot khoang thoi gian nhat dinh thi den sang
*/
timeoutButton_t timeoutButton_callback = NULL;
static TimerHandle_t xTimers; //Timer se chi duoc su dung trong file nay

static void IRAM_ATTR  gpio_input_handler (void* arg) {
    int gpio_num = (uint32_t) arg;
    uint32_t rtc = xTaskGetTickCountFromISR(); //ms
 
    if(gpio_get_level(gpio_num)== 0 )
    {//bat dau bam
        _start = rtc;
        xTimerStart(xTimers,0);
    }
    else{ //tha tay ra
        xTimerStop(xTimers,0);
        _stop = rtc;
        _pressTick = _stop - _start;
        input_callback(gpio_num, _pressTick);
    }
}

static void vTimerCallback( TimerHandle_t xTimer )
 {

 uint32_t ID;

    /* Optionally do something if the pxTimer parameter is NULL. */
    configASSERT( xTimer );

    /* The number of times this timer has expired is saved as the
       timer's ID. Obtain the count. */
    ID = ( uint32_t ) pvTimerGetTimerID( xTimer );
    if(ID==0){
        timeoutButton_callback(BUTTON0);
    }
 }

void input_io_create(gpio_num_t gpio_num, interrupt_type_edge_t type) {
    esp_rom_gpio_pad_select_gpio(gpio_num);
    gpio_set_direction(gpio_num, GPIO_MODE_INPUT);
    gpio_set_pull_mode(gpio_num, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(gpio_num, type);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(gpio_num, gpio_input_handler, (void*) gpio_num);

     xTimers  = xTimerCreate
                   ( /* Just a text name, not used by the RTOS kernel. */
                     "TimerTimeout",
                     /* The timer period in ticks, must be greater than 0. */
                     5000/portTICK_PERIOD_MS, //Quy doi tu MS sang TICK
                     /* The timers will auto-reload themselves when they expire. */
                     pdFALSE,
                     /* The ID is used to store a count of the number of times the
                        timer has expired, which is initialised to 0. */
                     ( void * ) 0, //Neu set la 0 thi day la Timer dau tien
                     /* Each timer calls the same callback when it expires. */
                     vTimerCallback
                   );

}

int input_io_get_level(gpio_num_t gpio_num) {
    return gpio_get_level(gpio_num);
}

void input_set_callback(void* cb) {
    input_callback = cb;
}

void input_set_timeout_callback(void* cb) {
     timeoutButton_callback = cb;
}