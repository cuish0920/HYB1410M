#ifndef __PMC006XX_LIB_H
#define __PMC006XX_LIB_H

#include "PUSIControlLib.h"

/**************************************�궨��**************************************/
//�ⲿֹͣ
#ifndef EXTSTOP1
#define EXTSTOP1	0x1
#endif

#ifndef EXTSTOP2
#define EXTSTOP2	0x2
#endif

#ifdef __cplusplus
extern "C" {
#endif
/**************************************���Ͷ���**************************************/


/**************************************������������**************************************/
PUSIResult __stdcall SetVelocityMode(DWORD dwAddress,BOOL bVelocityMode); //�����ٶ�ģʽ
PUSIResult __stdcall StartStep(DWORD dwAddress,DWORD dwStepNum); //��������
PUSIResult __stdcall AbortStep(DWORD dwAddress); //��ǰ����������ֹ	
PUSIResult __stdcall SetMicroStep(DWORD dwAddress,DWORD dwMicroStep); //����ϸ����
PUSIResult __stdcall GetMicroStep(DWORD dwAddress,DWORD* pMicroStep); //��ȡϸ����
PUSIResult __stdcall SetMotorPosition(DWORD dwAddress,int nMotorPos); //�������λ��
PUSIResult __stdcall GetMotorPosition(DWORD dwAddress,int* pMotorPos); //��ȡ���λ��
PUSIResult __stdcall SetDirection(DWORD dwAddress,BOOL bReverse); //����ת������
PUSIResult __stdcall SetVelocity(DWORD dwAddress,DWORD dwVelocity); //����ת���ٶ�
PUSIResult __stdcall GetVelocity(DWORD dwAddress,DWORD *pVelocity); //��ȡת���ٶ�
PUSIResult __stdcall SetVelocityCompFactor(DWORD dwAddress,DWORD dwVelocityComp); //�����ٶȲ�������
PUSIResult __stdcall GetVelocityCompFactor(DWORD dwAddress,DWORD *pVelocityComp); //��ȡ�ٶȲ�������
PUSIResult __stdcall SetStartVelocity(DWORD dwAddress,DWORD dwStartVelocity); //���������ٶ�
PUSIResult __stdcall GetStartVelocity(DWORD dwAddress,DWORD *pStartVelocity); //��ȡ�����ٶ�
PUSIResult __stdcall SetStopVelocity(DWORD dwAddress,DWORD dwStopVelocity); //����ֹͣ�ٶ�
PUSIResult __stdcall GetStopVelocity(DWORD dwAddress,DWORD *pStopVelocity); //��ȡֹͣ�ٶ�
PUSIResult __stdcall SetAccCoff(DWORD dwAddress,DWORD dwCoff); //���ü���ϵ��
PUSIResult __stdcall GetAccCoff(DWORD dwAddress,DWORD *pCoff);//��ȡ����ϵ��
PUSIResult __stdcall SetDecCoff(DWORD dwAddress,DWORD dwCoff); //���ü���ϵ��
PUSIResult __stdcall GetDecCoff(DWORD dwAddress,DWORD *pCoff); //��ȡ����ϵ��
PUSIResult __stdcall SetMaxCurrent(DWORD dwAddress,DWORD dwMaxCurrent); //����������
PUSIResult __stdcall GetMaxCurrent(DWORD dwAddress,DWORD *pMaxCurrent); //��ȡ������
PUSIResult __stdcall SetCurrCompFactor(DWORD dwAddress,DWORD dwCompFactor); //���õ�����������
PUSIResult __stdcall GetCurrCompFactor(DWORD dwAddress,DWORD *pCompFactor); //��ȡ������������
PUSIResult __stdcall SetExtStopEnable(DWORD dwAddress,DWORD dwStopEnable); //�����ⲿ����ֹͣʹ��
PUSIResult __stdcall SetExtStopTrigerMode(DWORD dwAddress,DWORD dwTrigerMode); //�����ⲿ����ֹͣ������ʽ
PUSIResult __stdcall GetExtStopTrigerMode(DWORD dwAddress,DWORD *pTrigerMode); //��ȡ�ⲿ����ֹͣ������ʽ
PUSIResult __stdcall ClearExtStop(DWORD dwAddress,DWORD dwExtStop); //����ⲿֹͣ
PUSIResult __stdcall SetCurrentAutoDecay(DWORD dwAddress,BOOL bEnable); //���õ���˥��ʹ��
PUSIResult __stdcall SetCurrentDecayFactor(DWORD dwAddress,DWORD dwDecayFactor); //���õ���˥������
PUSIResult __stdcall GetCurrentDecayFactor(DWORD dwAddress,DWORD *pDecayFactor); //��ȡ����˥������
PUSIResult __stdcall SetOfflineEnable(DWORD dwAddress,BOOL bEnable); //��������ʹ��
PUSIResult __stdcall GetControllerStatus1(DWORD dwAddress,DWORD *pStatus); //��ȡ������״̬1
PUSIResult __stdcall GetControllerStatus2(DWORD dwAddress,DWORD *pStatus); //��ȡ������״̬2

PUSIResult __stdcall GetStallConfig(DWORD dwAddress,DWORD *pStallConfig); //��ȡ��ת����
PUSIResult __stdcall SetStallConfig(DWORD dwAddress,DWORD dwStallConfig); //���ö�ת����
PUSIResult __stdcall GetStallTriger(DWORD dwAddress,DWORD *pTriger); //��ȡ��ת����ֵ
PUSIResult __stdcall SetStallTriger(DWORD dwAddress,DWORD dwTriger); //���ö�ת����ֵ
PUSIResult __stdcall GetStallTrigerLength(DWORD dwAddress,DWORD *pTrigerLen); //��ȡ��ת��������
PUSIResult __stdcall SetStallTrigerLength(DWORD dwAddress,DWORD dwTrigerLen); //���ö�ת��������
PUSIResult __stdcall ClearStallFlag(DWORD dwAddress); //�����ת��־λ



#ifdef __cplusplus
}
#endif

#endif
