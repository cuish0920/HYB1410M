#ifndef MYOBJECT_H
#define MYOBJECT_H
#include <node.h>


#include <node_object_wrap.h>
//////////////////�ַ�����///////////////////
#include "EncodeUtils.h"
//////////////////�ַ�����///////////////////


//////////////////����json///////////////////
#include "CJsonObject.h"
//////////////////����json///////////////////

//////////////////����opencv///////////////////
#include "OpencvUtils.h"
//////////////////����opencv///////////////////

//////////////////XML�ļ�///////////////////
#include "xmlOper.h"
//////////////////XML�ļ�///////////////////

//////////////////����֧��///////////////////
//#include "SerialPort.h"
#include "SerialUtils.h"
//////////////////����֧��///////////////////

//////////////////���������֧��///////////////////
#include "IpxCamera.h"
//////////////////���������֧��///////////////////

//////////////////���������֧��///////////////////
#include "LogUtils.h"
//////////////////���������֧��///////////////////

//////////////////���������֧��///////////////////
#include "threadsafe_queue.h"

//#include <thread>

//////////////////���������֧��///////////////////


#include <uv.h>

#define GPS_ANY_SIZE 1024

#define GPS_ANY_GGA 2048

#define GPS_ANY_SAVE 1024

#define CAMERY_SIZE_R 100
#define CAMERY_RENDER 6
#define RENDER_MAX_FPS 20
#define MB2
struct AsyncData
{
	std::wstring type;  // ����
	std::wstring message;  // ��Ϣ
};
static AsyncData g_asyncData;//�����ǻص�

static AsyncData g_sysInfoAsyncData;//ϵͳ��Ϣ�ص�

static AsyncData g_recCmdAsyncData;//

static AsyncData g_recGpsAsyncData;//

typedef struct  specData {
	uint8_t * imagedata;
	uint32_t bufSize;
	int Width;
	int Height;
};

namespace demo {

	class MyObject : public node::ObjectWrap {
	public:
		static void Init(v8::Isolate* isolate);
		static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);

	private:
		explicit MyObject();
		~MyObject();

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void callBack(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void sysInfoCallBack(const v8::FunctionCallbackInfo<v8::Value>& args);
		static v8::Persistent<v8::Function> constructor;

		// ��ȡ�� Frame
		static v8::Persistent<v8::Function> g_frameDataCallback;

		static v8::Persistent<v8::Function> g_sysInfoCallback;

		static void RecCmdCallBack(const v8::FunctionCallbackInfo<v8::Value>& args);
		static v8::Persistent<v8::Function> g_recCmdCallback;


		static void RecGpsCallBack(const v8::FunctionCallbackInfo<v8::Value>& args);
		static v8::Persistent<v8::Function> g_recGpsCallback;

		// �Ƿ��˳�����ͷץȡ�߳�
		static bool g_quit;
		static bool g_sysinfo;
		static bool g_syscmd;
		static bool g_sysgps;

		// ����ͷץȡ�߳�
		static uv_thread_t g_captureThreadId;
		static uv_thread_t g_captureThreadIdPicView;

		static uv_thread_t g_sysInfoThreadId;

		static uv_thread_t g_cmdCallbackThreadId;
		static uv_thread_t g_gpsCallbackThreadId;
		//����

		// ���������ݴ洢�߳�
		static uv_thread_t g_saveThreadId;

		// �첽��������������ͷץȡ�̺߳����߳�֮�䴫������
		static uv_async_t g_async;
		//ϵͳ״̬��Ϣ�ص�
		static uv_async_t g_asyncSysInfo;
		static uv_async_t g_asyncRecCmd;
		static uv_async_t g_asyncRecGps;

		static void start(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void stop(const v8::FunctionCallbackInfo<v8::Value>& args);

		static UINT WINAPI saveData(void *arg);

		static int saveDataThread();
		static int viewThread();
		static UINT WINAPI capture(void *arg);
		static UINT WINAPI captureview(void *arg);

		static UINT WINAPI sysInfoHandle(void *arg);

		static UINT WINAPI sysCmdHandle(void *arg);
		static UINT WINAPI sysGpsHandle(void *arg);
		//static void sysGpsHandle(void *arg);

		static void broadcast();

		static void eventCallback(uv_async_t* handle);

		static void eventCallbackSysInfo(uv_async_t* handle);
		static void eventCallbackRecCmd(uv_async_t* handle);
		static void eventCallbackRecGps(uv_async_t* handle);
		static double get_wall_time();
		static unsigned char* g_frameBuffer;
		static unsigned char* spectrumBuf;
		/*****************��־�ص�����******************************/
		static v8::Persistent<v8::Function> g_logCallback;
		static void setLogCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void log(v8::Isolate* isolate, const wchar_t * const message);

		static void createFrameBuffer(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void createSpecBuffer(const v8::FunctionCallbackInfo<v8::Value>& args);

	private:
		static void getCameryAQStatus(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void getCameryRecStatus(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void getCameryList(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void getHdrInfo(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void openCamery(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void cameryCallback(IpxCam::Buffer* buffer);
		static void closeCamery(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void setParamImperx(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void getParamImperx(const v8::FunctionCallbackInfo<v8::Value>& args);
		
		/*static std::queue<specData*>specDataT;
		static std::queue<specData*>specDataTView;*/

		static threadsafe_queue<specData> specDataT;
		static threadsafe_queue<specData> specDataTView;

		static bool is_save;
		static void start_rec(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void stop_rec(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void UART_SEND(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void wavelengthSwitch(const v8::FunctionCallbackInfo<v8::Value>& args);
	private:
		static void getSerialPortList(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void usartCallback(BYTE* data, int Length);
		static void usartgpsCallback(BYTE* data, int Length);
	private:

	private:
		static cv::Mat rgb_mat;
		static int r_row;
		static int g_row;
		static int b_row;
		static int spectrum_cols;
		static int rgb_mat_width;//ͼ��Ŀ����Լ�����ģ�����ԭͼ�Ŀ�
		static int current_col;
		static ASYNCSERIAL::AsyncSerial *m_gpsSerial;
		static ASYNCSERIAL::AsyncSerial *m_Serial;
		static char buf2hex[3];
		static std::string recvresult;
		//static std::ofstream gpsFileWrite;//���ڴ洢gps��Ϣ
		static std::FILE * gpsFile;
		//static std::ofstream gpsFileWriteTest;//���ڴ洢gps��Ϣ
		static bool is_save_gps;
		static std::string prefix;
		static std::queue<std::string> rev_cmd;
		//static std::queue<char> gps_data;
		//static std::queue<char> gps_data_save;
		//
		static char *gps_org;
		static int gps_org_index;
		static char *gps_any;
		static int gps_any_index;

		static std::string gps_ascii;
		static int64_t ExposureTime;
		static double time_stamp;
		static std::vector<std::string> gps_msg;
	private:
		static void ID_06_ANY(std::string data);
		static void ID_0E_ANY(std::string data);
		static std::string gps_buf_06;
		static std::string gps_buf_0E;
		static int ekf_modal;
		static int rtk_modal;
		static std::string EKF_STATUS;
		static std::string RTK_STATUS;
	private:

		static void openSerialPort(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void closeSerialPort(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void openCmdSerialPort(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void closeCmdSerialPort(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void takePhoto(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void startTakephoto(uv_work_t * work);
		static void TakephotoCompleted(uv_work_t * work, int status);


		static void startSendCommand(uv_work_t * work);
		static void SendCommandCompleted(uv_work_t * work, int status);

	private:
		/** ͬ������,�ٽ������� */
		static CRITICAL_SECTION   m_cs_0;//!<gps����
		static CRITICAL_SECTION   m_cs_1;//!<���Ŀ��ư�
		
		static CRITICAL_SECTION   m_cs_4;//!<ϵͳ��Ϣ�ص�
	private:
		//static void autoFocus();
		static double currentArticulation;
		static cv::Mat save_image;
		static bool m_takePhoto;
		static int64_t FrameRate;

		static DWORD HEXS(char *decString);
		static int recheckSum(std::string data);
		static char *getFormatStringForCan(int num, char*cmdhead);
		static int writeLines;
		static std::FILE * spefilewrite;
		static int writeWidth;
		static int writeHeight;
		static int skip_count;
	private:

		void LoadSystemIniParam();
		static neb::CJsonObject sysInfoJson;
	};

}  // namespace demo

#endif