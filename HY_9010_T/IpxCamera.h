#pragma once

/**************************************************
***@author:cui_sh
***@brief:imperx�������
***@time:2019-06-14
***@version:
**************************************************/

#ifndef __IPXCAMERA_H__
#define __IPXCAMERA_H__
#include <Windows.h>  
#include <string>
#include <vector>
#include <list>
//Imperx Camera API Functions
#include "IpxCameraApi.h"
#include <thread>
typedef void(*Imperxcallback)(IpxCam::Buffer* buffer);
class IpxCamera
{
public:
	IpxCamera();
	~IpxCamera();

public:
	/**************************************************
	***@brief:��ȡ�豸�б�
	**************************************************/
	std::string queryIpxCam();
	std::vector<IpxCam::Interface*> SelectDevice(IpxCam::System *aSystem);

	/**************************************************
	***@brief:���豸
	**************************************************/
	bool openImperx(int ImperxID);

	/**************************************************
	***@brief:�ر��豸
	**************************************************/
	bool closeImperx();

	/**************************************************
	***@brief:���ӵ�ָ���豸
	**************************************************/

	IpxCam::Device *ConnectToU3VDevice(IpxCam::DeviceInfo *aDeviceInfo);

	/**************************************************
	***@brief:tre-����Ѿ���
	***@brief:false-���δ��
	**************************************************/
	bool imperx_status;
	IpxGenParam::Array *arrays;
	/**************************************************
	***@brief:ͼ��ص���ʶ
	**************************************************/
	bool startAcquisition;
private:
	/**************************************************
	***@brief:�����������
	**************************************************/
	IpxCamErr imperxParamSet(std::string name, std::string value);
	/**************************************************
	***@brief:��������ݻ�ȡ
	**************************************************/
	IpxGenParam::Array *imperxParamGet();
public:
	/**************************************************
	***@brief:����
	**************************************************/

	IpxCam::Stream *IpxCamera::OpenStreamByIndx(IpxCam::Device *aDevice, uint32_t idx);

	void CreateDataStreamBuffers(IpxCam::Stream *aDataStream, std::list<IpxCam::Buffer*> *aBufferList);
	void FreeDataStreamBuffers(IpxCam::Stream *aDataStream, std::list<IpxCam::Buffer*> *aBufferList);

	/**************************************************
		***@brief:����Ƶ��
		**************************************************/
	void * m_instance;
	bool StartAcquisition(Imperxcallback m_callback);
	/**************************************************
	***@brief:�ر���Ƶ��
	**************************************************/
	bool StopAcquisition();
public:
	
	bool iniCameryParam();
	bool setIniParamValue(std::string param);
public:
	int64_t TESTFUNC();

	int64_t getCurrentFrameRate();
	int64_t getProgFrameTimeAbs();
	int64_t getCurrentTemperature();
	int64_t getExposureTimeRaw();
	int64_t getMaxExposure();
	int64_t getGainRaw();
	int64_t getTriggerActivation();
	int64_t getTriggerMode();

	bool setProgFrameTimeAbs(int64_t value);
	bool setExposureTimeRaw(int64_t value);
	bool setGainRaw(int64_t value);
	bool setTriggerMode(int64_t value);
	bool setTriggerActivation(int64_t value);
	static int AcquisitionThread(void *org);
	Imperxcallback _callback;
public:
	int ProgFrameTimeAbs;//֡Ƶ����
	int ExposureTimeRaw;//����ʱ������
	//�����������
	int BinningHorizontal;
	int BinningVertical;
	int Width;
	int Height;
	int ReverseX;
	int ReverseY;
	int PixelFormat;
	int TriggerMode;
	int TriggerActivation;
	int TriggerSource;
	int ExposureMode;
	int ProgFrameTimeEnable;
	int TriggerType;
public:
	IpxCam::Device *lDevice;
private:
	IpxCam::System * System;
	IpxCam::Stream *lStream;
	std::vector<IpxCam::Interface*> ifaceVector;
	std::list<IpxCam::Buffer*> lBufferList;
public:
	/** ͬ������,�ٽ������� */
	CRITICAL_SECTION   m_cs;//!<
	CRITICAL_SECTION   m_cs_stram;//!<
};

#endif 