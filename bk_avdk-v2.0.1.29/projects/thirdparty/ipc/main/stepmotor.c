#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
#include <components/shell_task.h>
#include <components/event.h>
#include <components/log.h>
#include <driver/gpio.h>
#include <driver/hal/hal_gpio_types.h>
#include "gpio_driver.h"
#include "stepmotor.h"
#include "rtc_bk.h"

#define TAG "StepMotor"
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)

static beken_thread_t  stepmotor_run_hal = NULL;
#define H1 52
#define H2 51
#define H3 26
#define H4 25

#define V1 24
#define V2 50
#define V3 49
#define V4 48

#define H_MAX    4096
#define V_MAX    1024

#define MAX_DELAY    500
#define BK_ABS(X) (X >= 0) ? X : -X
int sequence[][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};
uint32_t stepmotor_get_cur_timestamp(void){
	 return bk_get_milliseconds();
}
void stepmotor_pin_init_and_stop(void)
{
    gpio_dev_unmap(H1);
    bk_gpio_disable_input(H1);
    bk_gpio_enable_output(H1);
    bk_gpio_set_output_low(H1);
    bk_gpio_disable_pull(H1);

    gpio_dev_unmap(H2);
    bk_gpio_disable_input(H2);
    bk_gpio_enable_output(H2);
    bk_gpio_set_output_low(H2);
    bk_gpio_disable_pull(H2);


    gpio_dev_unmap(H3);
    bk_gpio_disable_input(H3);
    bk_gpio_enable_output(H3);
    bk_gpio_set_output_low(H3);
    bk_gpio_disable_pull(H3);


    gpio_dev_unmap(H4);
    bk_gpio_disable_input(H4);
    bk_gpio_enable_output(H4);
    bk_gpio_set_output_low(H4);
    bk_gpio_disable_pull(H4);


    gpio_dev_unmap(V1);
    bk_gpio_disable_input(V1);
    bk_gpio_enable_output(V1);
    bk_gpio_set_output_low(V1);
    bk_gpio_disable_pull(V1);


    gpio_dev_unmap(V2);
    bk_gpio_disable_input(V2);
    bk_gpio_enable_output(V2);
    bk_gpio_set_output_low(V2);
    bk_gpio_disable_pull(V2);

    gpio_dev_unmap(V3);
    bk_gpio_disable_input(V3);
    bk_gpio_enable_output(V3);
    bk_gpio_set_output_low(V3);
    bk_gpio_disable_pull(V3);
    
    gpio_dev_unmap(V4);
    bk_gpio_disable_input(V4);
    bk_gpio_enable_output(V4);
    bk_gpio_set_output_low(V4);
    bk_gpio_disable_pull(V4);
}


void gpio_set(int pins,int value)
{
   // LOGW("pins:%d value:%d\n",pins,value);
    if(value){ 
        bk_gpio_set_output_high(pins);
    }else{
        bk_gpio_set_output_low(pins);
        
    }
}

void step(int pins[], int direction) {

    static int j = 0;
    if(++j > 7)
     j = 0;
    if (direction == 1) { // foreard
        for (int pin = 0; pin < 4; pin++) {
            gpio_set(pins[pin], sequence[j][pin]);
        }
    } else { // back
        for (int pin = 3; pin >= 0; pin--) {
            gpio_set(pins[pin], sequence[j][3-pin]);
        }
    }
}


void stepmotor_run_main(void *arg)  //
{

    stepmotor_msg *stepmsg = (stepmotor_msg *)arg;
    stepmsg->runing = true;
    stepmsg->pause = true;
    int pins[4];
    int count = 0;
    int direct;
    uint32_t now;
    uint32_t delay;
    stepmotor_pin_init_and_stop();

    int abs_direction_x;
    int abs_direction_y;
    bool can_channel_0;
    bool can_channel_1;
    
    while(stepmsg->runing){
       if(stepmsg->pause){
		rtos_get_semaphore(&stepmsg->event_sem, BEKEN_WAIT_FOREVER);
		stepmsg->start_timestamp = stepmotor_get_cur_timestamp();
	}else{
	       now = stepmotor_get_cur_timestamp();
	       delay = now-stepmsg->start_timestamp;
	       if(delay <= MAX_DELAY){
			    if(stepmsg->one_way){	       
				    if(stepmsg->channel == 0){
					pins[0] = H1;pins[1] = H2;pins[2] = H3;pins[3] = H4;
				    }else{
					pins[0] = V1;pins[1] = V2;pins[2] = V3;pins[3] = V4;
				    }		       
				    step(pins,stepmsg->direct);
			    }else{
				 
				  can_channel_0 = false;
				  can_channel_1 = false;
				  abs_direction_x = BK_ABS(stepmsg->direction_x);
				  abs_direction_y = BK_ABS(stepmsg->direction_y);
				  if(abs_direction_x>0 && abs_direction_y >0){
					 
					if(abs_direction_x>abs_direction_y){
						can_channel_0 = true;
						if((abs_direction_y*MAX_DELAY*H_MAX)/(abs_direction_x*delay*V_MAX)>0){
							can_channel_1 = true;
						}

					}else{
						can_channel_1 = true;
						if((abs_direction_x*MAX_DELAY*V_MAX)/(abs_direction_y*delay*H_MAX)>0){
							can_channel_0 = true;
						}
					}

				  }else{
					  can_channel_0 = true;
					  can_channel_1 = true;
				  }
				   // channel 0
				  if(can_channel_0){
					  pins[0] = H1;pins[1] = H2;pins[2] = H3;pins[3] = H4;
					  if(stepmsg->direction_x>0){
						 step(pins,0);
					  }else if(stepmsg->direction_x<0){
						 step(pins,1);
					  }else{
						 
					  }
				  }
				  // channel 1
				  if(can_channel_1){
					  pins[0] = V1;pins[1] = V2;pins[2] = V3;pins[3] = V4;
					  if(stepmsg->direction_y>0){
						step(pins,0);
					  }else if(stepmsg->direction_y<0){
						step(pins,1); 
					  }else{
						 
					  }
				  }
				  
			   }
		            rtos_delay_milliseconds(5);
	       }else{
			//LOGW("%s %d   \n", __func__, __LINE__);
			stepmsg->pause = true;
	       }

       }
    }

    stepmsg->runing = false;
    stepmotor_pin_init_and_stop();
    stepmotor_run_hal = NULL;
    if(stepmsg->event_sem!= NULL){
		rtos_deinit_semaphore(&stepmsg->event_sem);
		stepmsg->event_sem = NULL;
     }
    LOGW("%s: stepmotor thread exit \n",__func__);
    rtos_delete_thread(NULL);
}
void stepmotor_ptz_control(stepmotor_msg *stepmsg,int direction_x,int direction_y){
	bool sendptz = true;
	stepmsg->direction_x = direction_x;
	stepmsg->direction_y = direction_y;
        LOGD("%s %d    direction_x = %d   direction_y = %d \n", __func__, __LINE__,direction_x,direction_y);
	if(direction_x==0 && direction_y==1){
		stepmsg->channel = 1;
		stepmsg->direct = 0;
		stepmsg->one_way = true;
	}else if(direction_x==0 && direction_y==-1){
		stepmsg->channel = 1;
		stepmsg->direct = 1;
		stepmsg->one_way = true;
	}else if(direction_x==1 && direction_y==0){
		stepmsg->channel = 0;
		stepmsg->direct = 0;
		stepmsg->one_way = true;
	}else if(direction_x==-1 && direction_y==0){
		stepmsg->channel = 0;
		stepmsg->direct = 1;
		stepmsg->one_way = true;
	}else{
		sendptz = true;
		stepmsg->one_way = false;	
	}
	if(sendptz){
		stepmsg->start_timestamp = stepmotor_get_cur_timestamp();
		if(stepmsg->pause== true){
				stepmsg->pause = false;
		}
		//LOGD("%s %d     channel = %d  direct = %d\n", __func__, __LINE__,stepmsg->channel,stepmsg->direct);
		if(stepmsg->event_sem!= NULL){
				int count = rtos_get_semaphore_count(&stepmsg->event_sem);
				if(count == 0){
					//LOGD("%s %d   \n", __func__, __LINE__);
					rtos_set_semaphore(&stepmsg->event_sem);
				}
		}
	}

}
void stepmotor_init(stepmotor_msg *stepmsg)
{
    bk_err_t ret = BK_OK;
    if(stepmsg->runing)
    {
        LOGW("---------%s: stepmotor is already start\n",__func__);
        return;
    }
    ret = rtos_init_semaphore_ex(&stepmsg->event_sem, 1, 0);
     if(ret!= BK_OK){
	LOGW("---------%s: stepmotor failed rtos_init_semaphore_ex\n",__func__);
	return;
     }
    
     ret = rtos_create_psram_thread(&stepmotor_run_hal,
						 5,
						 "stepmotor",
						 (beken_thread_function_t)stepmotor_run_main,
						 1024,
						 (beken_thread_arg_t)stepmsg);
    if(ret != BK_OK)
    {
        LOGE("err: stepmotor thread create\n");
    }
}
void stepmotor_uninit(stepmotor_msg *stepmsg)
{
    if(stepmsg->runing){
        stepmsg->runing = false;
	if(stepmsg->event_sem!= NULL){
		int count = rtos_get_semaphore_count(&stepmsg->event_sem);
		if(count == 0){
			rtos_set_semaphore(&stepmsg->event_sem);
		}
	}
        while(stepmotor_run_hal){
            rtos_delay_milliseconds(10);
        }
    }
}

