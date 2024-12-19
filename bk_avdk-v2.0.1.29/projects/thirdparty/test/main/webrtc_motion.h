#ifndef _WEBRTC_MOTION_H
#define _WEBRTC_MOTION_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct FrameBuffer_ {
    uint8_t *frame;
    int size;
    int width;
    int height;
}FrameBuffer;
typedef struct {
    int x;
    int y;
    int width;
    int height;
} Rectangle;

void webrtc_motion_init();
void webrtc_motion_uninit();
void webrtc_motion_reset();
void webrtc_motion_input_frame(uint8_t *frame,int size,int width,int height,uint16_t sequence,int framedelay);
int  webrtc_motion_detection();
Rectangle * webrtc_motion_rectangles();
bool isInRectangles(int x, int y, Rectangle* rectangles, int count);


#ifdef __cplusplus
}
#endif

#endif
