#ifndef __PUSICONTROL_LIB_H
#define __PUSICONTROL_LIB_H

/**************************************�궨��**************************************/


/**************************************���Ͷ���**************************************/
typedef enum
{
	RET_SUCCESS = 0,  //�����ɹ�
	RET_GENERAL_ERR,//ͨ�ô���
	RET_LOG_CREATE_FAIL,  //����LOG�ļ�ʧ��
	RET_COMM_OPEN_FAIL, //�򿪴���ʧ��
	RET_COMM_NOT_OPEN,  //����δ��
	RET_COMM_WRITE_TIMEOUT,  //д���ʱ
	RET_COMM_READ_TIMEOUT,  //�����ʱ
	RET_CMDHEAD_ERROR,   //����ͷ����
	RET_RESP_ADDR_ERROR,   //��Ӧ��ַ����
	RET_CMD_NOTSUPPORT,   //���֧��
	RET_UPDATEFILE_OPEN_FAIL, //�����ļ���ʧ��
	RET_UPDATE_CRC_ERROR, //��������CRC error
	RET_UPDATE_NOT_RESPONSE, //������������Ӧ
}PUSIResult;

typedef enum
{
	PMC006B4 = 0,  //PMC006ƽ̨
	PMC006B3 = 1,  //PIO002ƽ̨
	PIO002B2 = 2,   //PIO002B2ƽ̨
	PCS0801 = 3,  //PCS0801ƽ̨
}PLATFORM_TYPE;

#ifdef __cplusplus
extern "C" {
#endif

/**************************************���Ͷ���**************************************/
typedef void (__stdcall *TraceCallback)(TCHAR* strTrace); //���ٻص���������
typedef void (__stdcall *UpdateCallback)(DWORD dwPercent); //�����ص���������

/**************************************������������**************************************/
void __stdcall SetTraceCallback(TraceCallback callback); //���ø��ٻص�
void __stdcall SetPlatformType(PLATFORM_TYPE platformType); //����ƽ̨����
PUSIResult __stdcall SetLogPath(TCHAR* strLogPath); //��ʼ��LOG
PUSIResult __stdcall OpenPort(UINT portnr, DWORD baud); //��ʼ��ͨ�Ŷ˿�
PUSIResult __stdcall ClosePort(void); //�ر�ͨ�Ŷ˿�
//system control
PUSIResult __stdcall GetFirmwareVersion(DWORD dwAddress,DWORD* pVersion); //��ȡ�̼��汾
PUSIResult __stdcall SetAddress(DWORD dwAddress,DWORD dwNewAddr); //���ÿ�������ַ
PUSIResult __stdcall EnterBootloader(DWORD dwAddress); //����bootloaderģʽ
PUSIResult __stdcall SaveAllPara(DWORD dwAddress); //�������в���
//GPIO
PUSIResult __stdcall SetIOValue(DWORD dwAddress,DWORD dwIOValue); //����IO�˿ڵ�ֵ
PUSIResult __stdcall GetIOValue(DWORD dwAddress,DWORD *pIOValue); //��ȡIO�˿ڵ�ֵ
//offline cmd
PUSIResult __stdcall RunOfflineCmd(DWORD dwAddress,DWORD dwPointer); //���е�ǰ�û�ָ��
PUSIResult __stdcall SetCurrentOfflineCmd(DWORD dwAddress,DWORD dwCmd); //���ÿ���������buffer�ĵ�ǰָ��
PUSIResult __stdcall GetCurrentOfflineCmd(DWORD dwAddress,DWORD *pCmd); //��ȡ����������buffer�ĵ�ǰָ��
PUSIResult __stdcall SetCmdBuffPointer(DWORD dwAddress,DWORD dwPointer); //���õ�ǰ����bufferָ��
PUSIResult __stdcall GetCmdBuffPointer(DWORD dwAddress,DWORD *pPointer); //��ȡ��ǰ����bufferָ��
PUSIResult __stdcall SetCmdBuffCmdCount(DWORD dwAddress,DWORD dwCmdCount); //��������buffer����
PUSIResult __stdcall GetCmdBuffCmdCount(DWORD dwAddress,DWORD *pCmdCount); //��ȡ����buffer����
PUSIResult __stdcall SetOfflineAutoRun(DWORD dwAddress,BOOL bRun); //���������Զ�����
PUSIResult __stdcall SaveCmdBuffer(DWORD dwAddress); //��������buffer����ָ��
//update
void __stdcall SetUpdateCallback(UpdateCallback callback); //�����������̻ص�,������ʾ�ٷֱ�
PUSIResult __stdcall UpdateApplication(DWORD dwAddress,TCHAR *strAppPath); //����Ӧ��

PUSIResult __stdcall GetDebugData(DWORD dwAddress,DWORD *pDebugData);

#ifdef __cplusplus
}
#endif

#endif