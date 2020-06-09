#include "pusi.h"
#define MOTOR_FORWARD	0 //1
#define MOTOR_REVERSE	1 //0
#define MOTOR_READY_DIRECTIONFORWARD 0
#define MOTOR_READY_DIRECTIONREVERSE 1
#define ERROR_HANDLE(closePort,errInfo)	if(RET_SUCCESS != result){\
	std::printf(errInfo);\
	if(TRUE == closePort){\
		ClosePort();\
	}\
	return;\
}

//#include <vector>
namespace PUSI {
	using v8::Persistent;
	using v8::Function;
	struct Data {
		int addr;//��Ҫ����ĵ����ַ
		int offset;//���������λ
		int zeroSpeed;//�����ٶ�
		int speed;//�����ٶ�
	};

	//��¼ִ�й���ĵ������Щ
	int PUSI_FUN::cout_zero_equp = 0;
	bool PUSI_FUN::is_connection = false;
	bool PUSI_FUN::is_move = false;
	neb::CJsonObject PUSI_FUN::gohome_info;//��¼��ǰ����ĵ������Щ

	PUSI_FUN::PUSI_FUN() {

	}

	PUSI_FUN::~PUSI_FUN() {
	}

	/**************************************************
	***@author:cui_sh
	***@brief:���ݴ���Ĳ����õ��ִ�й���
	***@time:
	***@version:
	**************************************************/
	void PUSI_FUN::go_to_home(neb::CJsonObject list) 
	{
		//printf("go_to_home %d\r\n",list["addrList"].GetArraySize());
		//�жϵ�ǰ����͵����ַ��Ȼ��Դ���ĵ��ִ�������
		
		//cout_zero_equp = 0;
		for (int index = 0; index < list["addrList"].GetArraySize(); index++) 
		{
			int addr;
			int zeroSpeed;//�����ٶ�
			int speed;//�����ٶ�
			int offset;
			list["addrList"][index].Get("addr", addr);
			list["addrList"][index].Get("zeroSpeed", zeroSpeed);
			list["addrList"][index].Get("speed", speed);
			list["addrList"][index].Get("offset", offset);
			//printf("%d %d %d %d\r\n", addr, zeroSpeed, speed, offset);
			std::string isExist = "";
			gohome_info.Get(std::to_string(addr), isExist);
			if (isExist.compare("1")==0) {
				//printf("addrList addr:%d ���ڹ���\r\n", addr);
				
				continue;
			}
			gohome_info.Add(std::to_string(addr), std::to_string(true));
			/*gohome_info.Get(std::to_string(addr), isExist);
			printf("���ǰ%s\r\n", isExist.c_str());

			gohome_info.Add(std::to_string(addr), std::to_string(true));
			printf("״̬: %s\r\n", gohome_info.ToString().c_str());

			gohome_info.Get(std::to_string(addr), isExist);
			printf("���ǰ��%s\r\n", isExist.c_str());
			
			printf("addrList addr:%d \r\n", addr);*/
			
			cout_zero_equp++;

			//ȡ����Ҫ����ĵ��
			//���������̣߳���������ִ�й������
			Data* data = new Data();
			data->addr = addr;
			data->offset = offset;
			data->zeroSpeed = zeroSpeed;
			data->speed = speed;
			uv_work_t *req = new uv_work_t();
			req->data = data;
			uv_queue_work(
				uv_default_loop(),
				req,
				(uv_work_cb)pusiReset,
				(uv_after_work_cb)afterPusiReset
			);


		}
		//printf("gohome_info: %s\r\n",gohome_info.ToString().c_str());
		//�ȴ����е����������� ��ŷ���
		while (cout_zero_equp > 0) {

			Sleep(50);
		}

	}

	/**************************************************
	***@author:cui_sh
	***@brief:ִ�й������
	***@time:
	***@version:
	**************************************************/
	void PUSI_FUN::pusiReset(uv_work_t* req) 
	{
		Data* data = static_cast<Data*>(req->data);
		//�������
		DWORD dwVersion = 0;
		DWORD dwStatus = 0;
		PUSIResult result = AbortStep(data->addr);//��ֹ��ǰ����
		ERROR_HANDLE(FALSE, "AbortStep");

		ClearExtStop(data->addr, 1); //����ж�
		ERROR_HANDLE(FALSE, "ClearExtStop");

		result = SetMaxCurrent(data->addr, 2000); //����������
		ERROR_HANDLE(TRUE, "");
		result = SetMicroStep(data->addr, 16); //����ϸ����
		ERROR_HANDLE(TRUE, "");
		result = SetAccCoff(data->addr, 5); //���ü��ٶ�ϸ��5
		ERROR_HANDLE(TRUE, "");
		result = SetDecCoff(data->addr, 5); //���ü��ٶ�ϸ��5
		ERROR_HANDLE(TRUE, "");

		result = SetExtStopEnable(data->addr, 1);//ʹ���ⲿ�����ж�1
		ERROR_HANDLE(FALSE, "SetExtStopEnable");

		result = SetExtStopTrigerMode(data->addr, 0);//ʹ���ⲿ�����ж�1
		ERROR_HANDLE(FALSE, "SetExtStopTrigerMode");

		result = GetIOValue(data->addr, &dwStatus); //��ȡ������״̬
		ERROR_HANDLE(FALSE, "GetControllerStatus1");
		if ((dwStatus & 0x0400))
		{
			//����ж�
			result = ClearExtStop(data->addr, 1); //����ж�
			ERROR_HANDLE(FALSE, "ClearExtStop");
			//����ʹ��
			result = SetVelocity(data->addr, data->zeroSpeed);//����ת���ٶ�
			ERROR_HANDLE(FALSE, "SetVelocity");

			result = SetStopVelocity(data->addr, data->zeroSpeed);//����ֹͣ�ٶ�
			ERROR_HANDLE(FALSE, "SetStopVelocity");

			result = SetStartVelocity(data->addr, data->zeroSpeed);//����ֹͣ�ٶ�
			ERROR_HANDLE(FALSE, "SetStartVelocity");
			//����ת��
			result = SetDirection(data->addr, MOTOR_FORWARD/* MOTOR_REVERSE*/); //����ת������
			ERROR_HANDLE(FALSE, "SetDirection");

			result = StartStep(data->addr, 0x7FFFFFFF); //��������
			ERROR_HANDLE(FALSE, "StartStep");

			result = GetControllerStatus1(data->addr, &dwStatus); //��ȡ������״̬
			ERROR_HANDLE(FALSE, "GetControllerStatus1");
			while (dwStatus & 0x1)   //�ж�æ״̬��æ��ȴ�ת�����
			{
				Sleep(50);
				result = GetControllerStatus1(data->addr, &dwStatus); //��ȡ������״̬
				ERROR_HANDLE(FALSE, "GetControllerStatus1");
			}

			//����ж�
			result = ClearExtStop(data->addr, 1); //����ж�
			ERROR_HANDLE(FALSE, "ClearExtStop");

			result = ClearExtStop(data->addr, 1); //����ж�
			ERROR_HANDLE(FALSE, "ClearExtStop");
			//����ת��
			result = SetDirection(data->addr, /*MOTOR_FORWARD*/ MOTOR_REVERSE); //����ת������
			ERROR_HANDLE(FALSE, "SetDirection");

			result = StartStep(data->addr, data->offset); //��������
			ERROR_HANDLE(FALSE, "StartStep");

			result = GetControllerStatus1(data->addr, &dwStatus); //��ȡ������״̬
			ERROR_HANDLE(FALSE, "GetControllerStatus1");
			while ((dwStatus & 0x1) && data->addr)   //�ж�æ״̬��æ��ȴ�ת�����
			{
				Sleep(50);
				result = GetControllerStatus1(data->addr, &dwStatus); //��ȡ������״̬
				ERROR_HANDLE(FALSE, "GetControllerStatus1");
			}
		}
		else 
		{
			//����λ����Ҫȥ�������λ
			//����ж�
			result = ClearExtStop(data->addr, 1); //����ж�
			ERROR_HANDLE(FALSE, "ClearExtStop");

			result = ClearExtStop(data->addr, 1); //����ж�
			ERROR_HANDLE(FALSE, "ClearExtStop");
			//����ת��
			result = SetDirection(data->addr, /*MOTOR_FORWARD*/ MOTOR_REVERSE); //����ת������
			ERROR_HANDLE(FALSE, "SetDirection");

			result = StartStep(data->addr, data->offset); //��������
			ERROR_HANDLE(FALSE, "StartStep");

			result = GetControllerStatus1(data->addr, &dwStatus); //��ȡ������״̬
			ERROR_HANDLE(FALSE, "GetControllerStatus1");
			while (dwStatus & 0x1)   //�ж�æ״̬��æ��ȴ�ת�����
			{
				Sleep(50);
				result = GetControllerStatus1(data->addr, &dwStatus); //��ȡ������״̬
				ERROR_HANDLE(FALSE, "GetControllerStatus1");
			}
		}

		result = SetMotorPosition(data->addr, 0);//
		ERROR_HANDLE(FALSE, "SetMotorPosition");

		result = SetMicroStep(data->addr, 128); //����ϸ����
		ERROR_HANDLE(TRUE, "");

		//����ʹ��
		result = SetVelocity(data->addr, data->speed);//����ת���ٶ�                    ��λ�ĳɶ�/s  ����������� 
		ERROR_HANDLE(FALSE, "SetVelocity");

		result = SetStopVelocity(data->addr, data->speed);//����ֹͣ�ٶ�                ��λ�ĳɶ�/s  �����������
		ERROR_HANDLE(FALSE, "SetStopVelocity");

		result = SetStartVelocity(data->addr, data->speed);//����ֹͣ�ٶ�				  ��λ�ĳɶ�/s  �����������
		ERROR_HANDLE(FALSE, "SetStartVelocity");
		//printf("pusiReset: %d \r\n", cout_zero_equp);
		gohome_info.Delete(std::to_string(data->addr));
		cout_zero_equp--;
		
	}

	/**************************************************
	***@author:cui_sh
	***@brief:ִ�й���������Ϸ���
	***@time:
	***@version:
	**************************************************/
	void PUSI_FUN::afterPusiReset(uv_work_t* req)
	{
		//printf("afterPusiReset: %d \r\n", cout_zero_equp);
		Data* data = static_cast<Data*>(req->data);
		
		/* �ص���ɺ������Դ */
		delete data;
		data = NULL;
		delete req;
		req = NULL;

	}
	/**************************************************
	***@author:cui_sh
	***@brief:�򿪵������
	***@time:
	***@version:
	**************************************************/
	bool PUSI_FUN::OPEN(int port)
	{
		//printf("OPEN::opend:%d\r\n", port);
		if (RET_SUCCESS == OpenPort(port, CBR_9600))
		{
			//printf("RET_SUCCESS%d\r\n", port);
			is_connection = true;
			return true;
		}
		return false;
	}

	/**************************************************
	***@author:cui_sh
	***@brief:�رյ������
	***@time:
	***@version:
	**************************************************/
	bool PUSI_FUN::CLOSE()
	{
		is_connection = false;
		return ClosePort();
	}

	bool PUSI_FUN::GET_CONNECT_STATUS() 
	{
		return is_connection;
	}

	bool PUSI_FUN::GET_WORK_STATUS(int addr)
	{
		if (GET_CONNECT_STATUS())
		{
			DWORD dwStatus = 0;
			GetControllerStatus1(addr, &dwStatus);
			if (dwStatus & 0x1) {
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return false;
		}
		return is_connection;
	}


	/**************************************************
	***@author:cui_sh
	***@brief:�˶�ǰ״̬���
	***@time:
	***@version:
	**************************************************/
	bool PUSI_FUN::MOVE_CHECK(int addr, int dir, int rate, neb::CJsonObject controlData)
	{
		//��ȡ��ǰ��Ҫ���ƶ������ӦӦ����ʲô��
		std::string axis;
		controlData.Get(std::to_string(addr), axis);//��ȡ��Ҫ���Ƶ���

		//��ȡ��Ӧ��ĳ���
		int axisLength;
		controlData.Get(axis, axisLength);//ȡ�ö�Ӧ��ĳ���

		//���ݷ����жϵ�ǰ��Ҫ�˶��ķ����Ƿ����

		if (dir == MOVE_DIRECTION_1)
		{
			//����㷽���˶�
			//��ǰ����Ƿ�������λ���أ���û��������˶�
			DWORD dwStatus = 0;
			PUSIResult result = GetIOValue(addr, &dwStatus); //��ȡ������״̬
			//ERROR_HANDLE(FALSE, "GetControllerStatus1");
			if (result != RET_SUCCESS)
			{
				return false;
			}
			if ((dwStatus & 0x0400))
			{
				return true;
			}
			return false;
		}
		else if (dir == MOVE_DIRECTION_2)
		{
			//����㷽�����˶�
			int position = 0;
			PUSIResult result = GetMotorPosition(addr, &position); //��������
			//ERROR_HANDLE(FALSE, "GetMotorPosition");
			if (result != RET_SUCCESS)
			{
				return false;
			}
			//printf("position:%d  axisLength:%d \r\n", position, axisLength);
			if (abs(position) < axisLength) 
			{
				return true;
			}
			return false;
		}
		return false;
	}

	/**************************************************
	***@author:cui_sh
	***@brief:�򿪵������
	***@time:
	***@version:
	**************************************************/
	void PUSI_FUN::MOVE_CONTROL(int addr, int dir, int rate,int axisSpeed,int axisLength)
	{
		//����˶�����
		//�������������˶��Ĳ���
		//���õ���˶�����
		//��ʼ�˶���ֱ���û�ֹͣ���ߣ����Ｋ�ޣ�

		//��ȡ��ǰ��Ҫ���ƶ������ӦӦ����ʲô��
		//std::string axis;
		//controlData.Get(std::to_string(addr), axis);//��ȡ��Ҫ���Ƶ���

		//��ȡ��Ӧ��ĳ���
		//int axisLength;
		//controlData.Get(axis, axisLength);//ȡ�ö�Ӧ��ĳ���

		//char buf[24];
		//sprintf(buf,"%s%s", axis.c_str(),"_Speed");
		//int speed;
		//controlData.Get(buf, speed);//ȡ�ö�Ӧ����ٶ�

		//���õ�ǰ����ٶ�
		axisSpeed *= rate;
		//printf("axisSpeed=%d axisLength=%d\r\n", axisSpeed, axisLength);

		PUSIResult result = AbortStep(addr);//��ֹ��ǰ����
		ERROR_HANDLE(FALSE, "AbortStep");

		ClearExtStop(addr, 1); //����ж�
		ERROR_HANDLE(FALSE, "ClearExtStop");

		result = SetMaxCurrent(addr, 2000); //����������
		ERROR_HANDLE(TRUE, "");
		result = SetMicroStep(addr, 128); //����ϸ����
		ERROR_HANDLE(TRUE, "");
		result = SetAccCoff(addr, 5); //���ü��ٶ�ϸ��5
		ERROR_HANDLE(TRUE, "");
		result = SetDecCoff(addr, 5); //���ü��ٶ�ϸ��5
		ERROR_HANDLE(TRUE, "");

		result = SetExtStopEnable(addr, 1);//ʹ���ⲿ�����ж�1
		ERROR_HANDLE(FALSE, "SetExtStopEnable");

		result = SetExtStopTrigerMode(addr, 0);//ʹ���ⲿ�����ж�1
		ERROR_HANDLE(FALSE, "SetExtStopTrigerMode");

		//����ʹ��
		result = SetVelocity(addr, axisSpeed);//����ת���ٶ�
		ERROR_HANDLE(FALSE, "SetVelocity");

		result = SetStopVelocity(addr, axisSpeed);//����ֹͣ�ٶ�
		ERROR_HANDLE(FALSE, "SetStopVelocity");

		result = SetStartVelocity(addr, axisSpeed);//����ֹͣ�ٶ�
		ERROR_HANDLE(FALSE, "SetStartVelocity");
		printf("//ִ�в�������\r\n");
		//ִ�в�������
		if (dir == MOVE_DIRECTION_1)
		{
			//����㷽���˶�
			result = SetDirection(addr, MOTOR_FORWARD/* MOTOR_REVERSE*/); //����ת������
			ERROR_HANDLE(FALSE, "SetDirection");
			MOVE_TO_L(addr);
		}
		else if (dir == MOVE_DIRECTION_2)
		{
			//����㷽�����˶�

			result = SetDirection(addr, MOTOR_REVERSE /* MOTOR_FORWARD*/); //����ת������
			ERROR_HANDLE(FALSE, "SetDirection");
			MOVE_TO_R(addr, axisLength);

		}
		
		

	}
	/**************************************************
	***@author:cui_sh
	***@brief:�˶�����
	***@time:
	***@version:
	**************************************************/
	bool PUSI_FUN::MOVE_CTRL_FLG(bool value) 
	{
		is_move = value;
		return true;
	}

	/**************************************************
	***@author:cui_sh
	***@brief:�������㷽���ƶ�
	***@time:
	***@version:
	**************************************************/
	void PUSI_FUN::MOVE_TO_L(int addr)
	{
		DWORD dwStatus = 0;
		PUSIResult result = StartStep(addr, 0x7FFFFFFF); //��������
		ERROR_HANDLE(FALSE, "StartStep");

		/*while (is_move)
		{
			Sleep(80);
		}*/

		result = GetControllerStatus1(addr, &dwStatus); //��ȡ������״̬
		ERROR_HANDLE(FALSE, "GetControllerStatus1");
		while ((dwStatus & 0x1)&&(is_move))   //�ж�æ״̬��æ��ȴ�ת�����
		{
			
			result = GetControllerStatus1(addr, &dwStatus); //��ȡ������״̬
			ERROR_HANDLE(FALSE, "GetControllerStatus1");
			Sleep(50);
		}

		//����ж�
		result = ClearExtStop(addr, 1); //����ж�
		ERROR_HANDLE(FALSE, "ClearExtStop");

		result = ClearExtStop(addr, 1); //����ж�
		ERROR_HANDLE(FALSE, "ClearExtStop");

		//
		AbortStep(addr);//��ֹ��ǰ����
		ERROR_HANDLE(FALSE, "AbortStep");
	}
	/**************************************************
	***@author:cui_sh
	***@brief:�������㷴�����ƶ�
	***@time:
	***@version:
	**************************************************/
	void PUSI_FUN::MOVE_TO_R(int addr, int maxLength)
	{
		DWORD dwStatus = 0;
		PUSIResult result = StartStep(addr, maxLength); //��������
		ERROR_HANDLE(FALSE, "StartStep");
		//printf("MOVE_TO_R maxLength=%d\r\n", maxLength);

		result = GetControllerStatus1(addr, &dwStatus); //��ȡ������״̬
		ERROR_HANDLE(FALSE, "GetControllerStatus1");
		while ((dwStatus & 0x1) && (is_move))   //�ж�æ״̬��æ��ȴ�ת�����
		{

			result = GetControllerStatus1(addr, &dwStatus); //��ȡ������״̬
			ERROR_HANDLE(FALSE, "GetControllerStatus1");
			Sleep(50);
		}

		//
		AbortStep(addr);//��ֹ��ǰ����
		ERROR_HANDLE(FALSE, "AbortStep");
	}


	/**************************************************
	***@author:cui_sh
	***@brief:�������㷴�����ƶ�
	***@time:
	***@version:
	**************************************************/
	void PUSI_FUN::SCAN_GO(int addr, int maxLength)
	{
		DWORD dwStatus = 0;
		PUSIResult result = StartStep(addr, maxLength); //��������
		ERROR_HANDLE(FALSE, "StartStep");
		//printf("SCAN_TO_R: maxLength %d\r\n", maxLength);
		result = GetControllerStatus1(addr, &dwStatus); //��ȡ������״̬
		ERROR_HANDLE(FALSE, "GetControllerStatus1");
		while ((dwStatus & 0x1))   //�ж�æ״̬��æ��ȴ�ת�����
		{

			result = GetControllerStatus1(addr, &dwStatus); //��ȡ������״̬
			ERROR_HANDLE(FALSE, "GetControllerStatus1");
			Sleep(50);
		}

		//
		AbortStep(addr);//��ֹ��ǰ����
		ERROR_HANDLE(FALSE, "AbortStep");
	}

	/**************************************************
	***@author:cui_sh
	***@brief:������ɨ
	***@time:
	***@version:
	**************************************************/
	void PUSI_FUN::SCAN_CONTROL(int addr,int length)
	{
		//��ѡ�õ���ɨά�ȣ�X,Y,Z��
		
		//�����ʼλ�ú�ֹͣλ��
		//���ɨ���ٶ�

		//�������
		
		//�˶���ָ������ʼλ��
		

		//������ɨ
		//����ֹͣλ��
		//SCAN_TO_R(addr, length);
		//�رհ�ɨ
	}

	/**************************************************
	***@author:cui_sh
	***@brief:������ɨǰ׼��
	***@time:
	***@version:
	**************************************************/
	void PUSI_FUN::SCAN_READY(int addr, int direction,int speed,int zeroSpeed,int offset, int length)
	{
		//�������
		DWORD dwVersion = 0;
		DWORD dwStatus = 0;
		PUSIResult result = AbortStep(addr);//��ֹ��ǰ����
		ERROR_HANDLE(FALSE, "AbortStep");
		ClearExtStop(addr, 1); //����ж�
		ERROR_HANDLE(FALSE, "ClearExtStop");
		result = SetMaxCurrent(addr, 2000); //����������
		ERROR_HANDLE(TRUE, "");
		result = SetMicroStep(addr, 16); //����ϸ����
		ERROR_HANDLE(TRUE, "");
		result = SetAccCoff(addr, 5); //���ü��ٶ�ϸ��5
		ERROR_HANDLE(TRUE, "");
		result = SetDecCoff(addr, 5); //���ü��ٶ�ϸ��5
		ERROR_HANDLE(TRUE, "");
		result = SetExtStopEnable(addr, 1);//ʹ���ⲿ�����ж�1
		ERROR_HANDLE(FALSE, "SetExtStopEnable");

		result = SetExtStopTrigerMode(addr, 0);//ʹ���ⲿ�����ж�1
		ERROR_HANDLE(FALSE, "SetExtStopTrigerMode");
		//����ʹ��
		result = SetVelocity(addr, zeroSpeed);//����ת���ٶ�
		ERROR_HANDLE(FALSE, "SetVelocity");

		result = SetStopVelocity(addr, zeroSpeed);//����ֹͣ�ٶ�
		ERROR_HANDLE(FALSE, "SetStopVelocity");

		result = SetStartVelocity(addr, zeroSpeed);//����ֹͣ�ٶ�
		ERROR_HANDLE(FALSE, "SetStartVelocity");
		//MOTOR_READY_DIRECTIONFORWARD
		//MOTOR_READY_DIRECTIONREVERSE
		//����ת������
		if (direction == MOTOR_READY_DIRECTIONFORWARD)
		{
			result = SetDirection(addr, MOTOR_FORWARD/* MOTOR_REVERSE*/); //����ת������
			ERROR_HANDLE(FALSE, "SetDirection");
		}
		else if(direction == MOTOR_READY_DIRECTIONREVERSE)
		{
			result = SetDirection(addr, MOTOR_REVERSE /* MOTOR_FORWARD*/); //����ת������
			ERROR_HANDLE(FALSE, "SetDirection");
		}
		result = StartStep(addr, offset+length); //��������
		ERROR_HANDLE(FALSE, "StartStep");
		result = GetControllerStatus1(addr, &dwStatus); //��ȡ������״̬
		ERROR_HANDLE(FALSE, "GetControllerStatus1");
		while (dwStatus & 0x1)   //�ж�æ״̬��æ��ȴ�ת�����
		{
			
			result = GetControllerStatus1(addr, &dwStatus); //��ȡ������״̬
			ERROR_HANDLE(FALSE, "GetControllerStatus1");
			Sleep(50);
		}
		//����ת������
		if (direction == MOTOR_READY_DIRECTIONFORWARD)
		{
			result = SetDirection(addr, MOTOR_REVERSE /* MOTOR_FORWARD*/);
			ERROR_HANDLE(FALSE, "SetDirection");
		}
		//else
		//{
		//	result = SetDirection(addr, MOTOR_FORWARD  /* MOTOR_REVERSE*/);
		//	ERROR_HANDLE(FALSE, "SetDirection");
		//}
		result = SetMicroStep(addr, 128); //����ϸ����
		ERROR_HANDLE(TRUE, "");

		//�����ٶ�
		result = SetVelocity(addr, speed);//����ת���ٶ�                    ��λ�ĳɶ�/s  ����������� 
		ERROR_HANDLE(FALSE, "SetVelocity");

		result = SetStopVelocity(addr, speed);//����ֹͣ�ٶ�                ��λ�ĳɶ�/s  �����������
		ERROR_HANDLE(FALSE, "SetStopVelocity");

		result = SetStartVelocity(addr, speed);//����ֹͣ�ٶ�				  ��λ�ĳɶ�/s  �����������
		ERROR_HANDLE(FALSE, "SetStartVelocity");

		////�������
		//neb::CJsonObject addrList;
		//addrList.AddEmptySubArray("addrList");
		//neb::CJsonObject item;
		//item.Add("addr", addr);
		//item.Add("speed", speed);
		//item.Add("zeroSpeed", zeroSpeed);
		//item.Add("offset", offset+ length);
		//addrList["addrList"].Add(item);

		////printf("SCAN_READY %d\r\n", addrList["addrList"].GetArraySize());
		//go_to_home(addrList);

		////������ɺ󵽴�ָ������ʼλ��
		////SCAN_TO_R(addr, length);
	}
}