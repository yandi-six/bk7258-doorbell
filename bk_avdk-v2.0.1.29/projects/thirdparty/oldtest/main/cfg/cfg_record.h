/*!
*****************************************************************************

*****************************************************************************
*/

#ifndef _SDK_CFG_RECORD_H__
#define _SDK_CFG_RECORD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

/***********************************/
/***         record              ***/
/***********************************/
typedef struct {
    SDK_S32      enable;             /*�Ƿ�¼�� 0-�� 1-��*/
    SDK_S32      stream_no;            /* ѡ���ĸ�ͨ����ʼ¼�� 0-3 */
    SDK_S32      recordMode;         //  0:��Ԥ��ʱ��¼�� 1:Ԥ��ȫ��¼�� 2:�ֶ�¼��ģʽ 3:ֹͣ¼��
    SDK_S32      preRecordTime;      /* Ԥ¼ʱ�䣬��λ��s��0��ʾ��Ԥ¼�� */
    SDK_S32      audioRecEnable;     /*¼��ʱ����������ʱ�Ƿ��¼��Ƶ����*/
    SDK_S32      recAudioType;     /*¼�Ƶ���Ƶ��ʽ 0 a-law; 1 u-law; 2 pcm; 3-adpcm*/
    SDK_S32      recordLen;          //¼���ļ����ʱ��,�Է���Ϊ��λ
    SDK_S32      recycleRecord;      //�Ƿ�ѭ��¼��,0:����; 1:��
    SDK_SCHEDTIME  scheduleTime[7][12]; //¼��ʱ��Σ�����һ��������
} SDK_NET_RECORD_CFG, *LPSDK_NET_RECORD_CFG;
extern int RecordCfgSave();
extern int RecordCfgLoad();
extern void RecordCfgPrint();
extern int RecordCfgLoadDefValue();
extern cJSON * RecordCfgLoadJson();
extern int RecordJsonSaveCfg(cJSON *json);

#define RECORD_CFG_FILE "record_cfg.cjson"

extern SDK_NET_RECORD_CFG runRecordCfg;
extern SDK_CFG_MAP recordMap[];

#ifdef __cplusplus
}
#endif
#endif

