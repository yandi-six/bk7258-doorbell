

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
#include "webrtc_jpeg.h"
#if (CONFIG_WEBRTC_JPEG)
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"
#include "rtc_bk.h"
#define TAG "jpg"
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)

int yuv420sp_to_jpg(char *filename, int width, int height, unsigned char *pYUVBuffer)
{
    FILE *fJpg;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int row_stride;
    int i = 0, j = 0;
    unsigned char yuvbuf[width * 3];
    unsigned char *pY, *pU, *pV;
    int ulen;
    unsigned char *outJpeg = NULL;
    unsigned long outSize = 0;
    ulen = width * height / 4;

    if(pYUVBuffer == NULL){
        LOGW("pBGRBuffer is NULL!\n");
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, &outJpeg, &outSize);

    
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr;
    cinfo.dct_method = JDCT_ISLOW;
    jpeg_set_defaults(&cinfo);


    jpeg_set_quality(&cinfo, 99, TRUE);

    jpeg_start_compress(&cinfo, TRUE);
    row_stride = cinfo.image_width * 3; /* JSAMPLEs per row in image_buffer */
    
    pY = pYUVBuffer;
    pU = pYUVBuffer + width*height;
    pV = pYUVBuffer + width*height + ulen;
    j = 1;
    while (cinfo.next_scanline < cinfo.image_height) {
        /* jpeg_write_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could pass
         * more than one scanline at a time if that's more convenient.
         */

        /*Test yuv buffer serial is : yyyy...uu..vv*/
        if(j % 2 == 1 && j > 1){
            pU = pYUVBuffer + width*height + width / 2 * (j / 2);
            pV = pYUVBuffer + width*height * 5 / 4 + width / 2 *(j / 2);
        }
        for(i = 0; i < width; i += 2){
            yuvbuf[i*3] = *pY++;
            yuvbuf[i*3 + 1] = *pU;
            yuvbuf[i*3 + 2] = *pV;

            yuvbuf[i*3 + 3] = *pY++;
            yuvbuf[i*3 + 4] = *pU++;
            yuvbuf[i*3 + 5] = *pV++;
        }

        row_pointer[0] = yuvbuf;
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
        j++;
    }

    jpeg_finish_compress(&cinfo);

    jpeg_destroy_compress(&cinfo);
   

    return 0;
}
uint32_t yuv422_to_jpg(int width, int height, unsigned char *inputYuv,unsigned char *Jpeg)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	int i = 0;
	unsigned char *pY, *pU, *pV;
	unsigned long outSize = 0;
	unsigned char *yuvbuf =rtc_bk_malloc(width * 3); 
	if(yuvbuf){
	    	unsigned char *outJpeg = NULL;
		cinfo.err = jpeg_std_error(&jerr);//用于错误信息
		jpeg_create_compress(&cinfo);  //初始化压缩对象
		jpeg_mem_dest(&cinfo, &outJpeg, &outSize);
		cinfo.image_width = width;//设置输入图片宽度
		cinfo.image_height = height;//设置图片高度
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_YCbCr;//设置输入图片的格式，支持RGB/YUV/YCC等等
		//cinfo.dct_method = JDCT_FLOAT;
		jpeg_set_defaults(&cinfo);//其它参数设置为默认的！
		jpeg_set_quality(&cinfo, 75, TRUE);//设置转化图片质量，范围0-100
		jpeg_start_compress(&cinfo, TRUE);
		pY = inputYuv ;
		pU = inputYuv +1 ;
		pV = inputYuv + 3;
		while (cinfo.next_scanline < cinfo.image_height) {
			int index = 0;
			for (i = 0; i < width; i += 2){//输入的YUV图片格式为标准的YUV444格式，所以需要把YUV420转化成YUV444.
				yuvbuf[index++] = *pY;
				yuvbuf[index++] = *pU;
				yuvbuf[index++] = *pV;
				pY += 2;
				yuvbuf[index++] = *pY;
				yuvbuf[index++] = *pU;
				yuvbuf[index++] = *pV;
				pY += 2;
				pU += 4;
				pV += 4;
			}
			row_pointer[0] = yuvbuf;
			(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);//单行图片转换压缩
		}
		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);
		if(outJpeg!=NULL){
			rtc_bk_memcpy(Jpeg,outJpeg,(int)outSize);
			rtc_bk_free(outJpeg);
			outJpeg = NULL;
		}
		rtc_bk_free(yuvbuf);
	}
	return (uint32_t)outSize;
}

uint32_t yuv420sp_to_jpeg( unsigned char* yuvData,unsigned char *Jpeg,int image_width, int image_height, int quality)
{
        unsigned long outSize = 0;
        unsigned char *outJpeg = NULL;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
       
        jpeg_mem_dest(&cinfo, &outJpeg, &outSize);
 
	cinfo.image_width = image_width;    
	cinfo.image_height = image_height;
	cinfo.input_components = 3;    // # of color components per pixel  
	cinfo.in_color_space = JCS_YCbCr;  //colorspace of input image  
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);
 
	//  
	//  cinfo.raw_data_in = TRUE;  
	cinfo.jpeg_color_space = JCS_YCbCr;
	cinfo.comp_info[0].h_samp_factor = 2;
	cinfo.comp_info[0].v_samp_factor = 2;
	  
 
	jpeg_start_compress(&cinfo, TRUE);
 
	JSAMPROW row_pointer[1];
 
	// 获取y、u、v三个分量各自数据的指针地址
	unsigned char *ybase, *ubase, *vbase;
	ybase = yuvData;
	ubase = yuvData + image_width*image_height;
	vbase = ubase + image_height*image_width / 4;
 
	unsigned char *yuvLine = (unsigned char *)rtc_bk_malloc(image_width * 3); 
	rtc_bk_memset(yuvLine, 0, image_width * 3);
        if(yuvLine){
		int j = 0;
		while (cinfo.next_scanline < cinfo.image_height)
		{
			int idx = 0;
			for (int i = 0; i<image_width; i++)
			{
				// 分别取y、u、v的数据
				yuvLine[idx++] = ybase[i + j * image_width];
				yuvLine[idx++] = ubase[(j>>1) * image_width/2 + (i>>1) ];
				yuvLine[idx++] = vbase[(j>>1) * image_width/2 + (i>>1) ];
			}
			row_pointer[0] = yuvLine;
			jpeg_write_scanlines(&cinfo, row_pointer, 1);
			j++;
		}
		jpeg_finish_compress(&cinfo);
		rtc_bk_free(yuvLine);
		yuvLine = NULL;
	}
	jpeg_destroy_compress(&cinfo);
	if(outJpeg!=NULL){
			rtc_bk_memcpy(Jpeg,outJpeg,(int)outSize);
			rtc_bk_free(outJpeg);
			outJpeg = NULL;
	}
	return (uint32_t)outSize;
}

void yuyv_scale(const uint8_t* src, uint8_t* dst, int src_width, int src_height, int dst_width, int dst_height) {
            int src_stride = src_width * 2;
            int dst_stride = dst_width * 2;
            float scale_x = (float)src_width / dst_width;
            float scale_y = (float)src_height / dst_height;
        
            for (int j = 0; j < dst_height; ++j) {
                float src_y = (j + 0.5) * scale_y - 0.5;
                int src_row = (int)src_y;
                float y_frac = src_y - src_row;
        
                for (int i = 0; i < dst_width; ++i) {
                    float src_x = (i + 0.5) * scale_x - 0.5;
                    int src_col = (int)src_x;
                    float x_frac = src_x - src_col;
        
                    int src_index = (src_row * src_stride) + (src_col * 2);
                    int dst_index = (j * dst_stride) + (i * 2);
        
                    // Bilinear interpolation
                    dst[dst_index]     = (uint8_t)((1 - x_frac) * ((1 - y_frac) * src[src_index] + y_frac * src[src_index + 2]) + x_frac * ((1 - y_frac) * src[src_index + 4] + y_frac * src[src_index + 6]));
                    dst[dst_index + 1] = (uint8_t)((1 - x_frac) * ((1 - y_frac) * src[src_index + 1] + y_frac * src[src_index + 3]) + x_frac * ((1 - y_frac) * src[src_index + 5] + y_frac * src[src_index + 7]));
                }
            }
}
#define scale_clip(a, min, max) (a > max ? max : (a > min ? a : min))
int yuv420sp_scale(unsigned char *pYIn, int lineY, 
	       		unsigned char *pCbIn, int lineCb, 
    	   		unsigned char *pCrIn, int lineCr, 
		   	int srcWidth, int srcHeight, 
		   	int outWidth, int outHeight,
	       		unsigned char *pYOut, 
    	   		unsigned char *pCbOut, 
		        unsigned char *pCrOut){
	    float rx = (float)outWidth/(float)srcWidth;
	    float ry = (float)outHeight/(float)srcHeight;
	    int x = 0, y = 0;
            int xx = 0, yy = 0;
	    int outWidth_cr = outWidth / 2;
	    int outHeight_cr = outHeight / 2;
	    int srcWidth_cr = srcWidth / 2;
	    int srcHeight_cr = srcHeight / 2;

            for(y=0; y<outHeight; y++)
            {
                for(x=0; x<outWidth; x++)
                {
                    //g(x, y) = f(x/r, y/r) = f(x*rx_den/rx_num, y*ry_den/ry_num)
                    xx = scale_clip((int)((float)x / rx), 0, srcWidth-1);
                    yy = scale_clip((int)((float)y / ry), 0, srcHeight-1);
                    pYOut[y*outWidth+x] = pYIn[yy*lineY + xx];
                }
            }
            for(y=0; y<outHeight_cr; y++)
            {
                for(x=0; x<outWidth_cr; x++)
                {
                    //g(x, y) = f(x/r, y/r) = f(x*rx_den/rx_num, y*ry_den/ry_num)
                    xx = scale_clip((int)((float)x / rx), 0, srcWidth_cr-1);
                    yy = scale_clip((int)((float)y / ry), 0, srcHeight_cr-1);
                    pCbOut[y*outWidth_cr+x] = pCbIn[yy*lineCb + xx];
                }
            }
            for(y=0; y<outHeight_cr; y++)
            {
                for(x=0; x<outWidth_cr; x++)
                {
                    //g(x, y) = f(x/r, y/r) = f(x*rx_den/rx_num, y*ry_den/ry_num)
                    xx = scale_clip((int)((float)x / rx), 0, srcWidth_cr-1);
                    yy = scale_clip((int)((float)y / ry), 0, srcHeight_cr-1);
                    pCrOut[y*outWidth_cr+x] = pCrIn[yy*lineCr + xx];
                }
            }
	  return 1;

}
int Convert_yuv420sp_scale(unsigned char *src, int inWidth,int inHeight,unsigned char *dst,int outWidth,int outHeight){
	return yuv420sp_scale( src, inWidth, src + inWidth * inHeight, inWidth/2, src + inWidth * inHeight * 5 / 4,
		inWidth/2, inWidth, inHeight, outWidth, outHeight, dst, dst+outWidth*outHeight, dst+outWidth*outHeight*5/4);
}
void yuv422_to_yuv420sp(const unsigned char* yuv422, unsigned char* yuv420sp, int width, int height) {
    int i, j;
    int yIdx, uvIdx;
    int yIdxNext, uvIdxNext;
    int u, v;
 
    for (j = 0, yIdx = 0, uvIdx = 0; j < height; j += 2, yIdxNext = yIdx + width, uvIdxNext = uvIdx + width / 2) {
        for (i = 0; i < width; i += 2, yIdx++, uvIdx++) {
            // Y
            yuv420sp[yIdx] = yuv422[yIdx];
 
            // U
            if (i == 0 && j == 0) {
                u = uvIdx;
                v = uvIdx + 1;
            } else if (i == 0) {
                u = uvIdx;
                v = uvIdx + 1;
            } else if (j == 0) {
                u = uvIdx;
                v = uvIdx + 1;
            } else {
                u = uvIdx / 2;
                v = uvIdx / 2 + 1;
            }
            yuv420sp[uvIdx] = (yuv422[u] + yuv422[u + width]) / 2;
            yuv420sp[uvIdx + width / 2] = (yuv422[v] + yuv422[v + width]) / 2;
 
            // V
            yuv420sp[uvIdx + width / 2 + 1] = yuv420sp[uvIdx + width / 2];
        }
        yuv422 += width * 2;
        yuv420sp += width;
    }
}
int yuyv_to_yuv420p(const unsigned char *in, unsigned char *out, unsigned int width, unsigned int height)
{
    unsigned char *y = out;
    unsigned char *u = out + width*height;
    unsigned char *v = out + width*height + width*height/4;
 
    unsigned int i,j;
    unsigned int base_h;
    unsigned int is_y = 1, is_u = 1;
    unsigned int y_index = 0, u_index = 0, v_index = 0;
 
    unsigned long yuv422_length = 2 * width * height;
 
    //序列为YU YV YU YV，一个yuv422帧的长度 width * height * 2 个字节
    //丢弃偶数行 u v
    for(i=0; i<yuv422_length; i+=2){
        *(y+y_index) = *(in+i);
        y_index++;
    }
    for(i=0; i<height; i+=2){
        base_h = i*width*2;
        for(j=base_h+1; j<base_h+width*2; j+=2){
            if(is_u){
                *(u+u_index) = *(in+j);
                u_index++;
                is_u = 0;
            }else{
                *(v+v_index) = *(in+j);
                v_index++;
                is_u = 1;
            }
        }
    }
    return 1;
}
int vyuy_to_yuv420p(const unsigned char *in, unsigned char *out, unsigned int width, unsigned int height)
{
    unsigned char *y = out;
    unsigned char *u = out + width*height;
    unsigned char *v = out + width*height + width*height/4;
 
    unsigned int i,j;
    unsigned int base_h;
    unsigned int is_u = 0;
    unsigned int y_index = 0, u_index = 0, v_index = 0;
 
    unsigned long yuv422_length = 2 * width * height;
 
    //序列为UY VY UY VY，一个yuv422帧的长度 width * height * 2 个字节
    for(i=0; i<yuv422_length; i+=2){
        *(y+y_index) = *(in+i+1);
        y_index++;
    }
    for(i=0; i<height; i+=2){
        base_h = i*width*2;
        for(j=base_h; j<base_h+width*2; j+=2){
            if(is_u){
                *(u+u_index) = *(in+j);
                u_index++;
                is_u = 0;
            }else{
                *(v+v_index) = *(in+j);
                v_index++;
                is_u = 1;
            }
        }
    }
    return 1;
}
uint32_t uyvy_to_jpg(int width, int height, unsigned char *inputYuv,unsigned char *Jpeg) {
        struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	int i = 0;
	unsigned char *pY, *pU, *pV;
	unsigned long outSize = 0;
	unsigned char *yuvbuf =rtc_bk_malloc(width * 3); 
	if(yuvbuf){
	    	unsigned char *outJpeg = NULL;
		cinfo.err = jpeg_std_error(&jerr);//用于错误信息
		jpeg_create_compress(&cinfo);  //初始化压缩对象
		jpeg_mem_dest(&cinfo, &outJpeg, &outSize);
		cinfo.image_width = width;//设置输入图片宽度
		cinfo.image_height = height;//设置图片高度
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_YCbCr;//设置输入图片的格式，支持RGB/YUV/YCC等等
		//cinfo.dct_method = JDCT_FLOAT;
		jpeg_set_defaults(&cinfo);//其它参数设置为默认的！
		jpeg_set_quality(&cinfo, 75, TRUE);//设置转化图片质量，范围0-100
		jpeg_start_compress(&cinfo, TRUE);
		pY = inputYuv +1;
		pU = inputYuv ;
		pV = inputYuv + 2;
		while (cinfo.next_scanline < cinfo.image_height) {
			int index = 0;
			for (i = 0; i < width; i += 2){//输入的YUV图片格式为标准的YUV444格式，所以需要把UYVY转化成YUV444.
				yuvbuf[index++] = *pY;
				yuvbuf[index++] = *pU;
				yuvbuf[index++] = *pV;
				pY += 2;
				yuvbuf[index++] = *pY;
				yuvbuf[index++] = *pU;
				yuvbuf[index++] = *pV;
				pY += 2;
				pU += 4;
				pV += 4;
			}
			row_pointer[0] = yuvbuf;
			(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);//单行图片转换压缩
		}
		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);
		if(outJpeg!=NULL){
			memcpy(Jpeg,outJpeg,(int)outSize);
			free(outJpeg);
			outJpeg = NULL;
		}
		rtc_bk_free(yuvbuf);
	}
	return (uint32_t)outSize;

}
 
void overlay_osd(char *yuv422_image, int width, int height, char *osd_image, int osd_width, int osd_height) {
    int x, y;
    int osd_x, osd_y;
    char *y_ptr, *u_ptr, *v_ptr;
    char *osd_y_ptr, *osd_u_ptr, *osd_v_ptr;
 
    // 计算OSD在YUV422图像中的位置
    osd_x = (width - osd_width) / 2;
    osd_y = (height - osd_height) / 2;
 
    // 遍历YUV422图像的像素
    for (y = osd_y, y_ptr = yuv422_image + osd_y * width, osd_y_ptr = osd_image; y < osd_y + osd_height; y++, y_ptr += width, osd_y_ptr += osd_width) {
        for (x = osd_x, u_ptr = y_ptr + (width / 2), v_ptr = y_ptr + (width / 2) + 1, osd_u_ptr = osd_y_ptr + (osd_width / 2), osd_v_ptr = osd_y_ptr + (osd_width / 2) + 1; x < osd_x + osd_width; x++, u_ptr += (width / 2), v_ptr += (width / 2) + 1, osd_u_ptr += (osd_width / 2), osd_v_ptr += (osd_width / 2) + 1) {
            // 只需要叠加Y分量，UV分量保持不变
            *y_ptr = *osd_y_ptr;
 
            // 在YUV422格式中UV是交错存放的，所以需要跳过一个Y来处理一个UV对
            if ((x - osd_x) % 2 == 0) {
                *u_ptr = *osd_u_ptr;
                *v_ptr = *osd_v_ptr;
            }
 
            y_ptr++;
            osd_y_ptr++;
        }
    }
}
#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
// 假设的YUV422图像数据和其尺寸
unsigned char yuv422_data[] = { /* ... */ };
int image_width = 640;
int image_height = 480;
 
// 将字符写入YUV422图像的OSD
void write_osd_char(int x, int y, char c) {
    // 计算字符的起始点在YUV数据中的位置
    int char_start = (y * image_width + x) * 2; // YUV422中每个像素占2个字节
 
    // 遍历字符的每一行
    for (int row = 0; row < 16; row++) { // 假设字符的高度为16像素
        if (c < 32 || c > 127) break; // 非打印字符跳过
        unsigned char *line_data = &yuv422_data[char_start + (row * image_width * 2)];
 
        // 遍历字符的每一像素
        for (int col = 0; col < 8; col++) { // 假设字符的宽度为8像素
            if (7 - col >= 8 - (c >> 3)) { // 判断当前位是否需要显示
                line_data[col * 2] = 255; // 设置Y值为最大，表示白色
                // UV通常保持不变，这里为了简单起见，也设置为白色
                line_data[col * 2 + 1] = 255; 
                line_data[col * 2 + 2] = 255;
            }
            c <<= 1;
        }
    }
}
 
int main() {
    char text[] = "Hello, OSD!";
    int text_len = strlen(text);
 
    // 将字符串中的字符写入图像
    for (int i = 0; i < text_len; i++) {
        write_osd_char(100 + i * 8, 100, text[i]);
    }
 
    // 此时yuv422_data包含了OSD字符
    // 保存或处理yuv422_data...
 
    return 0;
}
#endif
#else
uint32_t yuv420sp_to_jpg(int width, int height, unsigned char *inputYuv,unsigned char *outJpeg){
   return 0;
}
uint32_t yuv422_to_jpg(int width, int height, unsigned char *inputYuv,unsigned char *outJpeg){
   return 0;
}
void scaleYUYV(const uint8_t* src, uint8_t* dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight){
}
#endif 
