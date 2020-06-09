#ifndef __PCS0801_LIB_H
#define __PCS0801_LIB_H

#include "PUSIControlLib.h"

/**************************************�궨��**************************************/
#define WORKMODE_CONSECUTIVE	0
#define WORKMODE_CALIBRATION	1

/**************************************���Ͷ���**************************************/

/**************************************������������**************************************/
PUSIResult __stdcall SetAutoLoadCalibData(DWORD dwAddress,BOOL bAutoLoad); //�����Զ�����У׼����
PUSIResult __stdcall SetWorkMode(DWORD dwAddress,DWORD dwWorkMode); //���ù���ģʽ
PUSIResult __stdcall SetMeasureMode(DWORD dwAddress,DWORD dwMeasureMode); //���ò���ģʽ
PUSIResult __stdcall GetSensorWorkMode(DWORD dwAddress,DWORD *pWorkMode); //���������Ĺ���ģʽ
PUSIResult __stdcall GetSensorWorkStatus(DWORD dwAddress,DWORD *pWorkStatus); //���������Ĺ���״̬
PUSIResult __stdcall GetCapValue(DWORD dwAddress,DWORD *pCapValue); //������ֵ
PUSIResult __stdcall GetCapCalibData(DWORD dwAddress,DWORD *pCapCalibData); //������У׼����
PUSIResult __stdcall SetCapCalibData(DWORD dwAddress,DWORD dwCapCalibData); //���õ���У׼ֵ
PUSIResult __stdcall SetCapOffset(DWORD dwAddress,DWORD dwCapOffset); //���õ���ƫ����
PUSIResult __stdcall SetCapTriger1(DWORD dwAddress,DWORD dwCapTriger1); //���õ��ݴ���ֵ1
PUSIResult __stdcall GetCapTriger1(DWORD dwAddress,DWORD *pCapTriger1); //��ȡ���ݴ���ֵ1
PUSIResult __stdcall SetCapTriger2(DWORD dwAddress,DWORD dwCapTriger2); //���õ��ݴ���ֵ2
PUSIResult __stdcall GetCapTriger2(DWORD dwAddress,DWORD *pCapTriger2); //��ȡ���ݴ���ֵ2
PUSIResult __stdcall SetCapTriger3(DWORD dwAddress,DWORD dwCapTriger3); //���õ��ݴ���ֵ3
PUSIResult __stdcall GetCapTriger3(DWORD dwAddress,DWORD *pCapTriger3); //���õ��ݴ���ֵ3
PUSIResult __stdcall SetCapTriger4(DWORD dwAddress,DWORD dwCapTriger4); //���õ��ݴ���ֵ4
PUSIResult __stdcall GetCapTriger4(DWORD dwAddress,DWORD *pCapTriger4); //���õ��ݴ���ֵ4
PUSIResult __stdcall SetMeasureRangeAdjust(DWORD dwAddress,BOOL bEnable); //������Χ����ʹ��
PUSIResult __stdcall SetMeasureAdjustA(DWORD dwAddress,DWORD dwAdjustA); //���ò�������ֵA
PUSIResult __stdcall SetMeasureAdjustB(DWORD dwAddress,DWORD dwAdjustB); //���ò�������ֵB
PUSIResult __stdcall SetVoltChannel(DWORD dwAddress,BOOL bEnable); //���õ�ѹͨ��ʹ��
PUSIResult __stdcall GetVoltValue(DWORD dwAddress,DWORD *pVoltValue); //��ȡ��ѹֵ
PUSIResult __stdcall ResetSensor(DWORD dwAddress); //��λ������

#endif
