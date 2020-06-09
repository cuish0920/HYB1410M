#include "IpxCamera.h"
#include <iostream>

#include <algorithm>


//////////////////����json///////////////////
#include "CJsonObject.h"
//////////////////����json///////////////////
#include "LogUtils.h"
/**************************************************
***@author:cui_sh
***@brief:���캯��
***@time:
***@version:
**************************************************/

IpxCamera::IpxCamera() :lDevice(nullptr), arrays(nullptr), imperx_status(false),
startAcquisition(false)
{
	//lDevice = nullptr;
	//startAcquisition = false;
	InitializeCriticalSection(&m_cs);//
	
	InitializeCriticalSection(&m_cs_stram);
	//��ʹ������Ĳ���
	ProgFrameTimeAbs = 50;//֡Ƶ����
	ExposureTimeRaw = 20;//����ʱ������
	BinningHorizontal = 4;
	BinningVertical = 4;
	Width = 344;
	Height = 260;
	ReverseX = 0;
	//imperx->ReverseY = this->ReverseY;
	PixelFormat = 17825797;
	TriggerMode = 0;
	TriggerActivation = 0;
	TriggerSource = 27;
	ExposureMode = 1;
	ProgFrameTimeEnable = 1;
	TriggerType = 1;

	//arrays = nullptr;
}


/**************************************************
***@author:cui_sh
***@brief:��������
***@time:
***@version:
**************************************************/

IpxCamera::~IpxCamera()
{
	
	DeleteCriticalSection(&m_cs_stram);
	DeleteCriticalSection(&m_cs);//
}

/**************************************************
***@author:cui_sh
***@brief:��ȡ����б�
***@time:
***@version:
**************************************************/

std::string IpxCamera::queryIpxCam()
{
	try {
		/** �����ٽ�� m_cs  */
		EnterCriticalSection(&m_cs);
		System = IpxCam::IpxCam_GetSystem();
		std::vector<IpxCam::Interface*>ifaceVector = SelectDevice(System);
		neb::CJsonObject imperxList;
		IpxCam::Interface *iface = NULL;
		IpxCam::DeviceInfo *lDeviceInfo = NULL;
		imperxList.AddEmptySubArray("imperx");
		for (; !ifaceVector.empty();)
		{
			neb::CJsonObject imperxItem;

			iface = ifaceVector.back();

			imperxItem.Add("name", iface->GetFirstDeviceInfo()->GetModel());
			imperxItem.Add("value", iface->GetDescription());
			imperxItem.Add("id", ifaceVector.size() - 1);

			imperxList["imperx"].Add(imperxItem);
			ifaceVector.pop_back();
		}
		/** �뿪�ٽ�� m_cs */
		LeaveCriticalSection(&m_cs);
		return imperxList.ToString();
	}
	catch (...) {

		LogUtils::error->error("��������-queryIpxCam");
		/** �뿪�ٽ�� m_cs */
		LeaveCriticalSection(&m_cs);
	}

}

std::vector<IpxCam::Interface*> IpxCamera::SelectDevice(IpxCam::System *aSystem)
{
	try {
		/** �����ٽ�� m_cs  */
		EnterCriticalSection(&m_cs);
		if (aSystem != nullptr)
		{
			IpxCam::Interface *iface = NULL;

			uint32_t idx = 0;

			ifaceVector.clear();
			auto list = aSystem->GetInterfaceList();
			for (auto iface = list->GetFirst(); iface; iface = list->GetNext())
			{
				if (iface->GetFirstDeviceInfo() != nullptr)
				{
					ifaceVector.push_back(iface);

					idx++;
				}


			}

			return ifaceVector;
		}
		/** �뿪�ٽ�� m_cs */
		LeaveCriticalSection(&m_cs);
		return {};
	}
	catch (...) {
		LogUtils::error->error("��������-SelectDevice");
		/** �뿪�ٽ�� m_cs */
		LeaveCriticalSection(&m_cs);
	}

}

/**************************************************
***@author:cui_sh
***@brief:�����
***@time:
***@version:
**************************************************/

bool IpxCamera::openImperx(int ImperxID)
{
	try {
		/** �����ٽ�� m_cs  */
		EnterCriticalSection(&m_cs);
		if (ifaceVector.empty()) {
			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return false;
		}

		IpxCam::DeviceInfo *lDeviceInfo = ifaceVector.at(ImperxID)->GetFirstDeviceInfo();
		if (lDeviceInfo != nullptr)
		{
			lDevice = ConnectToU3VDevice(lDeviceInfo);
			if (lDevice == nullptr)
			{
				/** �뿪�ٽ�� m_cs */
				LeaveCriticalSection(&m_cs);
				return false;
			}


			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return true;
		}
		/** �뿪�ٽ�� m_cs */
		LeaveCriticalSection(&m_cs);
		return false;
	}
	catch (...) {
		LogUtils::error->error("��������-openImperx");
		/** �뿪�ٽ�� m_cs */
		LeaveCriticalSection(&m_cs);
	}

}

/**************************************************
***@author:cui_sh
***@brief:�ر��豸
***@time:
***@version:
**************************************************/

bool IpxCamera::closeImperx()
{
	try {
		/**************************************************
		***@brief:����Ƿ����Ƶ��
		**************************************************/
		/** �����ٽ�� m_cs  */
		EnterCriticalSection(&m_cs);
		if (startAcquisition)
		{
			//�˳���Ƶ��
			this->StopAcquisition();
		}


		if (imperx_status)
		{
			imperx_status = false;
			//Disconnecting device
			lDevice->Release();
			lDevice = nullptr;
			arrays = nullptr;
			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return true;
		}
		/** �뿪�ٽ�� m_cs */
		LeaveCriticalSection(&m_cs);
		return false;
	}
	catch (...) {
		LogUtils::error->error("��������-closeImperx");
		/** �뿪�ٽ�� m_cs */
		LeaveCriticalSection(&m_cs);
	}


}


/**************************************************
***@author:cui_sh
***@brief:���ӵ�ָ�����豸
***@time:
***@version:
**************************************************/
IpxCam::Device *IpxCamera::ConnectToU3VDevice(IpxCam::DeviceInfo *aDeviceInfo)
{
	try {
		// Create the camera
		/** �����ٽ�� m_cs  */
		EnterCriticalSection(&m_cs);
		IpxCam::Device *lDevice = IpxCam_CreateDevice(aDeviceInfo);
		if (lDevice == nullptr)
		{
			//printf("ConnectToU3VDevice��error\r\n");

			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return nullptr;
		}
		//printf("ConnectToU3VDevice��success\r\n");
		/** �뿪�ٽ�� m_cs */
		LeaveCriticalSection(&m_cs);
		return lDevice;
	}
	catch (...) {
		LogUtils::error->error("��������-ConnectToU3VDevice");
		/** �뿪�ٽ�� m_cs */
		LeaveCriticalSection(&m_cs);
	}

}

IpxGenParam::Array *IpxCamera::imperxParamGet()
{
	try {
		/** �����ٽ�� m_cs  */
		//EnterCriticalSection(&m_cs);
		if (arrays == nullptr)
		{

			arrays = lDevice->GetCameraParameters();


		}
		/** �뿪�ٽ�� m_cs */
		//LeaveCriticalSection(&m_cs);
		return arrays;
	}
	catch (...) {
		LogUtils::error->error("��������-imperxParamGet");
		/** �뿪�ٽ�� m_cs */
		//LeaveCriticalSection(&m_cs);
	}


}

/*��ȡ���س�ʹ������*/
bool IpxCamera::setIniParamValue(std::string param)
{
	try {
		/** �����ٽ�� m_cs  */
		EnterCriticalSection(&m_cs);
		double value;
		neb::CJsonObject sys_iniparam(param);

		if (!sys_iniparam.Get("ProgFrameTimeAbs", ProgFrameTimeAbs)) {
			sys_iniparam.Get("ProgFrameTimeAbs", value);
			ProgFrameTimeAbs = value;
		}
		//printf("system_ini=%d \r\n", system_ini);
		//sys_iniparam.Get("ExposureTimeRaw", ExposureTimeRaw);
		if (!sys_iniparam.Get("ExposureTimeRaw", ExposureTimeRaw)) {
			sys_iniparam.Get("ExposureTimeRaw", value);
			ExposureTimeRaw = value;
		}

		//printf(" ExposureTimeRaw = %d\r\n",ExposureTimeRaw);
		//printf("ExposureTimeRaw=%d \r\n", ExposureTimeRaw);
		//sys_iniparam.Get("BinningHorizontal", BinningHorizontal);
		if (!sys_iniparam.Get("BinningHorizontal", BinningHorizontal)) {
			sys_iniparam.Get("BinningHorizontal", value);
			BinningHorizontal = value;
		}
		//printf("BinningHorizontal=%d \r\n", BinningHorizontal);
		//sys_iniparam.Get("BinningVertical", BinningVertical);
		if (!sys_iniparam.Get("BinningVertical", BinningVertical)) {
			sys_iniparam.Get("BinningVertical", value);
			BinningVertical = value;
		}
		//printf("BinningVertical=%d \r\n", BinningVertical);
		//sys_iniparam.Get("Width", Width);
		if (!sys_iniparam.Get("Width", Width)) {
			sys_iniparam.Get("Width", value);
			Width = value;
		}

	
		if (!sys_iniparam.Get("Height", Height)) {
			sys_iniparam.Get("Height", value);
			Height = value;
		}
		//sys_iniparam.Get("ReverseX", ReverseX);

		if (!sys_iniparam.Get("ReverseX", ReverseX)) {
			sys_iniparam.Get("ReverseX", value);
			ReverseX = value;
		}

		//sys_iniparam.Get("PixelFormat", PixelFormat);
		if (!sys_iniparam.Get("PixelFormat", PixelFormat)) {
			sys_iniparam.Get("PixelFormat", value);
			PixelFormat = value;
		}
		//sys_iniparam.Get("TriggerMode", TriggerMode);
		if (!sys_iniparam.Get("TriggerMode", TriggerMode)) {
			sys_iniparam.Get("TriggerMode", value);
			TriggerMode = value;
		}
		//sys_iniparam.Get("TriggerActivation", TriggerActivation);
		if (!sys_iniparam.Get("TriggerActivation", TriggerActivation)) {
			sys_iniparam.Get("TriggerActivation", value);
			TriggerActivation = value;
		}

		//sys_iniparam.Get("TriggerSource", TriggerSource);
		if (!sys_iniparam.Get("TriggerSource", TriggerSource)) {
			sys_iniparam.Get("TriggerSource", value);
			TriggerSource = value;
		}

		//sys_iniparam.Get("ExposureMode", ExposureMode);
		if (!sys_iniparam.Get("ExposureMode", ExposureMode)) {
			sys_iniparam.Get("ExposureMode", value);
			ExposureMode = value;
		}
		//sys_iniparam.Get("ProgFrameTimeEnable", ProgFrameTimeEnable);
		if (!sys_iniparam.Get("ProgFrameTimeEnable", ProgFrameTimeEnable)) {
			sys_iniparam.Get("ProgFrameTimeEnable", value);
			ProgFrameTimeEnable = value;
		}
		//sys_iniparam.Get("TriggerType", TriggerType);
		if (!sys_iniparam.Get("TriggerType", TriggerType)) {
			sys_iniparam.Get("TriggerType", value);
			TriggerType = value;
		}
		//������Ԫ�ϲ�
		//sys_iniparam.Get("BinningHorizontal", BinningHorizontal);
		if (!sys_iniparam.Get("BinningHorizontal", BinningHorizontal)) {
			sys_iniparam.Get("BinningHorizontal", value);
			BinningHorizontal = value;
		}
		Width = 1376 / BinningHorizontal;

		//sys_iniparam.Get("BinningVertical", BinningVertical);
		if (!sys_iniparam.Get("BinningVertical", BinningVertical)) {
			sys_iniparam.Get("BinningVertical", value);
			BinningVertical = value;
		}
		//��߼���
		Height = 1040 / BinningVertical;

		/** �뿪�ٽ�� m_cs */
		LeaveCriticalSection(&m_cs);
		return true;
	}
	catch (...) {
		LogUtils::error->error("��������-setIniParamValue");
		/** �뿪�ٽ�� m_cs */
		LeaveCriticalSection(&m_cs);
	}

}
bool IpxCamera::iniCameryParam()
{
	try {
		/** �����ٽ�� m_cs  */
		EnterCriticalSection(&m_cs);
		IpxCamErr err;
		arrays = imperxParamGet();

		if (arrays == nullptr)
		{
			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return false;
		}
		//Ϊ��ʹϵͳ�����������ý�ȥ
		err = imperxParamSet("ProgFrameTimeEnable", std::to_string(0));
		if (err != IpxCamErr::IPX_CAM_ERR_OK) {
			printf("%s\r\n", "ProgFrameTimeEnable");
			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return false;
		}

		err = imperxParamSet("BinningHorizontal", std::to_string(BinningHorizontal));
		if (err != IpxCamErr::IPX_CAM_ERR_OK) {
			printf("%s\r\n", "BinningHorizontal");
			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return false;
		}

		err = imperxParamSet("BinningVertical", std::to_string(BinningVertical));
		if (err != IpxCamErr::IPX_CAM_ERR_OK) {
			printf("%s\r\n", "BinningVertical");
			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return false;
		}
		//LogUtils::debugger->debug("Width=%d", Width);
		err = imperxParamSet("Width", std::to_string(Width));
		if (err != IpxCamErr::IPX_CAM_ERR_OK) {
			printf("%s\r\n", "Width");
			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return false;
		}
		//LogUtils::debugger->debug("Height=%d", Height);
		err = imperxParamSet("Height", std::to_string(Height));
		if (err != IpxCamErr::IPX_CAM_ERR_OK) {
			printf("%s\r\n", "Height");
			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return false;
		}

		///*
		//"Mono8": 17301505 (0x01080001)
		//"Mono10": 17825795 (0x01100003)
		//"Mono10Packed": 17563652 (0x010c0004)
		//"Mono12": 17825797 (0x01100005)
		//"Mono12Packed": 17563654 (0x010c0006)
		//"Mono16": 17825799 (0x01100007)
		//"BayerGR8": 17301512 (0x01080008)
		//"BayerRG8": 17301513 (0x01080009)
		//"BayerGB8": 17301514 (0x0108000a)
		//"BayerBG8": 17301515 (0x0108000b)
		//"BayerGR10": 17825804 (0x0110000c)
		//"BayerRG10": 17825805 (0x0110000d)
		//"BayerGB10": 17825806 (0x0110000e)
		//"BayerBG10": 17825807 (0x0110000f)
		//"BayerGR12": 17825808 (0x01100010)
		//"BayerRG12": 17825809 (0x01100011)
		//"BayerGB12": 17825810 (0x01100012)
		//"BayerBG12": 17825811 (0x01100013)
		//"RGB8Packed": 35127316 (0x02180014)
		//*/
		//err = m_this->imperxParamSet("PixelFormat", std::to_string(II_PIX_MONO12));
		//LogUtils::debugger->debug("PixelFormat=%d", PixelFormat);
		err = imperxParamSet("PixelFormat", std::to_string(PixelFormat));
		if (err != IpxCamErr::IPX_CAM_ERR_OK) {
			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return false;
		}
		//printf("777777777\r\n");
		//printf("TriggerMode = %d\r\n", TriggerMode);
		err = imperxParamSet("TriggerMode", std::to_string(TriggerMode));//0-off 1-on
		if (err != IpxCamErr::IPX_CAM_ERR_OK) {
			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return false;
		}
		//printf("666666666666\r\n");
		err = imperxParamSet("TriggerActivation", std::to_string(TriggerActivation));//0-RisingEdge entries: 1-FallingEdge
		if (err != IpxCamErr::IPX_CAM_ERR_OK) {
			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return false;
		}
		//printf("55555555555\r\n");
		err = imperxParamSet("TriggerSource", std::to_string(TriggerSource));//
		if (err != IpxCamErr::IPX_CAM_ERR_OK) {
			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return false;
		}
		//printf("444444444444\r\n");
		err = imperxParamSet("ExposureMode", std::to_string(ExposureMode));
		if (err != IpxCamErr::IPX_CAM_ERR_OK) {
			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return false;
		}



		//printf("3333333333333\r\n");
		//err = imperxParamSet("ReverseX", std::to_string(ReverseX));
		//if (err != IpxCamErr::IPX_CAM_ERR_OK) {
		//	/** �뿪�ٽ�� m_cs */
		//	LeaveCriticalSection(&m_cs);
		//	return false;
		//}


		/*err =imperxParamSet("ReverseY", std::to_string(m_this->ReverseY));
		if (err != ((IpxCamErr)0)) throw AcquireImages_error();*/
		//printf("22222222222\r\n");
		err = imperxParamSet("TriggerType", std::to_string(TriggerType));
		if (err != IpxCamErr::IPX_CAM_ERR_OK) {
			/** �뿪�ٽ�� m_cs */
			LeaveCriticalSection(&m_cs);
			return false;
		}
		//printf("11111111\r\n");
		/**************************************************
		***@brief:��������֡Ƶ ProgFrameTimeAbs= (1/֡Ƶ)*1000000
		**************************************************/
		if (TriggerMode == 0)
		{
			//ȡ�õ�ǰ֡
			int currentFps = 68;//getCurrentFrameRate();
			(currentFps > ProgFrameTimeAbs) ? ProgFrameTimeAbs = ProgFrameTimeAbs : ProgFrameTimeAbs = currentFps;
			LogUtils::debugger->debug("ProgFrameTimeAbs=%d %s", ProgFrameTimeAbs, std::to_string((1.0 / ProgFrameTimeAbs * 1000000)).c_str());
			err = imperxParamSet("ProgFrameTimeAbs", std::to_string((1.0 / ProgFrameTimeAbs * 1000000)));
			if (err != ((IpxCamErr)0)) {
				/** �뿪�ٽ�� m_cs */
				LeaveCriticalSection(&m_cs);
				return false;
			}
			err = imperxParamSet("ProgFrameTimeEnable", std::to_string(ProgFrameTimeEnable));
			if (err != ((IpxCamErr)0)) {
				/** �뿪�ٽ�� m_cs */
				LeaveCriticalSection(&m_cs);
				return false;
			}

			Sleep(200);
			int maxExposure = getMaxExposure();
			//LogUtils::debugger->debug("maxExposure=%d ExposureTimeRaw=%d", maxExposure,ExposureTimeRaw); 
			(maxExposure > ExposureTimeRaw) ? ExposureTimeRaw = ExposureTimeRaw : ExposureTimeRaw = maxExposure;
			//LogUtils::debugger->debug("maxExposure=%d ExposureTimeRaw=%d", maxExposure, ExposureTimeRaw);

			err = imperxParamSet("ExposureTimeRaw", std::to_string(ExposureTimeRaw));
			if (err != ((IpxCamErr)0)) {
				/** �뿪�ٽ�� m_cs */
				LeaveCriticalSection(&m_cs);
				return false;
			}

			

		}

		imperx_status = true;
		/** �뿪�ٽ�� m_cs */
		LeaveCriticalSection(&m_cs);
		return imperx_status;//���豸�ɹ�
	}
	catch (...) {
		LogUtils::error->error("��������-iniCameryParam");
		/** �뿪�ٽ�� m_cs */
		LeaveCriticalSection(&m_cs);
	}

}

IpxCamErr IpxCamera::imperxParamSet(std::string name, std::string value)
{
	try {
		/** �����ٽ�� m_cs  */
		//EnterCriticalSection(&m_cs);
		//����������
		IpxCamErr err;
		//IpxGenParam::Array *arrays = lDevice->GetCameraParameters(&err);
		if (arrays == NULL)
		{

			/** �뿪�ٽ�� m_cs */
			//LeaveCriticalSection(&m_cs);
			return IPX_CAM_ERR_UNKNOWN;
		}
		IpxGenParam::Param *m_param = arrays->GetParam(name.c_str(), &err);

		IpxGenParam::ParamType type = m_param->GetType();

		//std::cout << m_param->GetName() << "		" << value << std::endl;

		switch (type)
		{
		case IpxGenParam::ParamType::ParamInt:

			/** �뿪�ٽ�� m_cs */
			//LeaveCriticalSection(&m_cs);
			return arrays->SetIntegerValue(m_param->GetName(), atoll(value.c_str()));
			break;
		case IpxGenParam::ParamType::ParamFloat:
			/** �뿪�ٽ�� m_cs */
			//LeaveCriticalSection(&m_cs);
			return arrays->SetFloatValue(m_param->GetName(), atof(value.c_str()));
			break;
		case IpxGenParam::ParamType::ParamString:
			/** �뿪�ٽ�� m_cs */
			//LeaveCriticalSection(&m_cs);
			return arrays->SetStringValue(m_param->GetName(), value.c_str());
			break;
		case IpxGenParam::ParamType::ParamEnum:
			/** �뿪�ٽ�� m_cs */
			//LeaveCriticalSection(&m_cs);
			return arrays->SetEnumValue(m_param->GetName(), atoll(value.c_str()));
			//err = arrays->SetEnumValueStr(m_param->GetName(), "Timed");

			//return arrays->SetEnumValue("ExposureMode", 1);
			break;
			/*case IpxGenParam::ParamType::ParamEnumEntry:
				std::cout << "ParamEnumEntry ";
				break;*/
		case IpxGenParam::ParamType::ParamBoolean:
			//LeaveCriticalSection(&m_cs);
			return arrays->SetBooleanValue(m_param->GetName(), atoi(value.c_str()));
			break;
		case IpxGenParam::ParamType::ParamCommand:
			/** �뿪�ٽ�� m_cs */
			//LeaveCriticalSection(&m_cs);
			return arrays->ExecuteCommand(value.c_str());
			break;
			/*case IpxGenParam::ParamType::ParamCategory:
				std::cout << "ParamCategory ";
				break;*/
		case IpxGenParam::ParamType::ParamUnknown:
			/** �뿪�ٽ�� m_cs */
			//LeaveCriticalSection(&m_cs);
			return IPX_CAM_ERR_UNKNOWN;
			break;

		default:
			/** �뿪�ٽ�� m_cs */
			//LeaveCriticalSection(&m_cs);
			return IPX_CAM_ERR_UNKNOWN;
			break;
		}
		/** �뿪�ٽ�� m_cs */
		//LeaveCriticalSection(&m_cs);
		return IPX_CAM_ERR_UNKNOWN;
	}
	catch (...) {
		LogUtils::error->error("��������-imperxParamSet");
		/** �뿪�ٽ�� m_cs */
		//LeaveCriticalSection(&m_cs);
	}

}
int64_t IpxCamera::TESTFUNC()
{
	//EnterCriticalSection(&m_cs);
	printf("TESTFUNC\r\n");
	//LeaveCriticalSection(&m_cs);
	return 0;
}

int64_t IpxCamera::getCurrentFrameRate()
{
	try {
		//printf("getCurrentFrameRate �����ٽ��\r\n");
		///** �����ٽ�� m_cs  */
		//EnterCriticalSection(&m_cs);
		///** �뿪�ٽ�� m_cs */
		//LeaveCriticalSection(&m_cs);

		
		IpxCamErr err;
		IpxGenParam::Array *m_param = imperxParamGet();
		if (m_param == nullptr)
		{

			/** �뿪�ٽ�� m_cs */
			//LeaveCriticalSection(&m_cs);
			return 0;
		}

		/** �뿪�ٽ�� m_cs */
		//LeaveCriticalSection(&m_cs);
		return m_param->GetIntegerValue("CurrentFrameRate", &err);

	}
	catch (...) {
		LogUtils::error->error("��������-getCurrentFrameRate");
		printf("error �����ٽ��\r\n");
		/** �뿪�ٽ�� m_cs */
		//LeaveCriticalSection(&m_cs);
	}

}
int64_t IpxCamera::getCurrentTemperature()
{
	try {

		/** �����ٽ�� m_cs  */
		//EnterCriticalSection(&m_cs);
		IpxCamErr err;
		IpxGenParam::Array *m_param = imperxParamGet();
		if (m_param == nullptr)
		{
			/** �뿪�ٽ�� m_cs */

			return 0;
		}
		/** �뿪�ٽ�� m_cs */
		//LeaveCriticalSection(&m_cs);
		return m_param->GetIntegerValue("CurrentTemperature", &err);
	}
	catch (...) {
		LogUtils::error->error("��������-getCurrentTemperature");
		/** �뿪�ٽ�� m_cs */
		//LeaveCriticalSection(&m_cs);
	}

}
int64_t IpxCamera::getProgFrameTimeAbs()
{
	try {
		/** �����ٽ�� m_cs  */

		IpxCamErr err;
		IpxGenParam::Array *m_param = imperxParamGet();
		if (m_param == nullptr)
		{
			/** �뿪�ٽ�� m_cs */

			return 0;
		}
		/** �뿪�ٽ�� m_cs */

		return m_param->GetIntegerValue("ProgFrameTimeAbs", &err);
	}
	catch (...) {
		LogUtils::error->error("��������-getProgFrameTimeAbs");
		/** �뿪�ٽ�� m_cs */

	}

}

int64_t IpxCamera::getExposureTimeRaw()
{
	try {
		/** �����ٽ�� m_cs  */

		IpxCamErr err;
		IpxGenParam::Array *m_param = imperxParamGet();
		if (m_param == nullptr)
		{
			/** �뿪�ٽ�� m_cs */

			return 0;
		}
		/** �뿪�ٽ�� m_cs */

		return m_param->GetIntegerValue("ExposureTimeRaw", &err);
	}
	catch (...) {
		LogUtils::error->error("��������-getExposureTimeRaw");
		/** �뿪�ٽ�� m_cs */

	}

}

int64_t IpxCamera::getMaxExposure()
{
	try {
		/** �����ٽ�� m_cs  */

		IpxCamErr err;
		IpxGenParam::Array *m_param = imperxParamGet();
		if (m_param == nullptr)
		{
			/** �뿪�ٽ�� m_cs */

			return 0;
		}
		/** �뿪�ٽ�� m_cs */

		return m_param->GetIntegerValue("MaxExposure", &err);
	}
	catch (...) {
		LogUtils::error->error("��������-getMaxExposure");
		/** �뿪�ٽ�� m_cs */

	}

}

int64_t IpxCamera::getGainRaw()
{
	try {
		/** �����ٽ�� m_cs  */

		IpxCamErr err;
		IpxGenParam::Array *m_param = imperxParamGet();
		if (m_param == nullptr)
		{
			/** �����ٽ�� m_cs  */

			return 0;
		}
		/*Limits: Min: 0, Max: 1023*/
		/** �����ٽ�� m_cs  */

		return m_param->GetIntegerValue("GainRaw", &err);
	}
	catch (...) {
		LogUtils::error->error("��������-getGainRaw");
		/** �����ٽ�� m_cs  */

	}

}
int64_t IpxCamera::getTriggerActivation()
{
	try {
		/** �����ٽ�� m_cs  */

		IpxCamErr err;
		IpxGenParam::Array *m_param = imperxParamGet();
		if (m_param == nullptr)
		{
			/** �뿪�ٽ�� m_cs */

			return 0;
		}
		//"RisingEdge": 0 (0x00000000)
		//"FallingEdge" : 1 (0x00000001)
		/** �뿪�ٽ�� m_cs */

		return m_param->GetIntegerValue("TriggerActivation", &err);
	}
	catch (...) {
		LogUtils::error->error("��������-getTriggerActivation");
		/** �뿪�ٽ�� m_cs */

	}

}

int64_t IpxCamera::getTriggerMode()
{
	try {
		/** �����ٽ�� m_cs  */

		IpxCamErr err;
		IpxGenParam::Array *m_param = imperxParamGet();
		//printf("");
		if (m_param == nullptr)
		{
			printf("m_param == nullptr\r\n");

			/** �뿪�ٽ�� m_cs */

			return 0;
		}
		//"Off": 0 (0x00000000) "On": 1 (0x00000001)
		//"Off": 0 (0x00000000) "On": 1 (0x00000001)

		/** �뿪�ٽ�� m_cs */

		return m_param->GetEnumValue("TriggerMode", &err);
		/*int64_t TriggerMode = m_param->GetEnumValue("TriggerMode", &err);
		if (err != IpxCamErr::IPX_CAM_ERR_OK)
		{
			printf("IPX_CAM_ERR_OK == nullptr %d\r\n", err);
			return 0;
		}
		else
		{
			printf("TriggerMode = %d\r\n", TriggerMode);
			return TriggerMode;
		}*/
	}
	catch (...) {
		/** �뿪�ٽ�� m_cs */

	}


}

IpxCam::Stream *IpxCamera::OpenStreamByIndx(IpxCam::Device *aDevice, uint32_t idx)
{
	try {
		/** �����ٽ�� m_cs  */

		IpxCam::Stream *lStream = aDevice->GetStreamByIndex(idx);
		if (lStream == NULL)
		{
			/** �뿪�ٽ�� m_cs */

			return nullptr;
		}
		/** �뿪�ٽ�� m_cs */

		return lStream;
	}
	catch (...) {
		/** �뿪�ٽ�� m_cs */

	}


}

void IpxCamera::CreateDataStreamBuffers(IpxCam::Stream *aDataStream, std::list<IpxCam::Buffer*> *aBufferList)
{
	try {
		/** �����ٽ�� m_cs  */

		//Get the Payload Size
		auto bufSize = aDataStream->GetBufferSize();

		auto minNumBuffers = aDataStream->GetMinNumBuffers();

		for (size_t i = 0; i < minNumBuffers + 1; ++i) {
			// Create new buffer object
			IpxCam::Buffer* hBuffer;
			IpxCamErr err;
			//Allocate the payload memory of the new buffer object
			hBuffer = aDataStream->CreateBuffer(bufSize, nullptr, &err);

			aBufferList->push_back(hBuffer);
		}
		/** �뿪�ٽ�� m_cs */

	}
	catch (...) {
		/** �뿪�ٽ�� m_cs */

	}

}
void IpxCamera::FreeDataStreamBuffers(IpxCam::Stream *aDataStream, std::list<IpxCam::Buffer*> *aBufferList)
{
	try {
		/** �����ٽ�� m_cs  */

		// Go through the buffer list
		std::for_each(std::begin(*aBufferList), std::end(*aBufferList),
			[&aDataStream](IpxCam::Buffer* buffer) {
			aDataStream->RevokeBuffer(buffer);
		});
		//Clear the buffer List
		aBufferList->clear();

		/** �뿪�ٽ�� m_cs */

	}
	catch (...) {

		/** �뿪�ٽ�� m_cs */

	}

}

int IpxCamera::AcquisitionThread(void *org) {

	IpxCamera *obj = reinterpret_cast<IpxCamera*>(org);
	IpxCamErr err;
	while (obj->startAcquisition)
	{
		/** �����ٽ�� m_cs  */
		EnterCriticalSection(&obj->m_cs_stram);
		//IpxCam::Buffer* buffer = lStream->GetBuffer(UINT64_MAX, &err);
		IpxCam::Buffer* buffer = obj->lStream->GetBuffer(2000, &err);

		if ((IPX_CAM_ERR_OK == err) && buffer && buffer->GetHeight() && buffer->GetWidth())
		{
			//�ص�֡����
			obj->_callback(buffer);
			//printf("index = %d\r\n", index);
			obj->lStream->QueueBuffer(buffer);
			//index++;
		}
		else if (GC_ERR_TIMEOUT == err)
		{
			if (!obj->startAcquisition) {
				//printf("error%d\r\n", err);
				/** �뿪�ٽ�� m_cs */
				LeaveCriticalSection(&obj->m_cs_stram);
				break;
			}
			//index_error++;
			//printf("--------------------------------------index_error = %d \r\n", index_error);
		}
		/** �뿪�ٽ�� m_cs */
		LeaveCriticalSection(&obj->m_cs_stram);
		//Sleep(1);
	}
	return 0;
}

/**************************************************
***@author:cui_sh
***@brief:����Ƶ��
***@time:2019-06-17
***@version:
**************************************************/

bool IpxCamera::StartAcquisition(Imperxcallback m_callback)
{
	try
	{

		if (startAcquisition) {

			return false;
		}

		if (lDevice->GetNumStreams() < 1) {

			return false;
		}
		lStream = OpenStreamByIndx(lDevice, 0);
		if (!lStream) {

			return false;
		}
		CreateDataStreamBuffers(lStream, &lBufferList);

		if (m_callback == nullptr) {

			return false;
		}
		_callback = m_callback;
		//��ʼ���ͼ��
		IpxCamErr err;
		arrays = imperxParamGet();
		startAcquisition = true;
		if (arrays)
		{
			err = imperxParamSet("TLParamsLocked", "1");
			if (err != IpxCamErr::IPX_CAM_ERR_OK) {

				return false;
			}
			err = lStream->StartAcquisition();
			if (err != IpxCamErr::IPX_CAM_ERR_OK)
			{

				return false;
			}
			err = imperxParamSet("AcquisitionStart", "AcquisitionStart");
			if (err != IpxCamErr::IPX_CAM_ERR_OK)
			{

				return false;
			}
			int index = 0;
			int index_error = 0;
			//��ȡ֡����
			//printf("��ȡ֡���� \r\n");

			//std::thread t1(AcquisitionThread,this);
			//std::thread t2(AcquisitionThread, this);
			/*std::thread t3(AcquisitionThread, this);
			std::thread t4(AcquisitionThread, this);
			std::thread t5(AcquisitionThread, this)*/;


			//t1.join();
			//t2.join();
			/*t3.join();
			t4.join();
			t5.join();
*/

			while (startAcquisition)
			{
				//IpxCam::Buffer* buffer = lStream->GetBuffer(UINT64_MAX, &err);
				IpxCam::Buffer* buffer = lStream->GetBuffer(2000, &err);
				
				if ((IPX_CAM_ERR_OK == err) && buffer && buffer->GetHeight() && buffer->GetWidth())
				{
					//�ص�֡����
					m_callback(buffer);
					//printf("index = %d\r\n", index);
					lStream->QueueBuffer(buffer);
					//index++;
				}
				else if(GC_ERR_TIMEOUT== err)
				{
					if (!startAcquisition) {
						//printf("error%d\r\n", err);
						break;
					}
					//index_error++;
					//printf("--------------------------------------index_error = %d \r\n", index_error);
				}

				//printf("m_callback\r\n");
				//Sleep(1);
			}





			//printf("AcquisitionStop m_callback\r\n");
			IpxCamErr err = arrays->ExecuteCommand("AcquisitionStop");
			if (err != IPX_CAM_ERR_OK)
			{


				arrays->ExecuteCommand("AcquisitionAbort");
			}
			lStream->StopAcquisition(1);
			arrays->SetIntegerValue("TLParamsLocked", 0);


			lStream->FlushBuffers(IpxCam::Flush_AllDiscard);
			FreeDataStreamBuffers(lStream, &lBufferList);
			lStream->Release();
		}

	}
	catch (...)
	{

	}


	//printf("����\r\n");
	return false;
}

/**************************************************
***@author:�ر���Ƶ��
***@brief:
***@time:
***@version:
**************************************************/

bool IpxCamera::StopAcquisition()
{
	try {
		/** �����ٽ�� m_cs  */

		startAcquisition = false;
		//printf("startAcquisition = false \r\n");
		/** �뿪�ٽ�� m_cs */

		return true;
	}
	catch (...) {
		/** �뿪�ٽ�� m_cs */

	}

}

/**************************************************
***@author:����֡Ƶ
***@brief:
***@time:
***@version:
**************************************************/

bool IpxCamera::setProgFrameTimeAbs(int64_t value)
{
	try {
		/** �����ٽ�� m_cs  */

		int currentFps = 80;//getCurrentFrameRate();
		(currentFps > value) ? value = value : value = currentFps;


		IpxCamErr err = imperxParamSet("ProgFrameTimeAbs", std::to_string((1.0 / value * 1000000)));
		if (err == IPX_CAM_ERR_OK)
		{
			return true;
		}
		/** �뿪�ٽ�� m_cs */

		return false;
	}
	catch (...) {
		/** �뿪�ٽ�� m_cs */

	}

}

/**************************************************
***@author:���û���ʱ��
***@brief:
***@time:
***@version:
**************************************************/
bool IpxCamera::setExposureTimeRaw(int64_t value)
{
	try {
		/** �����ٽ�� m_cs  */

		int maxExposure = getMaxExposure();
		LogUtils::debugger->debug("maxExposure %d value=%d", maxExposure, value);
		(maxExposure > value) ? value = value : value = maxExposure;
		LogUtils::debugger->debug("maxExposurevalue=%d", value);
		IpxCamErr err = imperxParamSet("ExposureTimeRaw", std::to_string(value));
		if (err == IPX_CAM_ERR_OK)
		{

			/** �뿪�ٽ�� m_cs */

			return true;
		}

		/** �뿪�ٽ�� m_cs */

		return false;
	}
	catch (...) {

		/** �뿪�ٽ�� m_cs */

	}


}

/**************************************************
***@author:��������
***@brief:
***@time:
***@version:
**************************************************/
bool IpxCamera::setGainRaw(int64_t value)
{
	try {
		/** �����ٽ�� m_cs  */

		/*Limits: Min: 0, Max: 1023*/
		int GainRaw = 1023;
		(GainRaw > value) ? value = value : value = GainRaw;
		IpxCamErr err = imperxParamSet("GainRaw", std::to_string(value));
		if (err == IPX_CAM_ERR_OK)
		{
			/** �뿪�ٽ�� m_cs */

			return true;
		}
		/** �뿪�ٽ�� m_cs */

		return false;
	}
	catch (...) {
		/** �뿪�ٽ�� m_cs */

	}

}


/**************************************************
***@author:�򿪰�մ��� 0:off 1:on
***@brief:
***@time:
***@version:
**************************************************/
bool IpxCamera::setTriggerMode(int64_t value)
{
	try {
		/** �����ٽ�� m_cs  */

		IpxCamErr err = imperxParamSet("TriggerMode", std::to_string(value));
		if (err == IPX_CAM_ERR_OK)
		{
			/** �뿪�ٽ�� m_cs */

			return true;
		}
		/** �뿪�ٽ�� m_cs */

		return false;
	}
	catch (...) {
		/** �뿪�ٽ�� m_cs */

	}

}

/**************************************************
***@author:���ô�����
***@brief: "RisingEdge": 0 (0x00000000) "FallingEdge": 1 (0x00000001)
***@time:
***@version:
**************************************************/
bool IpxCamera::setTriggerActivation(int64_t value)
{
	try {
		/** �����ٽ�� m_cs  */

		IpxCamErr err = imperxParamSet("TriggerActivation", std::to_string(value));
		if (err == IPX_CAM_ERR_OK)
		{

			/** �뿪�ٽ�� m_cs */

			return true;
		}

		/** �뿪�ٽ�� m_cs */

		return false;
	}
	catch (...) {

		/** �뿪�ٽ�� m_cs */

	}

}