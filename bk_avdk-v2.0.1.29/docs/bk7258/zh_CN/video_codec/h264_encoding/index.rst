H264编码
=================================

:link_to_translation:`en:[English]`

1、功能概述
--------------------

	H264编码主要用于将YUV422的数据通过硬件编码，输出H264图像数据，并且压缩图像数据量

2、开发资料
--------------------
	当前avdk提供了详细的sdk接口，参考文件路径：``.\bk_idk\middleware\driver\h264``

	为了方便客户使用，重新封装了编码功能，当前只需要开关摄像头，然后针对图像质量，也提供一个api接口去调节，参考文件：``.\components\multimedia\app\media_app.c``


3、接口说明
-------------------------------

	基于h264图像质量的调试说明：`media_app_set_compression_ratio`:

	结构体参数说明：``compress_ratio_t``

	- yuv_mode_t mode:		当前需要调节的模式，仅支持JPEG和H264，此处应该选择H264
	- h264_qp_t  h264_qp：	h264编码QP的调整，仅支持H264
	- uint8_t    enable:	启动压缩率调节，1/0：启动/关闭
	- uint16_t   jpeg_up：	数据JPEG图像大小的上限值，单位byte，仅在JPEG模式下有效
	- uint16_t   jpeg_low:	数据JPEG图像大小的下限值，单位byte，仅在JPEG模式下有效
	- uint16_t   imb_bits：	H264编码输出I帧宏块的大小，单位byte，仅在H264模式下有效，值越大，压缩率越小，图像质量越高，编码输出图像越大，范围[1, 4095]
	- uint16_t   pmb_bits：	H264编码输出P帧宏块的大小，单位byte，仅在H264模式下有效，值越大，压缩率越小，图像质量越高，编码输出图像越大，范围[1, 4095]

	结构体参数说明：``h264_qp_t``

	- uint8_t init_qp;		H264编码时初始化QP，范围[0, 51];
	- uint8_t i_min_qp;		I帧编码的QP最小值，范围[0, 51];
	- uint8_t i_max_qp;		I帧编码的QP最大值，范围[0, 51];
	- uint8_t p_min_qp;		P帧编码的QP最小值，范围[0, 51];
	- uint8_t p_max_qp;		P帧编码的QP最大值，范围[0, 51];

.. note::

	上面的值除了需要注意在有效范围内之外，还需要注意，I/P帧编码的最大值一定要大于其最小值，方可生效。通过上面的接口可以调节图像质量，需要注意提高图像质量无疑会加大数据量，当前一帧yuv422数据编码完成的输出最大空间为64KB，如果调节图像质量，
	编码输出的H264图像大于64K，会出现不稳定的问题。

4、编码挡位调节
--------------------------

	默认SDK提供了h264编码挡位调节的配置，通过配置宏来定义：``CONFIG_H264_QUALITY_LEVEL``，具体说明如下：

	- 取值范围：[0, 3]，定义三个挡位，定义的值分别为：1/2/3，分别对应h264压缩图像质量由低到高，图像越清晰。
	- 如果CONFIG_H264_QUALITY_LEVEL=0，不使用三个挡位的参数，而使用默认的值进行配置，默认值参考路径：``.\bk_idk\middleware\soc\bk7258\hal\h264_default_config.h``，可以通过修改默认值来达到预期效果。
	- 默认SDK的挡位定义在中间挡位，CONFIG_H264_QUALITY_LEVEL=2。用户可以在工程中的配置中将宏改为需要的值。