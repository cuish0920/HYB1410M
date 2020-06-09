#pragma once

/**************************************************
***@author:cui_sh
***@brief:imperx�������
***@time:2019-06-14
***@version:
**************************************************/

#ifndef __MICHROME20__
#define __MICHROME20__


#include "opencv2\opencv.hpp"
typedef void(*UVCCallBack)(cv::Mat m_frame);
class UVCCamery
{
	public:
		UVCCamery();
		~UVCCamery();

	public:
		
		/**************************************************
		***@brief:�����
		**************************************************/
		static bool OpenCamera(uint8_t uiIdx);
		/**************************************************
		***@brief:�ر����
		**************************************************/
		static bool CloseCamera();
		
		/**************************************************
		***@brief:��ȡͼ��
		**************************************************/
		static void StartAcquisition(UVCCallBack callback);
		/**************************************************
		***@brief:�ر���Ƶ��
		**************************************************/
		static bool StopAcquisition();
		/**************************************************
		***@brief:��ȡ��Ƶ��״̬
		**************************************************/
		static bool getCameryStatus();
		static bool getAcquistionStatus();
	private:
		static cv::VideoCapture UVC;
		//static CvCapture* pCapture;
		
		static bool frameAcquisition;
		static bool waitStopAcquisition;
		static bool openStatus;
};

#endif 