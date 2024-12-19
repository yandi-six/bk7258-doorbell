#ifndef _STEP_MOTOOR_H
#define _STEP_MOTOOR_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _stepmotor_msg{
    bool runing;
    bool pause;
    bool one_way;
    int direct;     //
    int channel;
    int angel;
    int direction_x;
    int direction_y;
    uint32_t start_timestamp;
    beken_semaphore_t event_sem;
}stepmotor_msg;

void stepmotor_init(stepmotor_msg *stepmsg);
void stepmotor_uninit(stepmotor_msg *stepmsg);
void stepmotor_ptz_control(stepmotor_msg *stepmsg,int direction_x,int direction_y);


#ifdef __cplusplus
}
#endif

#endif

