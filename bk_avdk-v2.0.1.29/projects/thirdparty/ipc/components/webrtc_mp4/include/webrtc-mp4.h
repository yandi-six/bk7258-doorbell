#ifndef WEBRTC_MP4_H_89547832574143214378743dDHHGHG90
#define WEBRTC_MP4_H_89547832574143214378743dDHHGHG90
#include <stddef.h>
#include <stdint.h>
// ISO/IEC 14496-1:2010(E) 7.2.6.6 DecoderConfigDescriptor (p48)
// MPEG-4 systems ObjectTypeIndication
// http://www.mp4ra.org/object.html
#define WEBRTC_MP4_OBJECT_TEXT		0x08 // Text Stream
#define WEBRTC_MP4_OBJECT_MP4V		0x20 // Visual ISO/IEC 14496-2 (c)
#define WEBRTC_MP4_OBJECT_H264		0x21 // Visual ITU-T Recommendation H.264 | ISO/IEC 14496-10
#define WEBRTC_MP4_OBJECT_H265		0x23 // Visual ISO/IEC 23008-2 | ITU-T Recommendation H.265
#define WEBRTC_MP4_OBJECT_AAC		0x40 // Audio ISO/IEC 14496-3
#define WEBRTC_MP4_OBJECT_MP2V		0x60 // Visual ISO/IEC 13818-2 Simple Profile
#define WEBRTC_MP4_OBJECT_AAC_MAIN	0x66 // MPEG-2 AAC Main
#define WEBRTC_MP4_OBJECT_AAC_LOW	0x67 // MPEG-2 AAC Low
#define WEBRTC_MP4_OBJECT_AAC_SSR	0x68 // MPEG-2 AAC SSR
#define WEBRTC_MP4_OBJECT_MP3		0x69 // Audio ISO/IEC 13818-3
#define WEBRTC_MP4_OBJECT_MP1V		0x6A // Visual ISO/IEC 11172-2
#define WEBRTC_MP4_OBJECT_MP1A		0x6B // Audio ISO/IEC 11172-3
#define WEBRTC_MP4_OBJECT_JPEG		0x6C // Visual ISO/IEC 10918-1 (JPEG)
#define WEBRTC_MP4_OBJECT_PNG		0x6D // Portable Network Graphics (f)
#define WEBRTC_MP4_OBJECT_JPEG2000	0x6E // Visual ISO/IEC 15444-1 (JPEG 2000)
#define WEBRTC_MP4_OBJECT_VC1      0xA3 // SMPTE VC-1 Video
#define WEBRTC_MP4_OBJECT_DIRAC    0xA4 // Dirac Video Coder
#define WEBRTC_MP4_OBJECT_AC3      0xA5 // AC-3
#define WEBRTC_MP4_OBJECT_EAC3     0xA6 // Enhanced AC-3
#define WEBRTC_MP4_OBJECT_G719		0xA8 // ITU G.719 Audio
#define WEBRTC_MP4_OBJECT_DTS      0xA9 // Core Substream
#define WEBRTC_MP4_OBJECT_OPUS		0xAD // Opus audio https://opus-codec.org/docs/opus_in_isobmff.html
#define WEBRTC_MP4_OBJECT_VP9      0xB1 // VP9 Video
#define WEBRTC_MP4_OBJECT_FLAC     0xC1 // nonstandard from FFMPEG
#define WEBRTC_MP4_OBJECT_VP8      0xC2 // nonstandard
#define WEBRTC_MP4_OBJECT_H266		0xFC // ITU-T Recommendation H.266
#define WEBRTC_MP4_OBJECT_G711a	0xFD // ITU G.711 alaw
#define WEBRTC_MP4_OBJECT_G711u	0xFE // ITU G.711 ulaw
#define WEBRTC_MP4_OBJECT_AV1		0xFF // AV1: https://aomediacodec.github.io/av1-isobmff

#define WEBRTC_MP4_OBJECT_NONE		0x00 // unknown object id
#define WEBRTC_MP4_OBJECT_AVC		WEBRTC_MP4_OBJECT_H264
#define WEBRTC_MP4_OBJECT_HEVC		WEBRTC_MP4_OBJECT_H265
#define WEBRTC_MP4_OBJECT_VVC		WEBRTC_MP4_OBJECT_H266
#define WEBRTC_MP4_OBJECT_ALAW		WEBRTC_MP4_OBJECT_G711a
#define WEBRTC_MP4_OBJECT_ULAW		WEBRTC_MP4_OBJECT_G711u


/// MOV av stream flag
#define WEBRTC_MP4_AV_FLAG_KEYFREAME		0x0001
#define WEBRTC_MP4_AV_FLAG_SEGMENT_FORCE	0x8000 // exclude with WEBRTC_MP4_AV_FLAG_SEGMENT_DISABLE, fmp4_writer only
#define WEBRTC_MP4_AV_FLAG_SEGMENT_DISABLE	0x4000 // exclude with WEBRTC_MP4_AV_FLAG_SEGMENT_FORCE, fmp4_writer only

typedef struct webrtc_reader_mp4_param_t{
       int flags;
       int64_t pts;
       int64_t dts;
       uint32_t track;
       uint8_t object;
       uint8_t* ptr;
       int bytes;

}webrtc_reader_mp4_param_t;


typedef struct webrtc_reader_mp4_video_info_t{
	int isvideo;
        uint32_t track;
        uint8_t object;
        uint32_t frames;
        uint64_t duration;
        int width;
        int height;
}webrtc_reader_mp4_video_info_t;

typedef struct webrtc_reader_mp4_audio_info_t{
	int isaudio;
	uint32_t track;
        uint8_t object;
        int channel_count;
        int bit_per_sample;
        int sample_rate;
}webrtc_reader_mp4_audio_info_t;



typedef struct webrtc_mp4_reader_t webrtc_mp4_reader;
typedef struct webrtc_mp4_writer_t webrtc_mp4_writer;
typedef struct webrtc_mpeg_ts_writer_t webrtc_mpegts_writer;

webrtc_mp4_reader* webrtc_mp4_reader_create(char *file);
void webrtc_mp4_reader_destroy(webrtc_mp4_reader* mp4);

uint64_t webrtc_mp4_reader_getduration(webrtc_mp4_reader* mp4);

int webrtc_mp4_reader_getvideoinfo(webrtc_mp4_reader* mp4,webrtc_reader_mp4_video_info_t*info);
int webrtc_mp4_reader_getaudioinfo(webrtc_mp4_reader* mp4,webrtc_reader_mp4_audio_info_t*info);

/// same as mov_reader_read + user alloc buffer
/// NOTICE: user should free buffer on return error!!!
/// @return 1-read one frame, 0-EOF, <0-error 
int webrtc_mp4_reader_read(webrtc_mp4_reader* mp4, webrtc_reader_mp4_param_t* param);

/// @param[in,out] timestamp input seek timestamp, output seek location timestamp
/// @return 0-ok, other-error
int webrtc_mp4_reader_seek(webrtc_mp4_reader* mp4, int64_t* timestamp);



webrtc_mp4_writer* webrtc_mp4_writer_create(char *file,int width, int height,int video_fps,int channel_count, int bit_per_sample, int sample_rate);
int webrtc_mp4_writer_destroy(webrtc_mp4_writer* mp4);
int webrtc_mp4_writer_writeframe(webrtc_mp4_writer* mp4, int objedt,const uint8_t* data,  int bytes);
int webrtc_mp4_writer_set_audio_info(webrtc_mp4_writer* mp4, int objedt,int channel_count, int bit_per_sample, int sample_rate,int nb_samples);

webrtc_mpegts_writer* webrtc_mpegts_writer_create(char *file,int width, int height,int video_fps,int channel_count, int bit_per_sample, int sample_rate);
int webrtc_mpegts_writer_destroy(webrtc_mpegts_writer* mpegts);
int webrtc_mpegts_writer_write_frame(webrtc_mpegts_writer* mpegts, int objedt,const uint8_t* data, int bytes);
int webrtc_mpegts_writer_set_audio_info(webrtc_mpegts_writer* mpegts, int objedt,int channel_count, int bit_per_sample, int sample_rate,int nb_samples);







#endif
