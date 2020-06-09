#ifndef __PIO002XX_LIB_H
#define __PIO002XX_LIB_H

#include "PUSIControlLib.h"

/**************************************�궨��**************************************/
//PWMͨ��
#define PWM0	0
#define PWM1	1
#define PWM2	2
#define PWM3	3
//ADC��������
//����˿�
#define ADM_IN_AN0	0	
#define ADM_IN_AN1	1	
//����ģʽ
#define ADM_SINGLE			0  //����ת��	
#define ADM_CONSECUTIVE		2  //����ת��

//����������
//������ʹ��
#define ENC_DISABLE				0
#define ENC_ENABLE				1
//����������
#define ENC_POSEDGE_TRIGER		0
#define ENC_NEGEDGE_TRIGER		16

//�ⲿֹͣ
#ifndef EXTSTOP1
#define EXTSTOP1	0x1
#endif

#ifndef EXTSTOP2
#define EXTSTOP2	0x2
#endif

/**************************************���Ͷ���**************************************/

/**************************************������������**************************************/
//����������˿�
PUSIResult __stdcall SetPWMDutyCycle(DWORD dwAddress,DWORD dwPWMChannel,DWORD dwDutyCycle); //����PWMռ�ձȣ�һ������һ��ͨ��
PUSIResult __stdcall GetPWMDutyCycle(DWORD dwAddress,DWORD *pDutyCycle); //��ȡPWMռ�ձȣ�һ�λ�ȡ�ĸ�ͨ��

PUSIResult __stdcall SetDrvOutput(DWORD dwAddress,DWORD dwDrvOutput); //���ô������������˿�
PUSIResult __stdcall GetDrvOutput(DWORD dwAddress,DWORD *pDrvOutput); //��ȡ�������������˿�
//ADC
PUSIResult __stdcall StartADC(DWORD dwAddress,DWORD dwADCCfg); //����ADC��example: ADM_IN_AN1|ADM_CONSECUTIVE
PUSIResult __stdcall StopADC(DWORD dwAddress); //ֹͣADC
PUSIResult __stdcall GetADCResult(DWORD dwAddress,DWORD *pADCResult); //��ȡADCת�����
//GPIO
PUSIResult __stdcall SetGPIODirection(DWORD dwAddress,DWORD dwDir); //����GPIO����
PUSIResult __stdcall GetGPIODirection(DWORD dwAddress,DWORD *pDir); //��ȡGPIO����
//LED
PUSIResult __stdcall SetLEDEnable(DWORD dwAddress,BOOL bEnable); //����LED��ʾʹ��
PUSIResult __stdcall SetLEDDisplay(DWORD dwAddress,DWORD dwDispData); //����LED��ʾ����
//encoder
PUSIResult __stdcall SetEncoderEnable(DWORD dwAddress,DWORD dwEncoderCfg); //���ñ�����ʹ��,ENC_ENABLE|ENC_NEGEDGE_TRIGER
PUSIResult __stdcall SetEncoderPos(DWORD dwAddress,int nEncoderPos); //���ñ�����λ��
PUSIResult __stdcall GetEncoderPos(DWORD dwAddress,int *pEncoderPos); //��ȡ������λ��
//user data
PUSIResult __stdcall SetUserDataAddr(DWORD dwAddress,DWORD dwUserDataAddr); //�����û����ݵ�ַ
PUSIResult __stdcall GetUserDataAddr(DWORD dwAddress,DWORD *pUserDataAddr); //��ȡ�û����ݵ�ַ
PUSIResult __stdcall WriteUserData(DWORD dwAddress,BYTE bUserData); //дһ���ֽ��û�����
PUSIResult __stdcall ReadUserData(DWORD dwAddress,BYTE *pUserData); //��һ���ֽ��û�����
//motor drive
PUSIResult __stdcall SetMotorDriveEnable(DWORD dwAddress,BOOL bEnable); //�����������ʹ��
PUSIResult __stdcall SetVelocityMode(DWORD dwAddress,BOOL bVelocityMode); //�����ٶ�ģʽ
PUSIResult __stdcall StartStep(DWORD dwAddress,DWORD dwStepNum); //��������
PUSIResult __stdcall AbortStep(DWORD dwAddress); //��ǰ����������ֹ	
PUSIResult __stdcall SetMotorPosition(DWORD dwAddress,int dwMotorPos); //�������λ��
PUSIResult __stdcall GetMotorPosition(DWORD dwAddress,int* pMotorPos); //��ȡ���λ��
PUSIResult __stdcall SetDirection(DWORD dwAddress,BOOL bReverse); //����ת������
PUSIResult __stdcall SetVelocity(DWORD dwAddress,DWORD dwVelocity); //����ת���ٶ�
PUSIResult __stdcall GetVelocity(DWORD dwAddress,DWORD *pVelocity); //��ȡת���ٶ�
PUSIResult __stdcall SetStartVelocity(DWORD dwAddress,DWORD dwStartVelocity); //���������ٶ�
PUSIResult __stdcall GetStartVelocity(DWORD dwAddress,DWORD *pStartVelocity); //��ȡ�����ٶ�
PUSIResult __stdcall SetStopVelocity(DWORD dwAddress,DWORD dwStopVelocity); //����ֹͣ�ٶ�
PUSIResult __stdcall GetStopVelocity(DWORD dwAddress,DWORD *pStopVelocity); //��ȡֹͣ�ٶ�
PUSIResult __stdcall SetAccCoff(DWORD dwAddress,DWORD dwCoff); //���ü���ϵ��
PUSIResult __stdcall GetAccCoff(DWORD dwAddress,DWORD *pCoff);//��ȡ����ϵ��
PUSIResult __stdcall SetDecCoff(DWORD dwAddress,DWORD dwCoff); //���ü���ϵ��
PUSIResult __stdcall GetDecCoff(DWORD dwAddress,DWORD *pCoff); //��ȡ����ϵ��
PUSIResult __stdcall SetExtStopEnable(DWORD dwAddress,DWORD dwStopEnable); //�����ⲿ����ֹͣʹ��
PUSIResult __stdcall SetExtStopTrigerMode(DWORD dwAddress,DWORD dwTrigerMode); //�����ⲿ����ֹͣ������ʽ
PUSIResult __stdcall GetExtStopTrigerMode(DWORD dwAddress,DWORD *pTrigerMode); //��ȡ�ⲿ����ֹͣ������ʽ
PUSIResult __stdcall ClearExtStop(DWORD dwAddress,DWORD dwExtStop); //����ⲿֹͣ
PUSIResult __stdcall GetControllerStatus1(DWORD dwAddress,DWORD *pStatus); //��ȡ������״̬1
PUSIResult __stdcall GetControllerStatus2(DWORD dwAddress,DWORD *pStatus); //��ȡ������״̬2
#endif
