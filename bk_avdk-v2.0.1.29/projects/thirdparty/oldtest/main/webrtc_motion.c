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
#include "webrtc_motion.h"
#include "project_defs.h"
#include "rtc_bk.h"

#define TAG "runhua_motion"
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)

static    FrameBuffer *m_pframebuf[3] = {0};
static    FrameBuffer *m_pdiffframebuf = NULL;
static    int frameindex = 0;
static    int framecount = 0;
static    uint16_t framesequence = 0;
static    bool cal_diff = false;
static    bool motion_detection = false;
static    Rectangle* g_rectangles = NULL;
static    int max_rectangles = 1024;
static beken_mutex_t motion_detection_mutex = NULL;
void webrtc_motion_init(){
	rtos_init_mutex(&motion_detection_mutex);
        for(int i = 0;i<3;i++){
            m_pframebuf[i] = (FrameBuffer*)rtc_bk_malloc(sizeof(FrameBuffer));
            m_pframebuf[i]->frame = NULL;
            m_pframebuf[i]->size = 0;
        }
        m_pdiffframebuf = (FrameBuffer*)rtc_bk_malloc(sizeof(FrameBuffer));
        m_pdiffframebuf->frame = NULL;
        m_pdiffframebuf->size = 0;
	g_rectangles = (Rectangle*)rtc_bk_malloc(max_rectangles*sizeof(Rectangle));
}
void webrtc_motion_uninit(){
       for(int i = 0;i<3;i++){
           if(m_pframebuf[i]!= NULL){
           if(m_pframebuf[i]->frame!= NULL){
               rtc_bk_free(m_pframebuf[i]->frame);
           }
           rtc_bk_free(m_pframebuf[i]);
           }
       }
       if(m_pdiffframebuf!= NULL){
           if(m_pdiffframebuf->frame!= NULL){
               rtc_bk_free(m_pdiffframebuf->frame);
           }
           rtc_bk_free(m_pdiffframebuf);
	   m_pdiffframebuf = NULL;
       }
       if(g_rectangles!= NULL){
	   rtc_bk_free(g_rectangles);
	   g_rectangles = NULL;
       }
	if(motion_detection_mutex!= NULL){
		rtos_deinit_mutex(&motion_detection_mutex);
		motion_detection_mutex =NULL;
	}
	framecount = 0;
	frameindex = 0;
	cal_diff = false;
}
Rectangle * webrtc_motion_rectangles(){
	return g_rectangles;
}
void webrtc_motion_reset(){
    LOGW("webrtc_motion_reset \n");
	frameindex = 0;
	cal_diff = false;
}



//接收一帧图像数据并将其存储在一个缓冲区中，用于运动检测
void webrtc_motion_input_frame(uint8_t *frame,int size,int width,int height,uint16_t sequence,int framedelay){
	//LOGW("webrtc_motion_input_frame width = %d height = %d\n",width,height);
	framesequence = sequence;
/*
    //检查序列号连续性
	uint16_t frame_sequence = framesequence+framedelay;
	if(frame_sequence!= sequence ){
		webrtc_motion_reset();
		framesequence = sequence;
	}
*/
	if(motion_detection_mutex!= NULL){
		rtos_lock_mutex(&motion_detection_mutex);
		
		 if(m_pframebuf[frameindex]!= NULL){
		            if(m_pframebuf[frameindex]->frame == NULL || m_pframebuf[frameindex]->size!= size){
		               if(m_pframebuf[frameindex]->frame!= NULL){
		                   rtc_bk_free(m_pframebuf[frameindex]->frame);
		                   m_pframebuf[frameindex]->frame = NULL;
		               }

		               m_pframebuf[frameindex]->frame =(uint8_t*)rtc_bk_malloc(size);
			       if(m_pframebuf[frameindex]->frame!=NULL){
				       rtc_bk_memcpy(m_pframebuf[frameindex]->frame,frame,size);
				       m_pframebuf[frameindex]->width = width;
				       m_pframebuf[frameindex]->height = height;
				       m_pframebuf[frameindex]->size = size;
				       framecount++;
				       motion_detection = true;
				       //LOGW("webrtc_motion_input_frame %d %d\n",framecount,motion_detection);
			       }
		            }else{
				       rtc_bk_memcpy(m_pframebuf[frameindex]->frame,frame,size);
				       m_pframebuf[frameindex]->width = width;
				       m_pframebuf[frameindex]->height = height;
				       m_pframebuf[frameindex]->size = size;
				       framecount++;
				       motion_detection = true;
			    }

	       }else{
			LOGW("webrtc_motion_input_frame frameindex  is null \n");
	       }
	     rtos_unlock_mutex(&motion_detection_mutex);
      }else{
	LOGW("webrtc_motion_input_frame motion_detection_mutex  is null \n");
      }
}

//计算两幅图像之间的像素差
#define IMAGE_ABS(x) ( (x)>0?(x):-(x) )
static void image_difference(uint8_t *image1, uint8_t *image2, uint8_t *output, int width, int height) {
        for (int i = 0; i < width * height; i++) {
            output[i] = IMAGE_ABS(image1[i] - image2[i]);
        }
}

//将一幅灰度图像转换为二值图像（黑白图像）
static void binaryImage(uint8_t* image, int width, int height,uint8_t threshold) {
        uint8_t * source;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                source = image+(i*width+j);
                if (*source > threshold) {  // 像素值大于阈值则设为白色（255）
                    *source = 255;
                } else {  // 像素值小于或等于阈值则设为黑色（0）
                    *source = 0;
                }
            }
        }
}

//去除孤立的噪声点
static void medianFilter(uint8_t* image, int width, int height) {
        uint8_t * source;
        uint8_t * source_left;
        uint8_t * source_top;
        uint8_t * source_right;
        uint8_t * source_buttom;
        int count = 0;
        for (int i = 1; i < height-1; i++) {
            for (int j = 1; j < width-1; j++) {
                count = 0;
                source = image+(i*width+j);
                source_left = image+(i*width+j-1);
                source_right = image+(i*width+j+1);
                source_top = image+((i-1)*width+j);
                source_buttom = image+((i+1)*width+j);
                if(*source == 255){
                    if(*source_left == 0){
                        count++;
                    }
                    if(*source_right == 0){
                        count++;
                    }
                    if(*source_top == 0){
                        count++;
                    }
                    if(*source_buttom == 0){
                        count++;
                    }
                    if(count>2){
                        *source = 0;
                    }


                }else{

                    if(*source_left == 255){
                        count++;
                    }
                    if(*source_right == 255){
                        count++;
                    }
                    if(*source_top == 255){
                        count++;
                    }
                    if(*source_buttom == 255){
                        count++;
                    }
                    if(count>3){
                        *source = 255;
                    }
                }

            }
        }

    }

//检查一个给定的坐标点 (x, y) 是否位于一组矩形区域内    
bool isInRectangles(int x, int y, Rectangle* rectangles, int count) {
            int i = 0;
            for(i=0;i<count;i++){
                if(x>=rectangles->x && x<=(rectangles->x+rectangles->width) && y>=rectangles->y && y<=(rectangles->y+rectangles->height) ){
                    return true;
                }
                rectangles++;

            }
        return false;
}

//从一个二值图像中寻找白色的矩形区域，并将这些矩形的属性（位置和尺寸）存储到一个矩形数组
static void findRectangles(uint8_t* image, int width, int height, Rectangle* rectangles, int* count) {
        int i, j;
        int rectIndex = 0;

        for (i = 1; i < height-2; i++) {
            for (j = 1; j < width-2; j++) {
                if (image[i * width + j] == 255) { // 检查当前像素是否为白色（值为1）
                    if(isInRectangles(j,i,rectangles,rectIndex)){
                        continue;
                    }
                    int currentWidth = 0;
                    // 向右扩展直到不是白色或到达图像的右边缘
                    //LOGW("Found11111 %d %d\n",j,i);
                    while (j + currentWidth < width-2 && image[i * width + j + currentWidth] == 255) {
                        currentWidth++;
                    }
                    if(currentWidth>=width-2){
                        continue;
                    }
                     //LOGW("Found22222 %d %d\n",j,i);
                    int currentHeight = 1;

                    // 向下扩展直到不是白色或到达图像的底部
                    while (i + currentHeight < height-2 && image[(i + currentHeight) * width + j] == 255) {
/*
                        for (int k = j; k < currentWidth; k++) {
                            if (image[(i + currentHeight) * width  + k] != 255) {
                                currentHeight = 0;
                                break;
                            }
                        }
                        */

                        if (currentHeight > 0) {
                            currentHeight++;
                        }
                    }
                    if(isInRectangles(j+currentWidth,i+currentHeight,rectangles,rectIndex)){
                        continue;
                    }
                    if(isInRectangles((j+j+currentWidth)/2,(i+i+currentHeight)/2,rectangles,rectIndex)){
                        continue;
                    }
                    //LOGW("Found %d %d   %d  %d    %d\n", j,i,currentWidth,currentHeight,rectIndex);
                    if (currentWidth > 5 && currentHeight > 5) {
                        rectangles[rectIndex].x = j;
                        rectangles[rectIndex].y = i;
                        rectangles[rectIndex].width = currentWidth;
                        rectangles[rectIndex].height = currentHeight;
                        rectIndex++;
                         if(rectIndex>=max_rectangles){
                             *count =rectIndex;
                             return;
                         }
                    }
                    j += currentWidth; // 跳过已处理的像素
                }
            }
        }
        *count =rectIndex;
 }

 //检测视频流中是否有显著的运动
int  webrtc_motion_detection(){
	int count =0;
	if(motion_detection_mutex!= NULL){
		rtos_lock_mutex(&motion_detection_mutex);
		if(motion_detection){
		     if(cal_diff){
			
				uint8_t *dest=NULL;
		        	double y_sum = 0.0;
		        	uint8_t threshold = 128;
				int size;
				int width;
				int height;
				int i;
				int j;
				size = m_pframebuf[frameindex]->size;
				width = m_pframebuf[frameindex]->width;
				height = m_pframebuf[frameindex]->height;
		                if(m_pdiffframebuf->frame==NULL || m_pdiffframebuf->size!= size){
		                    if(m_pdiffframebuf->frame!= NULL){
		                        rtc_bk_free(m_pdiffframebuf->frame);
		                        m_pdiffframebuf->frame = NULL;
		                    }

		                    m_pdiffframebuf->frame =(uint8_t*)rtc_bk_malloc(size);
		                    m_pdiffframebuf->width = width;
		                    m_pdiffframebuf->height = height;
		                    m_pdiffframebuf->size = size;

		                }
		                if(m_pdiffframebuf->frame!= NULL){
		                   if(frameindex == 0){
		                        image_difference(m_pframebuf[0]->frame,m_pframebuf[2]->frame,m_pdiffframebuf->frame,width,height);
		                   }else if(frameindex == 1){
		                        image_difference(m_pframebuf[1]->frame,m_pframebuf[0]->frame,m_pdiffframebuf->frame,width,height);

		                   }else{
		                        image_difference(m_pframebuf[2]->frame,m_pframebuf[1]->frame,m_pdiffframebuf->frame,width,height);
		                   }
		                   dest = m_pdiffframebuf->frame;
		                   for( i = 0;i<height;i++){
		                       for( j = 0;j<width;j++){
		                           y_sum+=*dest;
		                           dest++;
		                       }
		                   }
		                   threshold = (uint8_t)(y_sum/size);
		                   if(threshold<10){
		                        threshold =10;
		                   }
		                   //LOGW("webrtc_motion_detection %lf --- %lf    %d\n",y_sum,y_sum/size,threshold);
		                   binaryImage(m_pdiffframebuf->frame,width,height,threshold);
		                   medianFilter(m_pdiffframebuf->frame,width,height);
		                   count = 0;
		                   findRectangles(m_pdiffframebuf->frame, width, height, g_rectangles, &count);                         
		                }

		     }
		     frameindex++;
		     if(frameindex>=3){
			    frameindex = 0;
			    cal_diff = true;
		     }
		     motion_detection = false;
		}
	   rtos_unlock_mutex(&motion_detection_mutex);
	}
	
	 return count;

}
