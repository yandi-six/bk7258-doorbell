
#ifndef _WEBRTC_MJPEG_H
#define _WEBRTC_MJPEG_H

#ifdef __cplusplus
extern "C"
{
#endif
int Convert_yuv420sp_scale(unsigned char *src, int inWidth,int inHeight,unsigned char *dst,int outWidth,int outHeight);
void yuyv_scale(const uint8_t* src, uint8_t* dst, int src_width, int src_height, int dst_width, int dst_height);
uint32_t yuv420sp_to_jpeg( unsigned char* yuvData,unsigned char *Jpeg,int image_width, int image_height, int quality);
uint32_t uyvy_to_jpg(int width, int height, unsigned char *inputYuv,unsigned char *Jpeg);
uint32_t yuv422_to_jpg(int width, int height, unsigned char *inputYuv,unsigned char *Jpeg);
int yuyv_to_yuv420p(const unsigned char *in, unsigned char *out, unsigned int width, unsigned int height);
int vyuy_to_yuv420p(const unsigned char *in, unsigned char *out, unsigned int width, unsigned int height);
#ifdef __cplusplus
}
#endif

#endif
