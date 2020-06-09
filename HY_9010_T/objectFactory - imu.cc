#include "objectFactory.h"
namespace demo {

	using v8::Context;
	using v8::Function;
	using v8::FunctionCallbackInfo;
	using v8::FunctionTemplate;
	using v8::Isolate;
	using v8::Local;
	using v8::NewStringType;
	using v8::Number;
	using v8::Object;
	using v8::Persistent;
	using v8::String;
	using v8::Value;
	using v8::Exception;

	//CSerialPort *m_SerialPort=NULL;

	xmlOPer *m_xml;
	CComPtr<IXMLDOMElement> spRootEle;
	Persistent<Function> MyObject::constructor;

	v8::Persistent<v8::Function> MyObject::g_frameDataCallback;

	v8::Persistent<v8::Function> MyObject::g_sysInfoCallback;

	v8::Persistent<v8::Function> MyObject::g_recCmdCallback;
	v8::Persistent<v8::Function> MyObject::g_recGpsCallback;

	bool MyObject::g_sysinfo;

	bool MyObject::g_syscmd;
	bool MyObject::g_sysgps;

	bool MyObject::g_quit=true;
	uv_thread_t MyObject::g_captureThreadId;
	uv_thread_t MyObject::g_sysInfoThreadId;
	uv_thread_t MyObject::g_cmdCallbackThreadId;
	uv_thread_t MyObject::g_gpsCallbackThreadId;
	
	uv_thread_t MyObject::g_saveThreadId;
	uv_async_t MyObject::g_async={0};
	uv_async_t MyObject::g_asyncSysInfo = { 0 };
	uv_async_t MyObject::g_asyncRecCmd = { 0 };
	uv_async_t MyObject::g_asyncRecGps = { 0 };

	CSerialPort *MyObject::m_Serial=NULL;
	CSerialPort *MyObject::m_gpsSerial = NULL;

	v8::Persistent<v8::Function> MyObject::g_logCallback;

	unsigned char* MyObject::g_frameBuffer;
	unsigned char*  MyObject::spectrumBuf;

	cv::Mat MyObject::save_image;
	bool MyObject::m_takePhoto;


	double MyObject::currentArticulation;

	int64_t MyObject::ExposureTime;
	


	IpxCamera *m_IpxCamera;

	#define BUFFER_SIZE 640*480*4 


	#define BUFFER_SPEC 270*2 

	#define BUFFER_QUEUE_SIZE 100
	//��AddOn�У�����һ���ṹ�����첽�����д�������

	//�ռ�ά��ػ���
	cv::Mat MyObject::rgb_mat;
	int  MyObject::r_row;
	int  MyObject::g_row;
	int  MyObject::b_row;
	int  MyObject::spectrum_cols = 10;
	int  MyObject::rgb_mat_width=640;//ͼ��Ŀ����Լ�����ģ�����ԭͼ�Ŀ�
	int  MyObject::current_col=0;
	bool MyObject::is_save=false;
	bool MyObject::is_save_gps = false;
	char MyObject::buf2hex[3];
	std::string MyObject::recvresult="";
	std::string MyObject::prefix="";
	std::queue<std::string> MyObject::rev_cmd;
	std::queue<char> MyObject::gps_data;
	int MyObject::ekf_modal;
	int MyObject::rtk_modal;
	std::string MyObject::EKF_STATUS;
	std::string MyObject::RTK_STATUS;

	std::string MyObject::gps_buf_06;
	std::string MyObject::gps_buf_0E;
	//�������ݶ���
	std::queue<specData>MyObject::specDataT;
	neb::CJsonObject MyObject::sysInfoJson;
	struct FocusElement {
		uv_work_t work;              //libuv
		Persistent<Function> callback;    //javascript callback  <heap>
		int type;
	};
	double MyObject::time_stamp;
	std::ofstream MyObject::gpsFileWrite;



	MyObject::MyObject(){
	

		m_xml = new xmlOPer();
		spRootEle = m_xml->openXmlDoc();
		m_IpxCamera = new IpxCamera();
		//LoadSystemIniParam();
		g_frameBuffer = new unsigned char[BUFFER_SIZE];
		spectrumBuf = new unsigned char[BUFFER_SPEC];
	
		//�������Թ���
		//AllocConsole();
		//freopen("CONOUT$", "w", stdout);
	}

	MyObject::~MyObject() {
		m_IpxCamera = NULL;
		//m_xml = NULL;
		
	}
	double MyObject::get_wall_time()
	{
		#ifdef _WIN32
				return static_cast<double>(clock()) / static_cast<double>(CLOCKS_PER_SEC);
		#else
				struct timeval time;
				if (gettimeofday(&time, NULL))
				{
					return 0;
				}
				return (double)time.tv_sec + (double)time.tv_usec*0.000001;
		#endif
	}

	

	/**************************************************
	***@author:cui_sh
	***@brief:�����û�Ĭ�ϲ���
	***@time:
	***@version:
	**************************************************/
	void MyObject::LoadSystemIniParam()
	{
		//��ȡ�����ļ�
	
		//CComPtr<IXMLDOMNode>SingleNode = m_xml->selectSingleNode(spRootEle, _T("/System/ProgFrameTimeAbs"));
		//key_value m_kv = m_xml->selectSingleNodeKeyValue(SingleNode);
		//std::string value = CT2A(m_kv.xml_value.GetBuffer());
		//std::atoi(value.c_str());//<!--�Զ����豸-->

	

	}


	void MyObject::Init(Isolate* isolate) {
	  //m_SerialPort = NULL;
	  m_takePhoto = false;
	  // ׼�����캯��ģ��
	  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	  tpl->SetClassName(String::NewFromUtf8(isolate, "MyObject", NewStringType::kNormal).ToLocalChecked());
	  tpl->InstanceTemplate()->SetInternalFieldCount(1);

	 
	  
	  NODE_SET_PROTOTYPE_METHOD(tpl, "setLogCallback", setLogCallback);

	  NODE_SET_PROTOTYPE_METHOD(tpl, "callBack", callBack);

	  NODE_SET_PROTOTYPE_METHOD(tpl, "sysInfoCallBack", sysInfoCallBack);//ϵͳ��Ϣ�ص�
      
	  NODE_SET_PROTOTYPE_METHOD(tpl, "RecCmdCallBack", RecCmdCallBack);//STM���ư�ص�

	  NODE_SET_PROTOTYPE_METHOD(tpl, "RecGpsCallBack", RecGpsCallBack);//
	  ///
	  /*��ȡ�豸�б��е����*/
	  NODE_SET_PROTOTYPE_METHOD(tpl, "getCameryList", getCameryList);
	  NODE_SET_PROTOTYPE_METHOD(tpl, "getHdrInfo", getHdrInfo);
	  /*�����*/
	  ///
	  NODE_SET_PROTOTYPE_METHOD(tpl, "openCamery", openCamery);
	  NODE_SET_PROTOTYPE_METHOD(tpl, "closeCamery", closeCamery);
	  //����Ƶ��
	  NODE_SET_PROTOTYPE_METHOD(tpl, "start", start);

	  NODE_SET_PROTOTYPE_METHOD(tpl, "stop", stop);
	  //�л��ռ�ά
	  NODE_SET_PROTOTYPE_METHOD(tpl, "wavelengthSwitch", wavelengthSwitch);
	  //
	  NODE_SET_PROTOTYPE_METHOD(tpl, "start_rec", start_rec);

	  NODE_SET_PROTOTYPE_METHOD(tpl, "stop_rec", stop_rec);
	  //��������
	  NODE_SET_PROTOTYPE_METHOD(tpl, "setParamImperx", setParamImperx);
	  //������ȡ
	  NODE_SET_PROTOTYPE_METHOD(tpl, "getParamImperx", getParamImperx);

	  ///���ڲ���
	  NODE_SET_PROTOTYPE_METHOD(tpl, "getSerialPortList", getSerialPortList);
	  NODE_SET_PROTOTYPE_METHOD(tpl, "openSerialPort", openSerialPort);
	  NODE_SET_PROTOTYPE_METHOD(tpl, "closeSerialPort", closeSerialPort);
	  NODE_SET_PROTOTYPE_METHOD(tpl, "openCmdSerialPort", openCmdSerialPort);
	  NODE_SET_PROTOTYPE_METHOD(tpl, "closeCmdSerialPort", closeCmdSerialPort);
	  NODE_SET_PROTOTYPE_METHOD(tpl, "UART_SEND", UART_SEND);

	  






	  NODE_SET_PROTOTYPE_METHOD(tpl, "takePhoto", takePhoto);
	  
	  
	  ///

	  Local<Context> context = isolate->GetCurrentContext();
	  constructor.Reset(isolate, tpl->GetFunction(context).ToLocalChecked());
	  
	  uv_async_init(uv_default_loop(), &g_async, eventCallback);
	  //ϵͳ״̬��Ϣ�ص�����
	  uv_async_init(uv_default_loop(), &g_asyncSysInfo, eventCallbackSysInfo);
	  //����ָ����ջص����� // ���ѽ��֪ͨ��ǰ��
	  uv_async_init(uv_default_loop(), &g_asyncRecCmd, eventCallbackRecCmd);

	  uv_async_init(uv_default_loop(), &g_asyncRecGps, eventCallbackRecGps);
	}

	void MyObject::New(const FunctionCallbackInfo<Value>& args) {
	  Isolate* isolate = args.GetIsolate();
	  Local<Context> context = isolate->GetCurrentContext();

	  if (args.IsConstructCall()) {
		// ���캯��һ�����ã�`new MyObject(...)`
		double value = args[0]->IsUndefined() ? 0 : args[0]->NumberValue(context).FromMaybe(0);
		MyObject* obj = new MyObject();
		obj->Wrap(args.This());
		args.GetReturnValue().Set(args.This());
	  } else {
		// ����ͨ���� `MyObject(...)` һ�����ã�תΪ������á�
		const int argc = 1;
		Local<Value> argv[argc] = { args[0] };
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
		args.GetReturnValue().Set(instance);
	  }
	}

	void MyObject::NewInstance(const FunctionCallbackInfo<Value>& args) {
	  Isolate* isolate = args.GetIsolate();

	  const unsigned argc = 1;
	  Local<Value> argv[argc] = { args[0] };
	  Local<Function> cons = Local<Function>::New(isolate, constructor);
	  Local<Context> context = isolate->GetCurrentContext();
	  Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();

	  args.GetReturnValue().Set(instance);
	}

	
	void MyObject::callBack(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		/*Isolate* isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cb = Local<Function>::Cast(args[0]);

		const unsigned argc = 1;
		int index = 0;
	
		std::string imperx_info = "test"+std::to_string(index++);
		Local<Value> argv[argc] = {
				String::NewFromUtf8(isolate,
				imperx_info.c_str(),
				NewStringType::kNormal).ToLocalChecked()
		};
		cb->Call(context, Null(isolate), argc, argv).ToLocalChecked();*/

		Isolate* isolate = args.GetIsolate();

		// �����һ���������Ǻ��������׳�����
		if (!args[0]->IsFunction())
		{
			v8::MaybeLocal<String> message = v8::String::NewFromUtf8(isolate, EncodeUtils::toUtf8String(L"The first argument must be a function!").c_str());
			
			
			/*isolate->ThrowException(v8::Exception::TypeError(
				v8::String::NewFromUtf8(isolate, EncodeUtils::toUtf8String(L"The first argument must be a function!").c_str())));*/
			isolate->ThrowException(v8::Exception::TypeError(message.ToLocalChecked()));
			return;
		}
		
		// ��ȡ�ص���������������ȫ�ֱ���
		Local<Function> frameDataCallback = v8::Local<v8::Function>::Cast(args[0]);
		g_frameDataCallback.Reset(isolate, frameDataCallback);

	}
	
	void MyObject::sysInfoCallBack(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		if (!args[0]->IsFunction())
		{
			v8::MaybeLocal<String> message = v8::String::NewFromUtf8(isolate, EncodeUtils::toUtf8String(L"The first argument must be a function!").c_str());
			isolate->ThrowException(v8::Exception::TypeError(
				message.ToLocalChecked()));
			return;
		}

		// ��ȡ�ص���������������ȫ�ֱ���
		Local<Function> Callback = v8::Local<v8::Function>::Cast(args[0]);
		g_sysInfoCallback.Reset(isolate, Callback);
	}


	void MyObject::RecCmdCallBack(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		if (!args[0]->IsFunction())
		{
			v8::MaybeLocal<String> message = v8::String::NewFromUtf8(isolate, EncodeUtils::toUtf8String(L"The first argument must be a function!").c_str());
			isolate->ThrowException(v8::Exception::TypeError(
				message.ToLocalChecked()));
			return;
		}

		// ��ȡ�ص���������������ȫ�ֱ���
		Local<Function> Callback = v8::Local<v8::Function>::Cast(args[0]);
		g_recCmdCallback.Reset(isolate, Callback);
		//double time_stamp = get_wall_time();
		//
		//while (true)
		//{
		//	double _time = get_wall_time();
		//	double microsecond = _time - time_stamp;//ms
		//	//printf("microsecond = %f\r\n", microsecond);
		//	if (microsecond > 2)
		//	{
		//		printf("microsecond = %f\r\n", microsecond);
		//		time_stamp = _time;
		//		//break;
		//	}
		//	Sleep(1);
		//}
		//double m_fps = 1.f / (_time - time_stamp);
		



		//����һ�����������Ϣִ���߳�
		/*g_syscmd = false;
		uv_thread_create(&g_cmdCallbackThreadId, sysCmdHandle, nullptr);*/
	}

	void MyObject::RecGpsCallBack(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		if (!args[0]->IsFunction())
		{
			v8::MaybeLocal<String> message = v8::String::NewFromUtf8(isolate, EncodeUtils::toUtf8String(L"The first argument must be a function!").c_str());
			isolate->ThrowException(v8::Exception::TypeError(
				message.ToLocalChecked()));
			return;
		}

		// ��ȡ�ص���������������ȫ�ֱ���
		Local<Function> Callback = v8::Local<v8::Function>::Cast(args[0]);
		g_recGpsCallback.Reset(isolate, Callback);

		//����һ�����������Ϣִ���߳�
		//g_syscmd = false;
		//uv_thread_create(&g_gpsCallbackThreadId, sysGpsHandle, nullptr);
	}

	void MyObject::saveData(void *arg)
	{
		int lines = 0;
		//CTime t = CTime::GetCurrentTime();
		/*char buf[128];
		sprintf(buf,"./data/%04d%02d%02d%02d%02d%02d", t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());*/
		//����ļ�·���Ƿ����
		//std::string prefix = buf;
		std::string spe_path = prefix + ".spe";
		EncodeUtils::createDirectory(spe_path.c_str());
		//д��spe�ļ����ļ�
		std::ofstream spefile(spe_path.c_str(), std::fstream::out | std::fstream::binary);
		
		
		
		int Width;
		int Height;
		while (is_save){
			/*if (specDataT.empty())
			{
				Sleep(100);
				continue;
			}*/
			if (specDataT.size() >= BUFFER_QUEUE_SIZE)
			{
				for (int index = 0; index < BUFFER_QUEUE_SIZE; index++)
				{
					specData m_data = specDataT.front();
					Width = m_data.Width;
					Height = m_data.Height;
					spefile.write((char*)m_data.imagedata, m_data.bufSize);
					delete[] m_data.imagedata;
					specDataT.pop();
					lines++;
				}
			}
			else
			{
				Sleep(100);
			}
			//printf("saveData:01  %d\r\n", specDataT.size());

			Sleep(1);
		}

		while (!specDataT.empty()) {
			
			specData m_data = specDataT.front();
			Width = m_data.Width;
			Height = m_data.Height;
			spefile.write((char*)m_data.imagedata, m_data.bufSize);
			delete[] m_data.imagedata;
			specDataT.pop();
			lines++;
			//printf("saveData:02  %d\r\n", specDataT.size());
			Sleep(1);
		}
		spefile.flush();
		spefile.close();
		is_save_gps = false;
		

		//д��hdr�ļ�
		//�ӵ�ǰ�ļ�Ŀ¼�¶�ȡhdr�ļ�
		std::string hdr_path = prefix + ".hdr";
		EncodeUtils::createDirectory(hdr_path.c_str());

		std::ofstream hdrfile(hdr_path, std::fstream::out);

		
		std::ifstream red_hdrfile("./template.hdr", std::fstream::in);
		char buffer[1024];
		while (red_hdrfile.getline(buffer, 1024)) {
			if (strstr(buffer, "lines"))
			{
				hdrfile << "lines = " << std::to_string(lines) << std::endl;
			}
			else if (strstr(buffer, "samples"))
			{
				hdrfile << "samples = " << std::to_string(Width) << std::endl;
			}
			else if (strstr(buffer, "bands"))
			{
				hdrfile << "bands = " << std::to_string(Height) << std::endl;
			}
			else
			{
				hdrfile << buffer << std::endl;
			}
			
			Sleep(1);
		}
		red_hdrfile.close();
		//�ر�gps�ļ�д����
		if (m_gpsSerial != NULL)
		{
			gpsFileWrite.flush();
			gpsFileWrite.close();
			//printf("gpsFileWrite close\r\n");
		}
	}

	void MyObject::capture(void *arg)
	{
		////// ��ȡ Frame ��ֱ���˳�ϵͳ
		//cv::Mat img = cv::imread("C:\\Users\\cui_sh\\Pictures\\Screenshots\\test.png", 1);

		////cv::resize(img, img, cv::Size(640, 480));
		////cv::Mat converted;
		////cv::cvtColor(img, converted, cv::COLOR_BGR2RGBA);

		//while (!g_quit)
		//{
		//	g_asyncData.type = L"Frame";
		//	g_asyncData.message = L"Frame captured!";
		//	g_async.data = &g_asyncData;
		//	img_base64 = OpencvUtils::Mat2Base64(img,"png");
		//	//memcpy(g_frameBuffer, converted.data, BUFFER_SIZE);
		//	uv_async_send(&g_async);
		//	Sleep(12);
		//}
		//img.release();
		////converted.release();

		////cvNamedWindow("show", 0);

		//������Ƶ��
		bool result = m_IpxCamera->StartAcquisition(cameryCallback);
	}
	/*
	*ϵͳ���յ�ָ�����ִ�й���
	*/
	void MyObject::sysCmdHandle(void *arg)
	{
		/*std::string cmd = "";
		int value = 0;*/
		int count_0 = 0;
		int count_1 = 1;
		while (!g_syscmd)
		{
			if (!rev_cmd.empty()) {
				g_recCmdAsyncData.type = L"SysCmd";
				g_recCmdAsyncData.message = L"SysCmd Callback!";
				g_asyncRecCmd.data = &g_recCmdAsyncData;
				/*cmd = rev_cmd.front();
				rev_cmd.pop();
				value = EncodeUtils::calcLSB(cmd);*/
				//printf("����ָ�%s result��%d %d\r\n", cmd.c_str(), value, rev_cmd.size());
				//����ָ��
				/*char * data = EncodeUtils::getFormatStringForMon(100, "AA12");
				printf("����ָ�%s", data);
				delete data;*/
				//printf("����ָ�\r\n");
				uv_async_send(&g_asyncRecCmd);
				Sleep(10);
				if (count_0 < 120)
				{
					count_0++;
				}
				//printf("count_0=%d\r\n", count_0);
			}
			else
			{

				Sleep(400);
				if (count_1 < 3)
				{
					count_1++;
				}
				//printf("count_1=%d\r\n", count_1);
			}

			if (count_0 >= 120)
			{
				count_1 = 0;
				count_0 = 0;
				//�㲥
				//printf("count_0�㲥\r\n");
				//����״̬
				broadcast();
			}
			if (count_1 >= 3)
			{
				count_1 = 0;
				count_0 = 0;
				broadcast();
				//printf("count_1�㲥\r\n");
			}
		}
	}
	/*
	*ϵͳ���յ�ָ�����ִ�й���
	*/
	void MyObject::sysGpsHandle(void *arg)
	{
		time_stamp = get_wall_time();
		while (!g_sysgps)
		{
			if (!gps_data.empty()) {
				g_recGpsAsyncData.type = L"SysGps";
				g_recGpsAsyncData.message = L"SysGps Callback!";
				g_asyncRecGps.data = &g_recGpsAsyncData;
				
				uv_async_send(&g_asyncRecGps);
				Sleep(1);
			}
			else
			{
				Sleep(200);
			}
			
		}
	}

	void MyObject::broadcast()
	{
		/*if (m_Serial == NULL)
			return;*/
		char * data = EncodeUtils::getFormatStringForMon(1, "AA23");
		m_Serial->WriteDataHex(data, strlen(data));
		delete data;

		//������״̬ 
		if (m_IpxCamera->lDevice != nullptr)
		{
			data = EncodeUtils::getFormatStringForMon(0, "AA10");
			m_Serial->WriteDataHex(data, strlen(data));
			delete data;
		}
		else {
			data = EncodeUtils::getFormatStringForMon(1, "AA10");
			m_Serial->WriteDataHex(data, strlen(data));
			delete data;
		}
		//����ʱ�䣨ms��
		if (m_IpxCamera->lDevice != nullptr) {
			int m_ExposureTime = double(ExposureTime / 10);
			data = EncodeUtils::getFormatStringForMon(m_ExposureTime, "AA11");
			m_Serial->WriteDataHex(data, strlen(data));
			delete data;
		}
		//ͼ��ֹͣ
		if (g_quit && !is_save) {
			data = EncodeUtils::getFormatStringForMon(2, "AA12");
			m_Serial->WriteDataHex(data, strlen(data));
			delete data;
		}
		else if (!g_quit && !is_save) {
			//ͼ��Ԥ��
			data = EncodeUtils::getFormatStringForMon(1, "AA12");
			m_Serial->WriteDataHex(data, strlen(data));
			delete data;
		}
		if (!g_quit && is_save) {
			//ͼ��ɼ�
			data = EncodeUtils::getFormatStringForMon(2, "AA12");
			m_Serial->WriteDataHex(data, strlen(data));
			delete data;
		}
		//����
		data = EncodeUtils::getFormatStringForMon(r_row, "AA13");
		m_Serial->WriteDataHex(data, strlen(data));
		delete data;
		//�ռ�ά
		data = EncodeUtils::getFormatStringForMon(spectrum_cols, "AA14");
		m_Serial->WriteDataHex(data, strlen(data));
		delete data;
	}
	void MyObject::sysInfoHandle(void *arg)
	{
		while (!g_sysinfo)
		{
			g_sysInfoAsyncData.type = L"SysInfo";
			g_sysInfoAsyncData.message = L"SysInfo Callback!";
			g_asyncSysInfo.data = &g_sysInfoAsyncData;
			//��ȡ�������Ϣ����json�У�֪ͨ�ص���������������ʾ
			/*double ExposureTime = PHY::camery2500::getExposureTime();
			double FrameRate = PHY::camery2500::getFrameRate();
			double Temperature = PHY::camery2500::getTemperature();*/
			sysInfoJson.Clear();
			if (m_IpxCamera->lDevice != nullptr)
			{
				int64_t FrameRate = m_IpxCamera->getCurrentFrameRate();
				sysInfoJson.Add("FrameRate", FrameRate);
				ExposureTime = m_IpxCamera->getExposureTimeRaw();
				sysInfoJson.Add("ExposureTime", double (ExposureTime/1000000.0));
				int64_t Temperature = m_IpxCamera->getCurrentTemperature();
				sysInfoJson.Add("Temperature", Temperature);
			}
		

			uv_async_send(&g_asyncSysInfo);
			Sleep(1000);
		}

	}

	/**************************************************
	***@author:cui_sh
	***@brief:��ʼ���ݲɼ�
	***@time:
	***@version:
	**************************************************/
	void MyObject::start_rec(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate*  isolate = v8::Isolate::GetCurrent();
		if (args.Length() < 2) {
			// �׳�һ�����󲢴��ص� JavaScript��
			std::string error = "invalid parameter";
			error = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(error));
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					error.c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}

		if (!args[0]->IsString())
		{
			std::string error = "Wrong parameter type";
			error = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(error));
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					error.c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}

		if (!args[1]->IsFunction()) {			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! invalid parameter[fun]").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}

		if (m_IpxCamera->lDevice == nullptr)
		{
			std::string error = "Device is not open";
			error = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(error));
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					error.c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}
		if (!m_IpxCamera->startAcquisition)
		{
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					EncodeUtils::toUtf8String(L"error! stream is open").c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
			//����Ƶ��
		}
		v8::String::Utf8Value param_type(args[0]->ToString());
		std::string data_d = std::string(*param_type);//ϵͳ��ʹ��������Ϣ
		//��������
		data_d = EncodeUtils::WStringToString(EncodeUtils::Utf8ToUnicode(data_d));

		//data_d = EncodeUtils::StringToWString(data_d);
		//std::wstring ww = EncodeUtils::Utf8ToUnicode(data_d);;
		CTime t = CTime::GetCurrentTime();
		char buf[128];
		
		if (data_d.compare("") == 0 || data_d.length() == 0)
		{
			sprintf(buf, "./data/%04d%02d%02d%02d%02d%02d/%04d%02d%02d%02d%02d%02d", t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond(), t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
		}
		else
		{
			sprintf(buf, "%s/%04d%02d%02d%02d%02d%02d/%04d%02d%02d%02d%02d%02d", data_d.c_str(), t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond(),t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
		}
		
		//��ʼ���ݴ洢
		prefix = buf;
		//printf("prefix = %s", prefix.c_str());
		//��gps�ļ���
		//printf("m_gpsSerial \r\n");
		if (m_gpsSerial != NULL)
		{
			
			std::string path = prefix + ".gps";
			EncodeUtils::createDirectory(path);
			gpsFileWrite.open(path.c_str(), std::fstream::out | std::fstream::binary);
			/*if (gpsFileWrite.is_open())
			{
				printf("success: \r\n", path.c_str());
			}
			printf("%s\r\n", path.c_str());*/
		}
		is_save_gps = true;
		is_save = true;
		//�����ⴥ��ָ��
		/*if (m_Serial != NULL) {
			m_Serial->WriteDataHex("AA36000000003655",strlen("AA36000000003655"));
		}*/
		uv_thread_create(&g_saveThreadId, saveData, nullptr);

		log(isolate, L"start_rec");
		
		v8::Local<v8::Function>::Cast(args[1])->Call(isolate->GetCurrentContext(), v8::Null(isolate), 0, NULL);
		//�ص�����
	}
	/**************************************************
	***@author:cui_sh
	***@brief:ֹͣ���ݲɼ�
	***@time:
	***@version:
	**************************************************/
	void MyObject::stop_rec(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate*  isolate = v8::Isolate::GetCurrent();
		
		is_save = false;
		uv_thread_join(&g_saveThreadId);

		log(isolate, L"stop_rec");
		//�ص�����

	}
	/**************************************************
	***@author:cui_sh
	***@brief:��ָ���Ĵ����Ϸ�����
	***@time:
	***@version:
	**************************************************/
	void MyObject::UART_SEND(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		if (args.Length() < 2)
		{
			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! need input camery index").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}
		if (!args[0]->IsString()) {			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! invalid params").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}
		if (!args[1]->IsFunction()) {			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! invalid params").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}
		v8::String::Utf8Value param1(args[0]->ToString());
		std::string data_d = std::string(*param1);
		if (m_Serial != NULL) {
			m_Serial->WriteDataHex((char*)data_d.c_str(), data_d.length());
		}
		v8::Local<v8::Function>::Cast(args[1])->Call(isolate->GetCurrentContext(), v8::Null(isolate), 0, NULL);
	}
	void MyObject::start(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate*  isolate = v8::Isolate::GetCurrent();
		if (m_IpxCamera->lDevice == nullptr)
		{
			std::string error = "Device is not open";
			error = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(error));
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					error.c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}
		if (m_IpxCamera->startAcquisition)
		{
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					EncodeUtils::toUtf8String(L"error! stream is open").c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}
		/*if (args.Length() < 1)
		{
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					EncodeUtils::toUtf8String(L"error! need input camery index").c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}
		if (!args[0]->IsNumber()) {
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					EncodeUtils::toUtf8String(L"error! invalid params").c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;

		}*/
	
		//�жϵ�ǰ��Ƶ���Ƿ��Ѿ���

		
		r_row= 250;
		g_row= 130;
		b_row= 100;

		current_col = 0;
		g_quit = false;
		
		// �����̣߳����߳��ڴ���ɼ���ʶ�𣬲������ս���ַ���ǰ��չʾ
		uv_thread_create(&g_captureThreadId, capture, nullptr);

		log(isolate, L"start");
		
		v8::Local<v8::Function>::Cast(args[0])->Call(isolate->GetCurrentContext(), v8::Null(isolate), 0, NULL);
	}
	void MyObject::stop(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate*  isolate = v8::Isolate::GetCurrent();
		if (m_IpxCamera->lDevice == nullptr)
		{
			std::string error = "Device is not open";
			error = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(error));
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					error.c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}
		if (!m_IpxCamera->startAcquisition)
		{
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					EncodeUtils::toUtf8String(L"error! stream is close").c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}

		if (is_save)
		{
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					EncodeUtils::toUtf8String(L"error! Data acquisition").c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}

		g_quit = true;
		bool result = m_IpxCamera->StopAcquisition();

		uv_thread_join(&g_captureThreadId);

		log(isolate, L"stop");
	}

	void MyObject::eventCallback(uv_async_t* handle)
	{
		Isolate*  isolate = v8::Isolate::GetCurrent();
		AsyncData *asyncData = reinterpret_cast<AsyncData*>(handle->data);
		//printf("%s\r\n", asyncData->type.c_str());
		if (asyncData->type == L"Error")
		{
			log(isolate, asyncData->message.c_str());
		}
		else if (asyncData->type == L"Frame")
		{
			

			v8::Local<v8::SharedArrayBuffer>  buffer = v8::SharedArrayBuffer::New(isolate, g_frameBuffer, BUFFER_SIZE);
			v8::Local<v8::Uint8ClampedArray> buffdata = v8::Uint8ClampedArray::New(buffer, 0, BUFFER_SIZE);
			
			//v8::Local<v8::SharedArrayBuffer>  spbuffer = v8::SharedArrayBuffer::New(isolate, spectrumBuf, BUFFER_SPEC);
			//v8::Local<v8::Uint16Array> spbuffdata = v8::Uint16Array::New(buffer,0, 270);

			v8::Local<v8::SharedArrayBuffer> spbuffer = v8::SharedArrayBuffer::New(isolate, spectrumBuf, BUFFER_SPEC);
			v8::Local<v8::Uint16Array> spbuffdata = v8::Uint16Array::New(spbuffer, 0, 270);
			
			//v8::Local<v8::SharedArrayBuffer>  buffermi = v8::SharedArrayBuffer::New(isolate, g_miframeBuffer, BUFFER_MI_SIZE);
			//v8::Local<v8::Uint8ClampedArray> buffdatami = v8::Uint8ClampedArray::New(buffermi, 0, BUFFER_MI_SIZE);


			v8::Local<v8::Value> argv[2] = { buffdata, spbuffdata };

			v8::Local<v8::Function> cb = v8::Local<v8::Function>::New(isolate, g_frameDataCallback);
			//cb->Call(v8::Null(isolate), 2, argv);
			//v8::Local<v8::Context> context = v8::Context::New(isolate)
			cb->Call(isolate->GetCurrentContext(),v8::Null(isolate),2, argv);

			//const unsigned argc = 1;

			//Local<Value> argv[argc] = {
			//String::NewFromUtf8(isolate,
			//					img_base64.c_str(),
			//					NewStringType::kNormal).ToLocalChecked() };
			//v8::Local<v8::Function> cb = v8::Local<v8::Function>::New(isolate, g_frameDataCallback);
			//cb->Call(Null(isolate), argc, argv);
			

			//v8::Local<v8::Function> cb = v8::Local<v8::Function>::New(isolate, g_frameDataCallback);
			//cb->Call(v8::Null(isolate), 0, NULL);
			
			
		}
		
	}

	void MyObject::eventCallbackRecCmd(uv_async_t* handle)
	{
		//g_asyncRecCmd
		Isolate*  isolate = v8::Isolate::GetCurrent();
		AsyncData *asyncData = reinterpret_cast<AsyncData*>(handle->data);

		if (asyncData->type == L"Error")
		{
			log(isolate, asyncData->message.c_str());
		}
		else if (asyncData->type == L"SysCmd")
		{
			std::string cmd = rev_cmd.front();
			
			int value = EncodeUtils::calcLSB(cmd);
			rev_cmd.pop();
			//���ݻص� json��ʽ����
			//sysInfoJson.ToString();
			v8::Local<v8::Function> cb = v8::Local<v8::Function>::New(isolate, g_recCmdCallback);

			const unsigned argc = 2;

			cmd = cmd.substr(0, 4);
			Local<Value> argv[argc] ={
					String::NewFromUtf8(isolate,cmd.c_str(),NewStringType::kNormal).ToLocalChecked(),
					Number::New(isolate,value)
			};


			cb->Call(isolate->GetCurrentContext(), v8::Null(isolate), argc, argv);
			
		}
	}


	void MyObject::eventCallbackRecGps(uv_async_t* handle)
	{
		//g_asyncRecCmd
		Isolate*  isolate = v8::Isolate::GetCurrent();
		AsyncData *asyncData = reinterpret_cast<AsyncData*>(handle->data);

		if (asyncData->type == L"Error")
		{
			log(isolate, asyncData->message.c_str());
		}
		else if (asyncData->type == L"SysGps")
		{
			//
			char buffer_tohex[3];
			char data = gps_data.front();
			gps_data.pop();
			sprintf(buffer_tohex, "%02X", (BYTE)data);

			ID_06_ANY(std::string(buffer_tohex));
			ID_0E_ANY(std::string(buffer_tohex));
			double _time = get_wall_time();
			//double m_fps = 1.f / (_time - time_stamp);
			double microsecond = _time - time_stamp;
			if (microsecond>=1.5)
			{
				//�㲥����
				v8::Local<v8::Function> cb = v8::Local<v8::Function>::New(isolate, g_recGpsCallback);

				const unsigned argc = 2;

				Local<Value> argv[argc] = {
						String::NewFromUtf8(isolate,EKF_STATUS.c_str(),NewStringType::kNormal).ToLocalChecked(),
						String::NewFromUtf8(isolate,RTK_STATUS.c_str(),NewStringType::kNormal).ToLocalChecked(),
				};


				cb->Call(isolate->GetCurrentContext(), v8::Null(isolate), argc, argv);



				time_stamp = _time;
			}
			

			//��Ƶ�ʿ��Ƶ�1500���� Ȼ����лص�
			//cb->Call(isolate->GetCurrentContext(), v8::Null(isolate), argc, argv);

		}
	}
	/*���� IMU״̬*/
	void MyObject::ID_06_ANY(std::string data)
	{
		gps_buf_06.append(data);//�ѵ�ǰgps���ݷ����ڴ�
		if (gps_buf_06.length() >= GPS_ANY_SIZE) {
			int pos = gps_buf_06.find("FF5A06");
			if (pos != -1)
			{
				/**************************************************
				***@brief:�õ�ǰ�ڴ��е�ֵ�� FF 5A 08��ͷ����ʽ����
				**************************************************/
				gps_buf_06 = gps_buf_06.substr(pos, gps_buf_06.length());
				/**************************************************
				***@brief:���� 33 FF 5A
				**************************************************/
				pos = gps_buf_06.find("33FF5A");
				if (pos != -1) {
					//�ҵ���һ������������
					gps_buf_06 = gps_buf_06.substr(0, pos + 2);
					/**************************************************
					***@brief:��ȡ��Ӧ״̬λ����
					************************************************** /
						// ת int ��   formatHexVale2Int()
						// ת float��  ConvertFloat();
						// ת double�� HexToDouble();
						//ȡ��״̬λ�е�

						///////////////////////////////////////////////�ص�֪ͨ///////////////////////////////////////////////
						/*
							0 SBG_ECOM_SOL_MODE_UNINITIALIZED 					The Kalman filter is not initialized and the returned data are all invalid.
							1 SBG_ECOM_SOL_MODE_VERTICAL_GYRO 					The Kalman filter only rely on a vertical reference to compute roll and pitch angles. Heading and navigation data drift freely.
							2 SBG_ECOM_SOL_MODE_AHRS 							A heading reference is available, the Kalman filter provides full orientation but navigation data drift freely.
							3 SBG_ECOM_SOL_MODE_NAV_VELOCITY 					The Kalman filter computes orientation and velocity. Position is freely integrated from velocity estimation.
							4 SBG_ECOM_SOL_MODE_NAV_POSITION 					Nominal mode, the Kalman filter computes all parameters (attitude, velocity, position). Absolute position is provi
						*/

					ekf_modal = EncodeUtils::formatHexVale2Int(gps_buf_06.substr(gps_buf_06.length() - 14, 8));
					//formatHexVale2Int(gps_buf_06.substr(gps_buf_06.length() - 14, 8));
					ekf_modal = ekf_modal & 0x0007;//4
					//printf("ekf_modal=%d\r\n",ekf_modal);
					if (ekf_modal == 0)
					{
						EKF_STATUS = "UNINITIALIZED";
						//RTK_STATUS="";
					}
					else if (ekf_modal == 1)
					{
						EKF_STATUS = "VERTICAL_GYRO";
					}
					else if (ekf_modal == 2)
					{
						EKF_STATUS = "AHRS";
					}
					else if (ekf_modal == 3)
					{
						EKF_STATUS = "NAV_VELOCITY";
					}
					else if (ekf_modal == 4)
					{
						EKF_STATUS = "NAV_POSITION";
					}
					///////////////////////////////////////////////�ص�֪ͨ///////////////////////////////////////////////
					gps_buf_06 = "";
				}
			}
			gps_buf_06 = "";
		}
	}
	/*���� RTK״̬*/
	void MyObject::ID_0E_ANY(std::string data)
	{
		gps_buf_0E.append(data);//�ѵ�ǰgps���ݷ����ڴ�
		if (gps_buf_0E.length() >= GPS_ANY_SIZE)
		{

			//���Ҳ����������Ϣ
			//��ȡ��������
			int pos = gps_buf_0E.find("FF5A0E");
			if (pos != -1)
			{

				/**************************************************
				***@brief:�õ�ǰ�ڴ��е�ֵ�� FF 5A 08��ͷ����ʽ����
				**************************************************/
				gps_buf_0E = gps_buf_0E.substr(pos, gps_buf_0E.length());

				/**************************************************
				***@brief:���� 33 FF 5A
				**************************************************/
				pos = gps_buf_0E.find("33FF5A");
				if (pos != -1)
				{
					//�ҵ���һ������������
					gps_buf_0E = gps_buf_0E.substr(0, pos + 2);
					rtk_modal = EncodeUtils::formatHexVale2Int(gps_buf_0E.substr(20, 8));
					//formatHexVale2Int(gps_buf_0E.substr(20, 8));
					rtk_modal >>= 6;
					rtk_modal &= 0x001F;
					/**************************************************
					***@brief:��ȡ��Ӧ״̬λ����
					**************************************************/
					// ת int ��   formatHexVale2Int()
					// ת float��  ConvertFloat();
					// ת double�� HexToDouble();
					//ȡ��״̬λ�е�

					if (rtk_modal == 0)
					{
						RTK_STATUS = "NO_SOLUTION";

					}
					else if (rtk_modal == 1)
					{
						RTK_STATUS = "UNKNOWN_TYPE";
					}
					else if (rtk_modal == 2)
					{
						RTK_STATUS = "SINGLE";
					}
					else if (rtk_modal == 3)
					{
						RTK_STATUS = "PSRDIFF";
					}
					else if (rtk_modal == 4)
					{
						RTK_STATUS = "SBAS SBAS";
					}
					else if (rtk_modal == 5)
					{
						RTK_STATUS = "OMNISTAR";
					}
					else if (rtk_modal == 6)
					{
						RTK_STATUS = "RTK_FLOAT";
					}
					else if (rtk_modal == 7)
					{
						RTK_STATUS = "RTK_INT";
					}
					else if (rtk_modal == 8)
					{
						RTK_STATUS = "PPP_FLOAT";
					}
					else if (rtk_modal == 9)
					{
						RTK_STATUS = "PPP_INT";
					}
					else if (rtk_modal == 10)
					{
						RTK_STATUS = "FIXED";
					}
					///////////////////////////////////////////////�ص�֪ͨ///////////////////////////////////////////////
					gps_buf_0E = "";
				}
			}
			gps_buf_0E = "";
		}
	}

	void MyObject::eventCallbackSysInfo(uv_async_t* handle)
	{
		Isolate*  isolate = v8::Isolate::GetCurrent();
		AsyncData *asyncData = reinterpret_cast<AsyncData*>(handle->data);
		if (asyncData->type == L"Error")
		{
			log(isolate, asyncData->message.c_str());
		}
		else if (asyncData->type == L"SysInfo")
		{
			//���ݻص� json��ʽ����
			//sysInfoJson.ToString();
			v8::Local<v8::Function> cb = v8::Local<v8::Function>::New(isolate, g_sysInfoCallback);

			const unsigned argc = 1;


			Local<Value> argv[argc] = {
					String::NewFromUtf8(isolate,
					sysInfoJson.ToString().c_str(),
					NewStringType::kNormal).ToLocalChecked()
			};


			cb->Call(isolate->GetCurrentContext(),v8::Null(isolate), 1, argv);

		}
	}

	void MyObject::setLogCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();

		// �����һ���������Ǻ��������׳�����
		if (!args[0]->IsFunction())
		{
			v8::MaybeLocal<String> message = v8::String::NewFromUtf8(isolate, EncodeUtils::toUtf8String(L"The first argument must be a function!").c_str());
			isolate->ThrowException(v8::Exception::TypeError(
				message.ToLocalChecked()));
			return;
		}
		
		// ��ȡ�ص���������������ȫ�ֱ���
		auto logCallback = v8::Local<v8::Function>::Cast(args[0]);
		g_logCallback.Reset(isolate, logCallback);

		log(isolate, L"Set log callback success!");
	}

	void MyObject::log(v8::Isolate* isolate, const wchar_t * const message)
	{
		std::string messageUtf8 = EncodeUtils::toUtf8String(message);
		v8::MaybeLocal<v8::String> argv = v8::String::NewFromUtf8(isolate, messageUtf8.c_str());
		v8::Local<v8::Value> value = argv.ToLocalChecked();
		v8::Local<v8::Function> logCallback = v8::Local<v8::Function>::New(isolate, g_logCallback);
		logCallback->Call(isolate->GetCurrentContext(),v8::Null(isolate), 1, &value);
	}






	/**************************************************
	***@author:cui_sh
	***@brief:������
	***@time:
	***@version:
	**************************************************/
	void MyObject::getCameryList(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		std::string imperx_info = m_IpxCamera->queryIpxCam();
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, imperx_info.c_str(), NewStringType::kNormal).ToLocalChecked());
	}
	/**************************************************
	***@author:cui_sh
	***@brief:��ȡ�߹���ͷ�ļ�
	***@time:
	***@version:
	**************************************************/
	void MyObject::getHdrInfo(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		//��ȡ����

		if (args.Length() < 1) {
			// �׳�һ�����󲢴��ص� JavaScript��
			std::string error = "invalid parameter";
			error = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(error));
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					error.c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}

		if (!args[0]->IsString()) {			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! invalid params[str]").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}
		v8::String::Utf8Value param_type(args[0]->ToString());
		std::string data_d = std::string(*param_type);//ϵͳ��ʹ��������Ϣ




		//�򿪱���hdr�ļ�
		std::ifstream red_hdrfile("./template.hdr", std::fstream::in);
		if (!red_hdrfile.is_open())
		{
			red_hdrfile.open("./resources/extraResources/template.hdr", std::fstream::in);
		}

		char buffer[2048];
		std::string result = "";
		bool isFind = false;
		if (data_d.compare("wavelength") == 0)
		{
			while (red_hdrfile.getline(buffer, 2048))
			{
				if (strstr(buffer, "wavelength ="))
				{
					isFind = true;

				}
				if (isFind)
				{
					result += buffer;
				}
			}

			int indefOx = result.find_first_of("{");
			if (indefOx != -1)
			{
				result = result.substr(indefOx + 1, result.length() - indefOx - 2);
			}
		}
		else if (data_d.compare("default bands") == 0) {
			while (red_hdrfile.getline(buffer, 2048))
			{
				if (strstr(buffer, "default bands"))
				{
					result += buffer;
					break;

				}
			}
			int indefOx = result.find_first_of("{");
			if (indefOx != -1)
			{
				result = result.substr(indefOx + 1, result.length() - indefOx - 2);
			}

		}
		red_hdrfile.close();

		//printf("indefOx =%d\r\n%s\r\n", indefOx, result.c_str());
		//
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str(), NewStringType::kNormal).ToLocalChecked());
	}

	/**************************************************
	***@author:cui_sh
	***@brief:��ȡ��ǰ�豸�б��еĴ����豸
	***@time:
	***@version:
	**************************************************/
	void MyObject::getSerialPortList(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		std::string serial_info = EncodeUtils::getComMonDevices();
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, serial_info.c_str(), NewStringType::kNormal).ToLocalChecked());
	}

	void MyObject::openCamery(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		/*��Ҫ��У�飬�豸û�򿪲���ִ��*/
		if (m_IpxCamera->lDevice != nullptr)
		{
			std::string error = "Device is already on";
			error = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(error));
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					error.c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}
		if (args.Length() < 2) {
			// �׳�һ�����󲢴��ص� JavaScript��
			std::string error = "invalid parameter";
			error = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(error));
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					error.c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}

		if (!args[0]->IsNumber())
		{
			std::string error = "Wrong parameter type";
			error = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(error));
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					error.c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}

		if (!args[1]->IsString()) {			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! invalid params[str]").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}
		v8::String::Utf8Value param_type(args[1]->ToString());
		std::string data_d = std::string(*param_type);//ϵͳ��ʹ��������Ϣ
		//printf("%s\r\n", data_d.c_str());
		// ִ�в���
		int value = args[0].As<Number>()->Value();

		bool res = m_IpxCamera->openImperx(value);
		if (res)
		{
			/*neb::CJsonObject sys_iniparam(data_d);
			int ExposureTimeRaw;
			sys_iniparam.Get("ExposureTimeRaw", ExposureTimeRaw);
			printf("ExposureTimeRaw = %d\r\n", ExposureTimeRaw);*/
			res = m_IpxCamera->setIniParamValue(data_d);//��ʹ������
			res = m_IpxCamera->iniCameryParam();//������ʹ��
			if (res)
			{
				g_sysinfo = false;
				//����һ��ϵͳ״̬�㲥�߳�
				uv_thread_create(&g_sysInfoThreadId, sysInfoHandle, nullptr);
				
			}
			
		}
		else
		{
			std::string error = "Open camera error";
			error = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(error));
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					error.c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}
		std::string msg = (res == true ? "success" : "error");
		msg = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(msg));
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, msg.c_str(), NewStringType::kNormal).ToLocalChecked());
	}

	void MyObject::closeCamery(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		if (is_save)
		{
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					EncodeUtils::toUtf8String(L"error! Data acquisition").c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}
		/*��Ҫ��У�飬�豸�򿪲���ִ��*/
		if (m_IpxCamera->lDevice != nullptr)
		{
			bool res = m_IpxCamera->closeImperx();

			
			
			
			g_sysinfo = true;
			uv_thread_join(&g_sysInfoThreadId);

			std::string msg = (res == true ? "success" : "error");
			msg = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(msg));
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, msg.c_str(), NewStringType::kNormal).ToLocalChecked());
		}
		else
		{
			std::string error = "�豸�Ѿ��ر�";
			error = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(error));
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					error.c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}


	}
	/**************************************************
	***@author:������������������
	***@brief:��������״̬ true�ǳɹ� false��ʧ��
	***@time:
	***@version:
	**************************************************/
	void MyObject::setParamImperx(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		//����Ƿ�򿪣��򿪲��ܰ��в�������

		if (m_IpxCamera->lDevice == nullptr)
		{
			std::string error = "Device is not open";
			error = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(error));
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					error.c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}

		//��һ���������ַ��� �������ò���������
		//�ڶ���������number���͵�ֵ 
		
		if (args.Length() < 2)
		{
			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! invalid param").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}
		if (!args[0]->IsString()) {			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! invalid params[str]").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}
		if (!args[1]->IsNumber()) {			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! invalid params[num]").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}
		//ִ�в������� ��Ҫ�����û�����Ĳ�������API����
		v8::String::Utf8Value param_type(args[0]->ToString());
		std::string data_d = std::string(*param_type);
		int param_value = args[1].As<Number>()->Value();
		bool result = false;
		if (data_d.compare("ProgFrameTimeAbs") == 0){
			result = m_IpxCamera->setProgFrameTimeAbs(param_value);
		}
		else if (data_d.compare("ExposureTimeRaw") == 0) {
			//printf("ExposureTimeRaw:%d\r\n", param_value);
			result = m_IpxCamera->setExposureTimeRaw(param_value);
		}
		else if (data_d.compare("GainRaw") == 0) {
			result = m_IpxCamera->setGainRaw(param_value);
		}
		else if (data_d.compare("TriggerMode") == 0) {
			result = m_IpxCamera->setTriggerMode(param_value);
		}
		else if (data_d.compare("TriggerActivation") == 0) {
			result = m_IpxCamera->setTriggerActivation(param_value);
		}
		std::wstring msg_out = (result==true)?L"update success":L"error!";
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, EncodeUtils::toUtf8String(msg_out.c_str()).c_str(), NewStringType::kNormal).ToLocalChecked());
	}
	/**************************************************
	***@author:��ȡ�������
	***@brief:���Ӿ�������ݶ�Ӧ��ֵ
	***@time:
	***@version:
	**************************************************/
	void MyObject::getParamImperx(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		//����Ƿ�򿪣��򿪲��ܰ��в�������

		if (m_IpxCamera->lDevice == nullptr)
		{
			std::string error = "Device is not open";
			error = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(error));
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					error.c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}

		//��һ���������ַ��� �������ò���������
		//�ڶ���������number���͵�ֵ 

		if (args.Length() < 1)
		{
			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! invalid param").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}
		if (!args[0]->IsString()) {			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! invalid params[str]").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}

		v8::String::Utf8Value param_type(args[0]->ToString());
		//����������
		std::string data_d = std::string(*param_type);

		int result = -1;
		if (data_d.compare("ProgFrameTimeAbs") == 0) {
			result = m_IpxCamera->getProgFrameTimeAbs();
		}
		else if (data_d.compare("CurrentFrameRate") == 0) {
			result = m_IpxCamera->getCurrentFrameRate();
		}
		else if (data_d.compare("ExposureTimeRaw") == 0) {
			result = m_IpxCamera->getExposureTimeRaw();
		}
		else if (data_d.compare("GainRaw") == 0) {
			result = m_IpxCamera->getGainRaw();
		}
		else if (data_d.compare("TriggerMode") == 0) {
			result = m_IpxCamera->getTriggerMode();
		}
		else if (data_d.compare("TriggerActivation") == 0) {
			result = m_IpxCamera->getTriggerActivation();
		}

		//Local<Number> num = Number::New(isolate, result);
		args.GetReturnValue().Set(Number::New(isolate, result));
		
		
	}
	void MyObject::cameryCallback(IpxCam::Buffer* buffer)
	{
		//
		
		g_asyncData.type = L"Frame";
		g_asyncData.message = L"Frame captured!";
		g_async.data = &g_asyncData;
		//����ͼ������
		auto imagePtr = static_cast<uchar*>(buffer->GetBufferPtr()) + buffer->GetImageOffset();
		cv::Mat image(buffer->GetHeight(), buffer->GetWidth(), CV_16UC1, imagePtr);
		//
		
		if (is_save)
		{
			specData data_struct;
			data_struct.imagedata = new uint8_t[buffer->GetBufferSize()];
			memcpy(data_struct.imagedata, image.data, buffer->GetBufferSize());
			data_struct.bufSize = buffer->GetBufferSize();
			data_struct.Height = buffer->GetHeight();
			data_struct.Width = buffer->GetWidth();
			specDataT.push(data_struct);
			//cv::imshow("image", image);
			//cv::waitKey(5);
		}
		
		//
		cv::Mat spectrum = image.colRange(spectrum_cols, spectrum_cols+1).clone();

		//cv::imshow("image", image);
		if (current_col == 0)
		{
			//����һ����ɫͨ��ͼ��
			rgb_mat = cv::Mat(buffer->GetWidth(), rgb_mat_width, CV_16UC3, cv::Scalar(4095, 4095, 4095));
			current_col++;
		}
		for (int index = 0; index < image.cols; index++)
		{
			/*if (index < image.rows)
			{
				spectrumBuf[index] =image.at<ushort>(index, spectrum_cols);
			}*/
			//b g r 
			//rgb_mat.at<cv::Vec3s>(rows,cols)
			rgb_mat.at<cv::Vec3s>(index, current_col-1)[0] = image.at<ushort>(b_row, index);//B
			rgb_mat.at<cv::Vec3s>(index, current_col-1)[1] = image.at<ushort>(g_row, index);//G
			rgb_mat.at<cv::Vec3s>(index, current_col-1)[2] = image.at<ushort>(r_row, index);//R
		}
		

		if (current_col < rgb_mat.cols)
		{
			current_col++;
		}
		else
		{
			//current_col = 1;
			//converted
			cv::Mat t_mat = cv::Mat::zeros(2, 3, CV_32FC1);

			t_mat.at<float>(0, 0) = 1;
			t_mat.at<float>(0, 2) = -1; //ˮƽƽ����
			t_mat.at<float>(1, 1) = 1;
			t_mat.at<float>(1, 2) = 0; //��ֱƽ����

			//����ƽ�ƾ�����з���任
			cv::warpAffine(rgb_mat, rgb_mat, t_mat, rgb_mat.size());

			t_mat.release();
		}
		//
		
		cv::Mat converted = rgb_mat.clone();

		cv::resize(converted, converted, cv::Size(640, 480));
		
		double g_MaxValue;
		cv::minMaxLoc(converted, 0, &g_MaxValue, 0, 0);
		(g_MaxValue > 4095) ? g_MaxValue = 4095: g_MaxValue= g_MaxValue;
		converted.convertTo(converted, CV_8U, 255.0 / g_MaxValue);
		
		//printf("%f    %f\r\n", 255.0 / g_MaxValue,g_MaxValue);
		//cv::imshow("color_tobgra", converted);
		//cv::waitKey(5);
		cv::cvtColor(converted, converted, cv::COLOR_RGB2BGRA);
		
		//cv::imshow("color_tobgra", converted);
		//cv::waitKey(5);
		memcpy(g_frameBuffer, converted.data, BUFFER_SIZE);

		memcpy(spectrumBuf, spectrum.data, BUFFER_SPEC);
		//cv::imshow("spectrum", spectrum);
		//cv::waitKey(5);
		uv_async_send(&g_async);
		
		spectrum.release();
		converted.release();
		image.release();

		

	}
	void MyObject::wavelengthSwitch(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		if (args.Length() < 2)
		{
			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! invalid param").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}
		if (!args[0]->IsString()) {			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! invalid params[str]").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}
		if (!args[1]->IsNumber()) {			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! invalid params[num]").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}

		v8::String::Utf8Value param_type(args[0]->ToString());
		std::string data_d = std::string(*param_type);
		int param_value = args[1].As<Number>()->Value();

		if (data_d.compare("r") == 0 || data_d.compare("R") == 0)
		{
			r_row = param_value;
		}
		else if (data_d.compare("g") == 0 || data_d.compare("G") == 0)
		{
			g_row = param_value;
		}
		else if (data_d.compare("b") == 0 || data_d.compare("B") == 0)
		{
			b_row = param_value;
		}
		else if (data_d.compare("spectrum") == 0)
		{
			spectrum_cols = param_value;
		}

		args.GetReturnValue().Set(String::NewFromUtf8(isolate, EncodeUtils::toUtf8String(L"Update success").c_str(), NewStringType::kNormal).ToLocalChecked());


	}
	//�򿪴���
	void MyObject::openCmdSerialPort(const v8::FunctionCallbackInfo<v8::Value>& args)
	{

		Isolate* isolate = args.GetIsolate();
		if (args.Length() < 1) {
			// �׳�һ�����󲢴��ص� JavaScript��
			std::string error = "invalid parameter";
			error = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(error));
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					error.c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}


		

		//������ģ���Ƿ��Ѿ�����
		if (m_Serial != NULL && m_Serial->connect_status)
		{
			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! STM Device is already on.").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}

		
		
		//��ȡ�����豸�б�
		neb::CJsonObject list(EncodeUtils::getComMonDevices());
		bool isFind = false;
		int tmp_port = 0;
		std::string cmd = "AA38000000326A55";//����ָ��
		for (int index = 0; index < list["SerialInfo"].GetArraySize(); index++)
		{
			
			std::string serial_name;
			list["SerialInfo"][index].Get("info", serial_name);
			tmp_port = EncodeUtils::getUsartCom(serial_name);//ȡ�ô��ں�
			//printf("%s\r\n", serial_name.c_str());
			if (tmp_port == -1)
			{
				continue;
			}
			//�������ڲ���Ŀ���豸
			CSerialPort *m_Serial_q = new CSerialPort();
			if (m_Serial_q->InitPort(tmp_port, CBR_115200/*CBR_115200*/, 'N', 8, 1, EV_RXCHAR)) {
				BYTE buf[512];
				int readLength = 512;
				//��������ָ��
				m_Serial_q->WriteDataHex((char*)cmd.c_str(), cmd.length());
				//ȡ������ָ��
				m_Serial_q->readDataAsyncHex(buf, readLength);
				//printf("%s %d\r\n", buf, readLength);
				if (readLength > 0) {
					//printf("%s %d\r\n", buf, readLength);
					std::string result((char*)buf);
					if (result.compare(cmd.c_str()) == 0) {
						if (m_Serial_q != NULL) {
							m_Serial_q->~CSerialPort();
							m_Serial_q = NULL;
						}
						//printf("�ҵ���\r\n");
						isFind = true;
						break;
					}
				}
			}
			if (m_Serial_q != NULL) {
				m_Serial_q->~CSerialPort();
				m_Serial_q = NULL;
			}
		}
		
		//printf("���ҽ���\r\n");
		if (!isFind) {
			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! STM Exception!").c_str(),					NewStringType::kNormal).ToLocalChecked()));						return;
		}
		//printf("���ҽ���555\r\n");
		//�Ѿ��ҵ�����Ҫ���ӵĴ��� tmp_port
		if (m_Serial == NULL) {
			m_Serial = new CSerialPort();
			if (m_Serial->InitPort(tmp_port, 115200/*CBR_115200*/, 'N', 8, 1, EV_RXCHAR)) {
				g_syscmd = false;
				uv_thread_create(&g_cmdCallbackThreadId, sysCmdHandle, nullptr);
				//�򿪿��ƴ��ڳɹ� ��Ҫ�豸�ص�����
				m_Serial->setCallbackFun(usartCallback);//���ûص�����
				m_Serial->OpenListenThread();//�򿪼���
				
				args.GetReturnValue().Set(String::NewFromUtf8(isolate, EncodeUtils::toUtf8String(L"success").c_str(), NewStringType::kNormal).ToLocalChecked());
				return;
			}
			else
			{
				m_Serial->~CSerialPort();
				m_Serial = NULL;
				isolate->ThrowException(Exception::TypeError(					String::NewFromUtf8(isolate,						EncodeUtils::toUtf8String(L"error! can't connect to stm").c_str(),						NewStringType::kNormal).ToLocalChecked()));				return;
			}
		}


		//����ʧ��
		isolate->ThrowException(Exception::TypeError(			String::NewFromUtf8(isolate,				EncodeUtils::toUtf8String(L"error! Serial Unknown Exception!").c_str(),				NewStringType::kNormal).ToLocalChecked()));		return;

	}

	//�򿪴���
	void MyObject::openSerialPort(const v8::FunctionCallbackInfo<v8::Value>& args)
	{

		Isolate* isolate = args.GetIsolate();
		if (args.Length() < 1) {
			// �׳�һ�����󲢴��ص� JavaScript��
			std::string error = "invalid parameter";
			error = EncodeUtils::UnicodeToUtf8(EncodeUtils::StringToWString(error));
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate,
					error.c_str(),
					NewStringType::kNormal).ToLocalChecked()));
			return;
		}

		
		if (!args[0]->IsNumber()) {			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! invalid params[number]").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}
		//gps�����Ƿ��Ѿ���
		if (m_gpsSerial != NULL && m_gpsSerial->connect_status)
		{
			isolate->ThrowException(Exception::TypeError(				String::NewFromUtf8(isolate,					EncodeUtils::toUtf8String(L"error! gps Device is already on.").c_str(),					NewStringType::kNormal).ToLocalChecked()));			return;
		}
		
		

		int gpsport = args[0].As<Number>()->Value();
		//printf("gpsport=%d ",gpsport);
		/*
		*1����gps�˿�
		*2���Զ����Ӵ������ƶ˿� ��õ�������źźͿ����ź�
		*/
		//��gps�˿�
		if (m_gpsSerial == NULL)
		{
			m_gpsSerial = new CSerialPort();
			//��gps�˿�
			if (m_gpsSerial->InitPort(gpsport, CBR_115200/*CBR_115200*/, 'N', 8, 1, EV_RXCHAR)) {
				//ע��ص����ڽ���gps��Ϣ
				m_gpsSerial->setCallbackFun(usartgpsCallback);
				//����һ�����������Ϣִ���߳�
				//printf("m_gpsSerial=%d ", gpsport);
				g_sysgps = false;
				uv_thread_create(&g_gpsCallbackThreadId, sysGpsHandle, nullptr);
				//�򿪼���
				m_gpsSerial->OpenListenThread();
				return;
			}
			else{
				m_gpsSerial->~CSerialPort();
				m_gpsSerial = NULL;
				isolate->ThrowException(Exception::TypeError(					String::NewFromUtf8(isolate,						EncodeUtils::toUtf8String(L"error! can't connect to gps").c_str(),						NewStringType::kNormal).ToLocalChecked()));				return;
			}
		}
		
		//����ʧ��
		isolate->ThrowException(Exception::TypeError(			String::NewFromUtf8(isolate,				EncodeUtils::toUtf8String(L"error! Serial Unknown Exception!").c_str(),				NewStringType::kNormal).ToLocalChecked()));		return;
	
	}
	
	void MyObject::usartgpsCallback(char data){
		/*��������gps�忨���ư����Ϣ*/
		/*��Ϣ�洢������Ŀ¼*/
		if (is_save_gps)
		{
			//printf("%c", data);
			gpsFileWrite.put(data);
			//gpsFileWrite.write(&data,1);
		}

		//������Ҫ����  ���IMU����״̬ �� RTKʵ��״̬
		gps_data.push(data);
		printf("%c\r\n", data);
	}

	void MyObject::usartCallback(char data)
	{
		/*��������STM32���ư����Ϣ*/
		/*��Ϣ����ϵͳ����*/
		//printf("��Ϣ����ϵͳ���� \r\n");
		sprintf_s(buf2hex, "%02X", (BYTE)data);
		//printf("%s ", buf2hex);
		recvresult.append(buf2hex);
		//printf("%s ", recvresult.c_str());
		//printf("%s %d %d %d\r\n", recvresult.c_str(), recvresult.find("AA"), recvresult.substr(recvresult.length() - 2, 2).compare("55"), recvresult.length());
		
		if ((recvresult.find("AA") != 0) && (recvresult.length() >= 2))
		{
			//printf("recvresult=""\r\n");
			recvresult = "";
			//printf("recvresult = """);
		}
		/*if (recvresult.length() == 2 && recvresult.compare("AA") != 0)
		{
			recvresult = "";
		}*/
		else if (recvresult.find("AA") == 0 /*��AA��ͷ*/ 
				&& recvresult.substr(recvresult.length()-2,2).compare("55")==0 /*��55����*/
				&& (recvresult.length() == 16)/*ָ���*/)
		{
			//�õ�ָ�����У����Ƿ���ȷ
			//printf("��ȷָ��\r\n");
			//AA 37 00 00 00 00 37 55
			if (EncodeUtils::getCheck_ConnectCmd((char*)recvresult.c_str(), recvresult.length())) {
				//У�����ȷ
				//printf("��ȷָ��--%s \r\n", recvresult.c_str());
				//ָ����Ҫ������Ӧ����
				//ָ���������� ���̸߳������  ͬʱ����㲥
				rev_cmd.push(recvresult);
			}
			recvresult = "";
		}
		else if (recvresult.length() > 16)
		{
			recvresult = "";
		}
	}
	void MyObject::closeSerialPort(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		if (m_gpsSerial != NULL)
		{
			g_sysgps = true;
			uv_thread_join(&g_gpsCallbackThreadId);


			m_gpsSerial->~CSerialPort();
			m_gpsSerial = NULL;
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, EncodeUtils::toUtf8String(L"success!").c_str(), NewStringType::kNormal).ToLocalChecked());
			return;
		}
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, EncodeUtils::toUtf8String(L"error! Device not open.").c_str(), NewStringType::kNormal).ToLocalChecked());
	}

	void MyObject::closeCmdSerialPort(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		if (m_Serial != NULL)
		{
			g_syscmd = true;
			uv_thread_join(&g_cmdCallbackThreadId);

			m_Serial->~CSerialPort();
			m_Serial = NULL;
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, EncodeUtils::toUtf8String(L"success!").c_str(), NewStringType::kNormal).ToLocalChecked());
			//printf("closeCmdSerialPort\r\n");
			return;
		}

		args.GetReturnValue().Set(String::NewFromUtf8(isolate, EncodeUtils::toUtf8String(L"error! Device not open.").c_str(), NewStringType::kNormal).ToLocalChecked());
	}
	
	void MyObject::takePhoto(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Isolate* isolate = args.GetIsolate();
		
		//����
		//cv::write("", matimg);
		FocusElement * element = new FocusElement();
		element->work.data = element;
		element->type = -1;
		element->callback.Reset(isolate, Local<Function>::Cast(args[0]));
		m_takePhoto = true;
		uv_queue_work(uv_default_loop(), &element->work, startTakephoto, TakephotoCompleted);

	}

	void MyObject::startTakephoto(uv_work_t * work)
	{
		FocusElement * element = (FocusElement*)work->data;
		while (m_takePhoto)
		{
			Sleep(100);
		}
		vector <int > compression_params;
		compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
		compression_params.push_back(0);
		CTime t = CTime::GetCurrentTime();
		char buf[128];
		sprintf(buf,"./qhyy_data/%04d%02d%02d%02d%02d%02d.png", t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
		std::string path = buf;
		EncodeUtils::createDirectory(path);
		cv::imwrite(path.c_str(), save_image, compression_params);
		save_image.release();
		element->type = 100;
	}
	void MyObject::TakephotoCompleted(uv_work_t * work, int status)
	{
		FocusElement * element = (FocusElement*)work->data;
		Isolate * isolate = Isolate::GetCurrent();
		const unsigned argc = 1;
		Local<Value> argv[argc] = { String::NewFromUtf8(isolate, std::to_string(element->type).c_str()).ToLocalChecked() };
		Local<Function>::New(isolate, element->callback)->Call(isolate->GetCurrentContext(),isolate->GetCurrentContext()->Global(), argc, argv);
		element->callback.Reset();
		delete element;
	}
	
	
	DWORD MyObject::HEXS(char *decString){
		DWORD hexValue = 0;
		DWORD sl = 0;
		BOOL isWhile = FALSE;
		DWORD idx = 0;
		char str[256];
		BYTE ct = 0;

		sl = strlen(decString);
		if ((sl > 0) && (sl < 256))
			isWhile = TRUE;
		strcpy(str, decString);
		idx = 0;
		while (isWhile)
		{
			ct = (BYTE)(str[idx]);
			switch (ct)
			{
			case 48: // "0"
				hexValue = hexValue << 4;
				hexValue += 0x0;
				break;
			case 49: // "1"
				hexValue = hexValue << 4;
				hexValue += 0x01;
				break;
			case 50: // "2"
				hexValue = hexValue << 4;
				hexValue += 0x02;
				break;
			case 51: // "3"
				hexValue = hexValue << 4;
				hexValue += 0x03;
				break;
			case 52: // "4"
				hexValue = hexValue << 4;
				hexValue += 0x04;
				break;
			case 53: // "5"
				hexValue = hexValue << 4;
				hexValue += 0x05;
				break;
			case 54: // "6"
				hexValue = hexValue << 4;
				hexValue += 0x06;
				break;
			case 55: // "7"
				hexValue = hexValue << 4;
				hexValue += 0x07;
				break;
			case 56: // "8"
				hexValue = hexValue << 4;
				hexValue += 0x08;
				break;
			case 57: // "9"
				hexValue = hexValue << 4;
				hexValue += 0x09;
				break;
			case 65: // "A"
				hexValue = hexValue << 4;
				hexValue += 0x0a;
				break;
			case 97: // "a"
				hexValue = hexValue << 4;
				hexValue += 0x0a;
				break;
			case 66: // "B"
				hexValue = hexValue << 4;
				hexValue += 0x0b;
				break;
			case 98: // "b"
				hexValue = hexValue << 4;
				hexValue += 0x0b;
				break;
			case 67: // "C"
				hexValue = hexValue << 4;
				hexValue += 0x0c;
				break;
			case 99: // "c"
				hexValue = hexValue << 4;
				hexValue += 0x0c;
				break;
			case 68: // "D"
				hexValue = hexValue << 4;
				hexValue += 0x0d;
				break;
			case 100: // "d"
				hexValue = hexValue << 4;
				hexValue += 0x0d;
				break;
			case 69: // "E"
				hexValue = hexValue << 4;
				hexValue += 0x0e;
				break;
			case 101: // "e"
				hexValue = hexValue << 4;
				hexValue += 0x0e;
				break;
			case 70: // "F"
				hexValue = hexValue << 4;
				hexValue += 0x0f;
				break;
			case 102: // "f"
				hexValue = hexValue << 4;
				hexValue += 0x0f;
				break;
			default: //unknown char
				isWhile = FALSE;
				break;
			}
			idx++;
			if (idx >= sl)
				isWhile = FALSE;
		}
		/*if (str)
		{
			delete[] str;
		}*/


		return  hexValue;
	}
	int MyObject::recheckSum(std::string data) {
		//printf("check -001\r\n");
		int sum = 0;
		char tmp[3];



		tmp[0] = data.at(2);
		tmp[1] = data.at(3);
		tmp[2] = '\0';
		DWORD value = HEXS(tmp);
		sum += value;
		tmp[0] = data.at(4);
		tmp[1] = data.at(5);
		tmp[2] = '\0';
		value = HEXS(tmp);
		sum += value;
		tmp[0] = data.at(6);
		tmp[1] = data.at(7);
		tmp[2] = '\0';
		value = HEXS(tmp);
		sum += value;
		tmp[0] = data.at(8);
		tmp[1] = data.at(9);
		tmp[2] = '\0';
		value = HEXS(tmp);
		sum += value;
		tmp[0] = data.at(10);
		tmp[1] = data.at(11);
		tmp[2] = '\0';
		value = HEXS(tmp);
		sum += value;
		sum &= 0x00ff;
		return sum;
	}
	char *MyObject::getFormatStringForCan(int num, char*cmdhead) {
		char *result = new char[128];
		memset(result, '\0', 128);
		sprintf(result, "%s", cmdhead);
		char buff[16];
		memset(buff, '\0', 16);



		int value = (num >> 24) & 0x00FF;
		sprintf(buff, "%02X", value);
		sprintf(result, "%s%s", result, buff);
		memset(buff, '\0', 16);

		value = (num >> 16) & 0x00FF;
		sprintf(buff, "%02X", value);
		sprintf(result, "%s%s", result, buff);
		memset(buff, '\0', 16);

		value = (num >> 8) & 0x00FF;
		sprintf(buff, "%02X", value);
		sprintf(result, "%s%s", result, buff);
		memset(buff, '\0', 16);



		value = num & 0x00FF;
		sprintf(buff, "%02X", value);
		sprintf(result, "%s%s", result, buff);

		memset(buff, '\0', 16);


		//����У���
		value = recheckSum(result);
		sprintf(buff, "%02X", value);
		sprintf(result, "%s%s%s", result, buff, "BA");


		return result;
	}
	
	
	
}  // namespace demo

