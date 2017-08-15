/**
* $Id: WGBasicTest.cpp 2015-04-29 22:20:40 karl CSN ������ $
*
* �Ž������� �̱���Э�� ���԰���
* V1.1 �汾  2013-11-05 15:02:02  
*            ��������:  ��ѯ������״̬ 
*                       ��ȡ����ʱ��
*                       ��������ʱ��
*                       ��ȡָ�������ŵļ�¼
*                       �����Ѷ�ȡ���ļ�¼������
*                       ��ȡ�Ѷ�ȡ���ļ�¼������
*                       Զ�̿���
*                       Ȩ����ӻ��޸�
*                       Ȩ��ɾ��(����ɾ��)
*                       Ȩ�����(ȫ�����)
*                       Ȩ��������ȡ
*                       Ȩ�޲�ѯ
*                       �����ſ��Ʋ���(����/��ʱ)
*                       ��ȡ�ſ��Ʋ���(����/��ʱ)

*                       ���ý��շ�������IP�Ͷ˿�
*                       ��ȡ���շ�������IP�Ͷ˿�
*                       
*
*                       ���շ�������ʵ�� (��61005�˿ڽ�������) -- ����� һ��Ҫע�����ǽ���� ����������������ݵ�.
* V1.2 2013-11-07 12:40:49
*                 �޸���������ʱ����֤
*                 �����ݰ�����, Type, �������˿�, �����ʶ�̶�����WGPacketShort��
* V2.5 2015-04-29 20:41:30 ���� V6.56�����汾 �ͺ���0x19��Ϊ0x17
*/

#include "ace/INET_Addr.h"
#include "ace/SOCK_Dgram.h"
#include "ace/Time_Value.h"
#include "ace/SOCK_CODgram.h"
#include "ace/SOCK_Dgram_Bcast.h"
#include <time.h>   
#include <stdio.h>
#include <fstream>

#include "zht_WGbacic.h"
class WGPacketShort {				//�̱���Э��
public:
	const static unsigned int	 WGPacketSize = 64;			    //���ĳ���
	//2015-04-29 22:22:41 const static unsigned char	 Type = 0x19;					//����
	const static unsigned char	 Type = 0x17;		//2015-04-29 22:22:50			//����
	
	const static unsigned int    SpecialFlag =0x55AAAA55;       //�����ʶ ��ֹ�����

	unsigned char	 functionID;		    //���ܺ�
	unsigned int	 iDevSn;                //�豸���к� 4�ֽ�
	unsigned char    data[56];              //56�ֽڵ����� [����ˮ��]
	unsigned int     ControllerPort;        //�������˿�
	unsigned char    recv[WGPacketSize];    //���յ�������
	WGPacketShort(void)
	{
		Reset();
	}
	void Reset()  //���ݸ�λ
	{
		memset(data,0,sizeof(data));
	}
	void toByte(char* buff, size_t buflen) //����64�ֽ�ָ���
	{
		if (buflen == WGPacketSize)
		{
			memset(buff,0,sizeof(buff));
			buff[0] = Type;
			buff[1] = functionID;
			memcpy(&(buff[4]),&(iDevSn), 4);
			memcpy(&(buff[8]),data,sizeof(data));
		}
	}
	int run(ACE_SOCK_CODgram udp)  //ͨ��ָ����UDP����ָ�� ���շ�����Ϣ
	{
		unsigned char buff[WGPacketSize];
		int errcnt =0;
		WGPacketShort::sequenceId++;
		memset(buff,0,sizeof(buff));
		buff[0] = Type;
		buff[1] = functionID;
		memcpy(&(buff[4]),&(iDevSn), 4);
		memcpy(&(buff[8]),data,sizeof(data));
		unsigned int currentSequenceId = WGPacketShort::sequenceId;
		memcpy(&(buff[40]),&(currentSequenceId), 4);
		int tries =3;
		do 
		{
			if (-1 == udp.send (buff, WGPacketSize))
			{
				return -1;
			}
			else 
			{
				ACE_INET_Addr your_addr;
				ACE_Time_Value recvTimeout(0, 400*1000);
				size_t recv_cnt = udp.recv(recv, WGPacketSize, &recvTimeout); 
				if (recv_cnt == WGPacketSize)
				{
					//��ˮ��
					unsigned int  sequenceIdReceived=0;
					memcpy(&sequenceIdReceived, &(recv[40]),4);

					if ((recv[0]== Type) //����һ��
						&& (recv[1]== functionID) //���ܺ�һ��
						&& (sequenceIdReceived == currentSequenceId) )  //���кŶ�Ӧ
					{
						return 1;
					}
					else
					{
						errcnt++;
					}
				}
			}
		} while(tries-- >0); //��������

		return -1;
	}
	static unsigned int sequenceIdSent()// ��󷢳���ˮ��
	{
		return sequenceId; // ��󷢳���ˮ��
	}
private:
	static  unsigned int     sequenceId;     //���к�	
};
unsigned int WGPacketShort::sequenceId = 0;  //��ˮ��ֵ

unsigned char GetHex(int val) //��ȡHexֵ, ��Ҫ��������ʱ���ʽ
{
	return ((val % 10) + (((val -(val % 10)) / 10)%10) *16);
}
void log(char* info)  //��־��Ϣ
{
	//stdout<< info;
	//Console.WriteLine(info);
	//printf("%s\r\n", info);
	time_t nowtime;     
	struct tm* ptm;     
	time(&nowtime);     
	ptm = localtime(&nowtime);     
	printf("%02d:%02d:%02d %s\r\n", ptm->tm_hour, ptm->tm_min, ptm->tm_sec, info);
}
int testBasicFunction(char *ControllerIP, unsigned int controllerSN);  //�������ܲ���
int testWatchingServer(char *ControllerIP, unsigned int controllerSN, char *watchServerIP,int watchServerPort);  //���շ���������
int WatchingServerRuning (char *watchServerIP,int watchServerPort);   //2013-11-05 13:08:39 ������������״̬

#include<iostream>
using namespace std;
#include <windows.h>

//typedef void(*FUNPTR_CALLBACK)(int nID, int eventType, void* param);



char* RecordDetails[] =
        {
//��¼ԭ�� (������ SwipePass ��ʾͨ��; SwipeNOPass��ʾ��ֹͨ��; ValidEvent ��Ч�¼�(�簴ť �Ŵ� �������뿪��); Warn �����¼�)
//����  ����   Ӣ������  ��������
"1","SwipePass","Swipe","ˢ������",
"2","SwipePass","Swipe Close","ˢ����",
"3","SwipePass","Swipe Open","ˢ����",
"4","SwipePass","Swipe Limited Times","ˢ������(���޴�)",
"5","SwipeNOPass","Denied Access: PC Control","ˢ����ֹͨ��: ���Կ���",
"6","SwipeNOPass","Denied Access: No PRIVILEGE","ˢ����ֹͨ��: û��Ȩ��",
"7","SwipeNOPass","Denied Access: Wrong PASSWORD","ˢ����ֹͨ��: ���벻��",
"8","SwipeNOPass","Denied Access: AntiBack","ˢ����ֹͨ��: ��Ǳ��",
"9","SwipeNOPass","Denied Access: More Cards","ˢ����ֹͨ��: �࿨",
"10","SwipeNOPass","Denied Access: First Card Open","ˢ����ֹͨ��: �׿�",
"11","SwipeNOPass","Denied Access: Door Set NC","ˢ����ֹͨ��: ��Ϊ����",
"12","SwipeNOPass","Denied Access: InterLock","ˢ����ֹͨ��: ����",
"13","SwipeNOPass","Denied Access: Limited Times","ˢ����ֹͨ��: ��ˢ����������",
"14","SwipeNOPass","Denied Access: Limited Person Indoor","ˢ����ֹͨ��: ������������",
"15","SwipeNOPass","Denied Access: Invalid Timezone","ˢ����ֹͨ��: �����ڻ�����Чʱ��",
"16","SwipeNOPass","Denied Access: In Order","ˢ����ֹͨ��: ��˳���������",
"17","SwipeNOPass","Denied Access: SWIPE GAP LIMIT","ˢ����ֹͨ��: ˢ�����Լ��",
"18","SwipeNOPass","Denied Access","ˢ����ֹͨ��: ԭ����",
"19","SwipeNOPass","Denied Access: Limited Times","ˢ����ֹͨ��: ˢ����������",
"20","ValidEvent","Push Button","��ť����",
"21","ValidEvent","Push Button Open","��ť��",
"22","ValidEvent","Push Button Close","��ť��",
"23","ValidEvent","Door Open","�Ŵ�[�Ŵ��ź�]",
"24","ValidEvent","Door Closed","�Źر�[�Ŵ��ź�]",
"25","ValidEvent","Super Password Open Door","�������뿪��",
"26","ValidEvent","Super Password Open","�������뿪",
"27","ValidEvent","Super Password Close","���������",
"28","Warn","Controller Power On","�������ϵ�",
"29","Warn","Controller Reset","��������λ",
"30","Warn","Push Button Invalid: Disable","��ť������: ��ť����",
"31","Warn","Push Button Invalid: Forced Lock","��ť������: ǿ�ƹ���",
"32","Warn","Push Button Invalid: Not On Line","��ť������: �Ų�����",
"33","Warn","Push Button Invalid: InterLock","��ť������: ����",
"34","Warn","Threat","в�ȱ���",
"35","Warn","Threat Open","в�ȱ�����",
"36","Warn","Threat Close","в�ȱ�����",
"37","Warn","Open too long","�ų�ʱ��δ�ر���[�Ϸ����ź�]",
"38","Warn","Forced Open","ǿ�д��뱨��",
"39","Warn","Fire","��",
"40","Warn","Forced Close","ǿ�ƹ���",
"41","Warn","Guard Against Theft","��������",
"42","Warn","7*24Hour Zone","����ú���¶ȱ���",
"43","Warn","Emergency Call","�������ȱ���",
"44","RemoteOpen","Remote Open Door","����ԱԶ�̿���",
"45","RemoteOpen","Remote Open Door By USB Reader","������ȷ��������Զ�̿���"
        };
 char* getReasonDetailChinese(int Reason) //����
        {
            if (Reason > 45)
            {
                return "";
            }
            if (Reason <= 0)
            {
                return "";
            }
            return RecordDetails[(Reason - 1) * 4 + 3]; //������Ϣ
        }

        char* getReasonDetailEnglish(int Reason) //Ӣ������
        {
            if (Reason > 45)
            {
                return "";
            }
            if (Reason <= 0)
            {
                return "";
            }
            return RecordDetails[(Reason - 1) * 4 + 2]; //Ӣ����Ϣ
        }
        /// <summary>
        /// ��ʾ��¼��Ϣ
        /// </summary>
        /// <param name="recv"></param>
        void displayRecordInformation(unsigned char* recv)
        {
			//	  	���һ����¼����Ϣ		
		//8-11	���һ����¼��������
		//(=0��ʾû�м�¼)	4	0x00000000
		int recordIndex =0;
		memcpy(&recordIndex, &(recv[8]),4);

		//12	��¼����
		//0=�޼�¼
		//1=ˢ����¼
		//2=�Ŵ�,��ť, �豸����, Զ�̿��ż�¼
		//3=������¼	1	
		int recordType = recv[12];

		//13	��Ч��(0 ��ʾ��ͨ��, 1��ʾͨ��)	1	
		int recordValid = recv[13];

		//14	�ź�(1,2,3,4)	1	
		int recordDoorNO = recv[14];

		//15	����/����(1��ʾ����, 2��ʾ����)	1	0x01
		int recordInOrOut = recv[15];

		//16-19	����(������ˢ����¼ʱ)
		//����(�������ͼ�¼)	4	
		long long recordCardNO = 0;
		memcpy(&recordCardNO, &(recv[16]),4);

		//20-26	ˢ��ʱ��:
		//������ʱ���� (����BCD��)������ʱ�䲿�ֵ�˵��
		char recordTime[]="2000-01-01 00:00:00";
		sprintf(recordTime,"%02X%02X-%02X-%02X %02X:%02X:%02X", 
			recv[20],recv[21],recv[22],recv[23],recv[24],recv[25],recv[26]);

		//2012.12.11 10:49:59	7	
		//27	��¼ԭ�����(���Բ� ��ˢ����¼˵��.xls���ļ���ReasonNO)
		//��������Ϣ����	1	
		int reason = recv[27];

              //0=�޼�¼
            //1=ˢ����¼
            //2=�Ŵ�,��ť, �豸����, Զ�̿��ż�¼
            //3=������¼	1	
            //0xFF=��ʾָ������λ�ļ�¼�ѱ����ǵ���.  ��ʹ������0, ȡ������һ����¼������ֵ
            if (recordType == 0)
            {
				 printf("����λ=%u  �޼�¼\r\n", recordIndex);
            }
            else if (recordType == 0xff)
            {
                log(" ָ������λ�ļ�¼�ѱ����ǵ���,��ʹ������0, ȡ������һ����¼������ֵ");
            }
            else if (recordType == 1) //2015-06-10 08:49:31 ��ʾ��¼����Ϊ���ŵ�����
            {
                //����
                printf("����λ=%u\r\n", recordIndex);
                printf("  ���� = %u\r\n", recordCardNO);
                printf("  �ź� = %u\r\n", recordDoorNO);
                printf("  ���� = %s\r\n", recordInOrOut == 1 ? "����" : "����");
                printf("  ��Ч = %s\r\n", recordValid == 1 ? "ͨ��" : "��ֹ");
                printf("  ʱ�� = %s\r\n", recordTime);
                printf("  ���� = %s\r\n", getReasonDetailChinese(reason));
            }
            else if (recordType == 2)
            {
                //��������
                //�Ŵ�,��ť, �豸����, Զ�̿��ż�¼
                printf("����λ=%u  ��ˢ����¼\r\n ", recordIndex);
                printf("  ��� = %u\r\n", recordCardNO);
                printf("  �ź� = %u\r\n", recordDoorNO);
                printf("  ʱ�� = %s\r\n", recordTime);
                printf("  ���� = %s\r\n", getReasonDetailChinese(reason));
         }
            else if (recordType == 3)
            {
                //��������
                //������¼
               
                printf("����λ=%u  ������¼\r\n ", recordIndex);
                printf("  ��� = %u\r\n", recordCardNO);
                printf("  �ź� = %u\r\n", recordDoorNO);
                printf("  ʱ�� = %s\r\n", recordTime);
                printf("  ���� = %s\r\n", getReasonDetailChinese(reason));
           }
        }


class ControlInfo
{
public:
	int ID;
	int SN;
	char controlIP[16];
	int controlPort;
	int localPort;
	WGPacketShort pkt;
	int ConnectFlag;   // 1 connect 0 close
	FUNPTR_CALLBACK fbackcall;
	char localIP[16];
	//ACE_INET_Addr controller_addr;
	ACE_SOCK_CODgram udp;

	//ACE_INET_Addr server_addr(static_cast<u_short> (watchServerPort), watchServerIP);
	ACE_SOCK_Dgram_Bcast serUdp;//(server_addr);
	void setAddr()
	{
		ACE_INET_Addr controller_addr(controlPort,controlIP);
		///*controller_addr.set_address(controlIP,strlen(controlIP));
		//*/controller_addr.set_port_number(controlPort);
		cout << controlIP << endl;
		cout << controlPort << endl;
		if (0 != udp.open(controller_addr))
		{
			//��������ЧIP			
			cout << "��������ЧIP" << endl;
		}
	}
	void setSerUdp(ACE_SOCK_Dgram_Bcast udp)
	{
		serUdp = udp;
	}
};
ControlInfo MC[20];

void log(char *info,int data)  //��־��Ϣ
{
	//txtInfo.Text += string.Format("{0}\r\n", info);
	//txtInfo.AppendText(string.Format("{0}\r\n", info));
	//txtInfo.AppendText(string.Format("{0} {1}\r\n", DateTime.Now.ToString("HH:mm:ss"), info)); //2015-11-03 20:55:49 ��ʾʱ��
	//txtInfo.ScrollToCaret();//��������괦
	//Application.DoEvents();
	cout << info << data << endl;
}
int byteToLong(unsigned char *buff, int start, int len)
{
	int val = 0;
	for (int i = 0; i < len && i < 4; i++)
	{
		int lng = buff[i + start];
		val += (lng << (8 * i));
	}
	return val;
}
long getSnForControl(WGPacketShort pkt,int id)
{
	int ret = 0;
	int controllerSN = 0;

	pkt.Reset();
	pkt.functionID = 0x94;

	ret = pkt.run(MC[id].udp);
	log("��ȡ�豸��...");
	if (ret > 0)
	{		
		controllerSN = (int)byteToLong(pkt.recv, 4, 4);		
		log("1.1 ��ȡ�豸��.....controllerSN = ", controllerSN);
		//get sn
	}
	return controllerSN;
}
//ControllerIP ������IP��ַ
//controllerSN ���������к�
int zht_InitPort(int id, int iPort, int gPort, char* ControllerIP)
{
	int controllerSN = 0;
	int ret = 0;
	int success = 0;  //0 ʧ��, 1��ʾ�ɹ�    
	if (id > 10 || id < 0)
	{
		return -1;
	}
	MC[id].pkt.ControllerPort = gPort;
	MC[id].controlPort = gPort;
	cout << MC[id].controlPort << endl;
	memcpy(MC[id].controlIP, ControllerIP, strlen(ControllerIP));
	MC[id].setAddr();
	controllerSN = getSnForControl(MC[id].pkt,id);//223209404
	if (controllerSN == 0)
	{
		return -1;
	}
	MC[id].pkt.iDevSn = controllerSN;

	MC[id].SN = controllerSN;
	MC[id].ID = id;	
	MC[id].controlPort = gPort;
	MC[id].localPort = iPort;
	MC[id].ConnectFlag = 1;
	return controllerSN;
}
int zht_ClosePort(int hComm, int id)
{
	if (id > 10 || id < 0)
	{
		return -1;
	}
	if (MC[id].SN != hComm)
	{
		return -1;
	}
	MC[id].ConnectFlag = 0;  //close	
	int aa = MC[id].udp.close();	
	int bb = MC[id].serUdp.close();	
	cout << aa << "bb" << bb << endl;
	cout << "�ر����" << endl;
	return 0;
}
int zht_SetTime(int hComm, int id, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nWeekDay)
{
	int ret = 0;
	int success = 0;  //0 ʧ��, 1��ʾ�ɹ�  
	if ((id > 10 || id < 0) || MC[id].ConnectFlag == 0 || MC[id].SN != hComm)
	{
		return -1;
	}
	MC[id].pkt.Reset();
	MC[id].pkt.functionID = 0x30;
	nYear += 2000;
	MC[id].pkt.data[0] = GetHex((nYear - nYear % 100) / 100);
	MC[id].pkt.data[1] = GetHex((int)((nYear) % 100)); //st.GetMonth()); 
	MC[id].pkt.data[2] = GetHex(nMonth);
	MC[id].pkt.data[3] = GetHex(nDay);
	MC[id].pkt.data[4] = GetHex(nHour);
	MC[id].pkt.data[5] = GetHex(nMinute);
	MC[id].pkt.data[6] = GetHex(nSecond);
	ret = MC[id].pkt.run(MC[id].udp);
	success = 0;
	log("��������ʱ��...");
	if (ret > 0)
	{
		bool bSame = true;
		for (int i = 0; i < 7; i++)
		{
			if (MC[id].pkt.data[i] != MC[id].pkt.recv[8 + i])
			{
				bSame = false;
				break;
			}
		}
		if (bSame)
		{
			log("1.6 ��������ʱ�� �ɹ�...");
			success = 1;
		}
		else
		{
			log("1.6 ��������ʱ�� ʧ��...");
			success = 0;
		}
	}
	if (success == 1)
		return 0;
	else
		return -1;
}
//int setRevIPandRevPort(int watchServerPort, WGPacketShort pkt, int id)
//{
//	int ret = 0;
//	int success = 0;  //0 ʧ��, 1��ʾ�ɹ�
//	string name = Dns.GetHostName();
//	string watchServerIP = "192.168.132.166";
//	IPAddress[] ipadrlist = Dns.GetHostAddresses(name);
//	foreach(IPAddress ipa in ipadrlist)
//	{
//		if (ipa.AddressFamily == AddressFamily.InterNetwork)
//			watchServerIP = ipa.ToString();//onsole.Writeline(ipa.ToString());
//	}
//	if (watchServerPort < 0 || watchServerPort > 65535)
//	{
//		return -1;
//	}
//	//1.18	���ý��շ�������IP�Ͷ˿� [���ܺ�: 0x90] **********************************************************************************
//	//(��������ÿ�������������, ֻҪ�����շ�������IP��Ϊ0.0.0.0 ������)
//	//���շ������Ķ˿�: 61005
//	//ÿ��5�뷢��һ��: 05
//	pkt.Reset();
//	pkt.functionID = 0x90;
//	string[] strIP = watchServerIP.Split('.');
//	if (strIP.Length == 4)
//	{
//		pkt.data[0] = byte.Parse(strIP[0]);
//		pkt.data[1] = byte.Parse(strIP[1]);
//		pkt.data[2] = byte.Parse(strIP[2]);
//		pkt.data[3] = byte.Parse(strIP[3]);
//	}
//	else
//	{
//		return -1;
//	}
//	MC[id].localIP = watchServerIP;
//	//���շ������Ķ˿�: 61005
//	pkt.data[4] = (byte)((watchServerPort & 0xff));
//	pkt.data[5] = (byte)((watchServerPort >> 8) & 0xff);
//
//	//ÿ��5�뷢��һ��: 05 (��ʱ�ϴ���Ϣ������Ϊ5�� [��������ʱÿ��5�뷢��һ��  ��ˢ��ʱ��������])
//	pkt.data[6] = 5;
//	ret = pkt.run();
//	success = 0;
//	log("���ý��շ�������IP�Ͷ˿�");
//	if (ret > 0)
//	{
//		if (pkt.recv[8] == 1)
//		{
//			log("1.18 ���ý��շ�������IP�Ͷ˿� 	 �ɹ�...");
//			success = 1;
//		}
//	}
//	else
//	{
//		return -1;
//	}
//
//
//	//1.19	��ȡ���շ�������IP�Ͷ˿� [���ܺ�: 0x92] **********************************************************************************
//	pkt.Reset();
//	pkt.functionID = 0x92;
//
//	ret = pkt.run();
//	success = 0;
//	log("��ȡ���շ�������IP�Ͷ˿�");
//	if (ret > 0)
//	{
//		if (byte.Parse(strIP[0]) == pkt.recv[8] && byte.Parse(strIP[1]) == pkt.recv[9] && byte.Parse(strIP[2]) == pkt.recv[10] && byte.Parse(strIP[3]) == pkt.recv[11])
//		{
//			if (pkt.recv[12] == (byte)((watchServerPort & 0xff)) && pkt.recv[13] == (byte)((watchServerPort >> 8) & 0xff))
//			{
//				log("1.19 ��ȡ���շ�������IP�Ͷ˿� 	 �ɹ�...");
//				success = 1;
//				return 0;
//			}
//		}
//	}
//	return -1;
//}
//ControllerIP �����õĿ�����IP��ַ
//controllerSN �����õĿ��������к�
//watchServerIP   Ҫ���õķ�����IP
//watchServerPort Ҫ���õĶ˿�
int setRevIpandRevPort(WGPacketShort pkt , int id)  //���շ��������� -- ����
{
	int ret = 0;
	int success = 0;  //0 ʧ��, 1��ʾ�ɹ�

	//1.18	���ý��շ�������IP�Ͷ˿� [���ܺ�: 0x90] **********************************************************************************
	//	���շ�������IP: 192.168.168.101  [��ǰ����IP]
	//(��������ÿ�������������, ֻҪ�����շ�������IP��Ϊ0.0.0.0 ������)
	//���շ������Ķ˿�: 61005
	//ÿ��5�뷢��һ��: 05
	pkt.Reset();
	pkt.functionID = 0x90;
	pkt.iDevSn = MC[id].SN;

	//������IP: 192.168.168.101
	//pkt.data[0] = 192; 
	//pkt.data[1] = 168; 
	//pkt.data[2] = 168; 
	//pkt.data[3] = 101; 
	int watchServerPort = MC[id].localPort;
	char *watchServerIP = MC[id].localIP;

	ACE_INET_Addr watchServer_addr(watchServerPort, watchServerIP); //�˿�  IP��ַ
	unsigned int iwatchServerIPInfo = watchServer_addr.get_ip_address();
	pkt.data[0] = (iwatchServerIPInfo >> 24) & 0xff;
	pkt.data[1] = (iwatchServerIPInfo >> 16) & 0xff;
	pkt.data[2] = (iwatchServerIPInfo >> 8) & 0xff;
	pkt.data[3] = iwatchServerIPInfo & 0xff;


	//���շ������Ķ˿�: 61005
	pkt.data[4] = (watchServerPort & 0xff);
	pkt.data[5] = (watchServerPort >> 8) & 0xff;
	cout << "�������˿���:" << watchServerPort << endl;
	unsigned char lPort = pkt.data[4];
	unsigned char hPort = pkt.data[5];
	//ÿ��5�뷢��һ��: 05 (��ʱ�ϴ���Ϣ������Ϊ5�� [��������ʱÿ��5�뷢��һ��  ��ˢ��ʱ��������])
	pkt.data[6] = 5;

	ret = pkt.run(MC[id].udp);
	success = 0;
	if (ret >0)
	{
		if (pkt.recv[8] == 1)
		{
			log("1.18 ���ý��շ�������IP�Ͷ˿� 	 �ɹ�...");
			success = 1;
		}
		else
		{
			return -1;
		}
	}
	//1.19	��ȡ���շ�������IP�Ͷ˿� [���ܺ�: 0x92] **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x92;
	pkt.iDevSn = MC[id].SN;

	ret = pkt.run(MC[id].udp);
	success = 0;
	log("1.19 ��ȡ���շ�������IP�Ͷ˿�");
	if (ret >0)
	{
		if (((iwatchServerIPInfo >> 24) & 0xff) == pkt.recv[8] && ((iwatchServerIPInfo >> 16) & 0xff) == pkt.recv[9] && ((iwatchServerIPInfo >> 8) & 0xff) == pkt.recv[10] && ((iwatchServerIPInfo) & 0xff) == pkt.recv[11])
		{
			if (pkt.recv[12] == lPort && pkt.recv[13] == hPort)
			{
				log("1.19 ��ȡ���շ�������IP�Ͷ˿� 	 �ɹ�...");
				success = 1;
				return 0;
			}
		}
		log("1.19 ��ȡ���շ�������IP�Ͷ˿� 	 �ɹ�...");
		success = 1;
	}
	return -1;
}
void toJson(char *root, char *key, char *value, int flag)
{
	char json[100];
	char *begin = "[{";
	char *temp = "}]";
	char* temp_1 = "\"";
	char* temp_2 = ":";
	char* temp_3 = ",";
	if (strlen(root) == 0)
	{
		cout << "root" << root << endl;
		strcat(root, begin);
	}
	//���һ��dict
	memset(json,'\0',100);
	strcat(json, temp_1);  //"
	strcat(json, key);  //key
	strcat(json, temp_1);  //"
	strcat(json, temp_2);  //:
	strcat(json, temp_1);  //"
	strcat(json, value);  //value
	strcat(json, temp_1);  //"
	if (flag == 0)
	{
		strcat(json, temp_3);  //,
	}
	else
	{
		strcat(json, temp);
	}
	strcat(root, json);  //"
}
int zht_WatchingServerRuning(char *watchServerIP, int watchServerPort,int id)
{
	//ע�����ǽ Ҫ����˶˿ڵ����а��������
	ACE_INET_Addr server_addr(static_cast<u_short> (watchServerPort), watchServerIP);
	ACE_SOCK_Dgram_Bcast udp(server_addr);
	MC[id].setSerUdp(udp);
	unsigned char buff[WGPacketShort::WGPacketSize];
	size_t buflen = sizeof(buff);
	ssize_t recv_cnt;
	log("������շ��������״̬....");
	unsigned int recordIndex = 0;
	char root[200] = { 0 };
	memset(root, '\0', 200);
	char temp[10];
	int index = 0;
	while (true)
	{
		ACE_INET_Addr any_addr;
		recv_cnt = udp.recv(buff, buflen, any_addr);
		if (recv_cnt > 0)
		{
			if (recv_cnt == WGPacketShort::WGPacketSize)
			{
				if (buff[1] == 0x20) //
				{
					unsigned int sn;
					unsigned int recordIndexGet;
					memcpy(&sn, &(buff[4]), 4);
					printf("���յ����Կ�����SN = %d �����ݰ�..\r\n", sn);

					/*memcpy(&recordIndexGet, &(buff[8]), 4);
					if (recordIndex < recordIndexGet)
					{
						recordIndex = recordIndexGet;
						displayRecordInformation(buff);
					}
					*/
					//���յ����ݣ�����IDֵ���з���
					memset(root, '\0', 200);
					index ++;
					if(index == 10)
					{
						zht_ClosePort(MC[id].SN, 1);
					}
					//TIME
					char controllerTime[] = "2000.01.01 00:00:00"; //��������ǰʱ��
					sprintf(controllerTime, "20%02X.%02X.%02X %02X:%02X:%02X",
						buff[51], buff[52], buff[53], buff[37], buff[38], buff[39]);
					toJson(root, "Time", controllerTime,0);
					//ID
					memset(temp, '\0', 10);
					_itoa(id, temp, 10);
					//sprintf(temp, "%d", id);
					toJson(root, "nID", temp, 0);
					
					//14	�ź�(1,2,3,4)	1	
					int recordDoorNO = buff[14];
					memset(temp, '\0', 10);
					_itoa(recordDoorNO, temp, 10);
					//sprintf(temp, "%d", id);
					toJson(root, "ChannelID", temp, 0);

					//16-19	����(������ˢ����¼ʱ)
					//����(�������ͼ�¼)	4	
					int recordCardNO = 0;
					memcpy(&recordCardNO, &(buff[16]), 4);
					memset(temp, '\0', 10);
					_itoa(recordCardNO, temp, 10);					
					toJson(root, "UID", temp, 0);


					//15	����/����(1��ʾ����, 2��ʾ����)	1	0x01
					int recordInOrOut = buff[15];
					memset(temp, '\0', 10);					
					if (recordInOrOut == 1)
					{//����
						memcpy(temp, "IN", 2);
					}
					else
					{
						memcpy(temp, "OUT", 3);
					}
					toJson(root, "direction", temp, 0);

					//13	��Ч��(0 ��ʾ��ͨ��, 1��ʾͨ��)	1	
					int recordValid = buff[13];
					memset(temp, '\0', 10);
					if (recordValid == 1)
					{//ͨ��
						toJson(root, "Count", "2", 1); //����
					}
					else
					{//δͨ��
						toJson(root, "Count", "1", 1); //����
					}
					//json��װ���
					MC[id].fbackcall(id, 1, root);
				}
			}
		}
	}
	udp.close();
	return 0;
}

int zht_SetCallbackAddr(int hComm, int id, FUNPTR_CALLBACK callback , char *localIP)
{
	int ret = 0;
	int success = 0;  //0 ʧ��, 1��ʾ�ɹ�  
	if ((id > 10 || id < 0) || MC[id].ConnectFlag == 0 || MC[id].SN != hComm)
	{
		return -1;
	}
	memcpy(MC[id].localIP, localIP,strlen(localIP));
	MC[id].fbackcall = callback;

	if (setRevIpandRevPort(MC[id].pkt, id) == -1)
	{
		return -1;
	}
	zht_WatchingServerRuning(MC[id].localIP, MC[id].localPort, id);
	return 0;
}
int *arrayIndex;
void getIDCardIndex(char *data)
{
	int i = 0;
	int j = 0;		
	arrayIndex[i++] = -1;
	while(j < strlen(data))
	{
		if (data[j] == ';')
		{			
			arrayIndex[i++] = j;
			log("whitet list index = ", j);
		}
		j++;
	}	
	//arrayIndex[i] = 0;
}
int getIDforCard(char *data, int index)
{
	char temp[12] = {0};
	int dataIndex = arrayIndex[index];
	int len = 10;
	if(arrayIndex[index+1] == 0)
		return 0;
		//len = arrayIndex[index + 1] - arrayIndex[index] - 1;	
	//memset(temp,'\0',10);
	strncat(temp, &data[dataIndex+1], len);
	log("whitet num = ", atoi(temp));
	return atoi(temp);
}
int zht_AddtoWhitelist(int hComm, int id, char *cardid)
{
	int ret = 0;
	int success = 0;  //0 ʧ��, 1��ʾ�ɹ�  
	if ((id > 10 || id < 0) || MC[id].ConnectFlag == 0 || MC[id].SN != hComm || strlen(cardid) < 10)
	{
		return -1;
	}
	WGPacketShort pkt = MC[id].pkt;

	int cardNOOfPrivilege = 0;
	int index = 0;
	arrayIndex = new int[int(strlen(cardid) / 11) + 1 ];	
	memset(arrayIndex, 0, sizeof(int)*(strlen(cardid) / 11) + 1 );
	getIDCardIndex(cardid);

	while(1)
	{
		cardNOOfPrivilege = getIDforCard(cardid, index++);
		log("1.11 Ȩ����ӻ��޸� = ", cardNOOfPrivilege);
		if (cardNOOfPrivilege == 0)
			break;
		success = 0;
		pkt.Reset();
		pkt.functionID = 0x50;
		pkt.iDevSn = hComm;
		//0D D7 37 00 Ҫ��ӻ��޸ĵ�Ȩ���еĿ��� = 0x0037D70D = 3659533 (ʮ����)
		//int cardNOOfPrivilege = 0x0037D70D;
		memcpy(&(pkt.data[0]), &cardNOOfPrivilege, 4);
		//20 10 01 01 ��ʼ����:  2010��01��01��   (�������2001��)
		pkt.data[4] = 0x20;
		pkt.data[5] = 0x10;
		pkt.data[6] = 0x01;
		pkt.data[7] = 0x01;
		//20 29 12 31 ��ֹ����:  2029��12��31��
		pkt.data[8] = 0x20;
		pkt.data[9] = 0x29;
		pkt.data[10] = 0x12;
		pkt.data[11] = 0x31;
		//01 ����ͨ�� һ���� [�Ե���, ˫��, ���ſ�������Ч] 
		pkt.data[12] = 0x01;
		//01 ����ͨ�� ������ [��˫��, ���ſ�������Ч]
		pkt.data[13] = 0x01;  //�����ֹ2����, ��ֻҪ��Ϊ 0x00
							  //01 ����ͨ�� ������ [�����ſ�������Ч]
		pkt.data[14] = 0x01;
		//01 ����ͨ�� �ĺ��� [�����ſ�������Ч]
		pkt.data[15] = 0x01;

		ret = pkt.run(MC[id].udp);
		success = 0;
		log("1.11 Ȩ����ӻ��޸�");
		if (ret >0)
		{
			if (pkt.recv[8] == 1)
			{
				//��ʱ ˢ����Ϊ= 0x0037D70D = 3659533 (ʮ����)�Ŀ�, 1���ż̵�������.
				log("1.11 Ȩ����ӻ��޸�	 �ɹ�...");
				success = 1;
			}
		}
		if(success == 0)
		{
			break;
		}
	}
	delete[] arrayIndex;
	if(success == 1)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}


void back(int nID, int eventType, void* param)
{
	if (eventType == 1)
	{
		log("jinlaile zheshi eventtype == 1");
	}
	log((char *)param);
}
void createBackcall()
{
	char localip[32] = "192.168.121.201";
	zht_SetCallbackAddr(MC[1].SN, 1, back, localip);
}
	//������δ������������  �� ����IP�Ĺ���  (ֱ����IP���ù��������)
	//�������в���˵��
	//������SN  = 422101164
	//������IP  = 192.168.168.123
	//����  IP  = 192.168.168.101
	//������Ϊ���շ�������IP (������IP 192.168.168.101), ���շ������˿� (61005)

int ACE_TMAIN (int, ACE_TCHAR *[]) //�������
{
	int ret =0;
	char ip[32] = "192.168.121.150";
	
	////log("�����������IP:");
	//cout << ("�����������IP:  ");
	//cin >> ip;
	int port = 60000;
	////log("�����������IP:");
	//cout << ("�����������port:  ");
	//cin >> port;
	int SN = zht_InitPort(1, 61000, port, ip);	
	HANDLE h; //�߳̾��
    h = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)createBackcall,NULL,1,0); //�������߳�
    ResumeThread(h);  //�������߳�

    if (ret != 0)
    {
        cout << "pthread_create error: error_code=" << ret << endl;
    }
	getchar();
	zht_SetTime(SN, 1, 12, 11, 23, 12, 58, 32, 1);
	zht_AddtoWhitelist(SN, 1, (char *)"0001254545;9632145690;9632145190;9232145690;0632145690;1632145690;");
	//zht_SetCallbackAddr(SN, 1, back, localip);
	////log("�����������IP:");
	//cout << ("�����������IP:  ");
	//cin >> localip;
	getchar();
	zht_ClosePort(SN, 1);
	//int sn =0;
	//cout<<("�����������SN(9λ��):  ");
	//cin>>sn;;
	//
	//char ip[32];
	////log("�����������IP:");
	//cout<< ("�����������IP:  ");
	//cin >>ip;
	//	
	//ret = testBasicFunction(ip,sn); //�������ܲ���
	//if (ret !=1)
	//{
	//  log("�������ܲ���ʧ��, �� X �س����˳�...");
	//  char stop;
	//  cin >>stop;
	//  return 0;
 //   }
	//
	////���շ���������
	//char watchServerIP[32]; // = "192.168.168.101";
	//cout<< ("��������շ�������IP:  ");
	//cin >>watchServerIP;
 //   int  watchServerPort = 61005;
	////cout<<("��������շ������˿�:  ");
	////cin>>watchServerPort;;

	//ret = testWatchingServer(ip,sn,watchServerIP, watchServerPort); //���շ���������

	//ret = WatchingServerRuning(watchServerIP, watchServerPort); //����������....
	//
	log("���Խ���, ���س����˳�...");
	getchar();
	return 1;
}
/*
int testBasicFunction(char *ControllerIP, unsigned int controllerSN)  //�������ܲ���
{
	int ret =0;
	int success =0;  //0 ʧ��, 1��ʾ�ɹ�

	ACE_INET_Addr controller_addr (9630, ControllerIP); //�˿�  IP��ַ
	ACE_SOCK_CODgram udp;
	if (0 != udp.open (controller_addr))
	{
		//��������ЧIP
		log("��������ЧIP...");
		return -1;
	}

	//�����̱��� pkt
	WGPacketShort pkt;  


	//1.4	��ѯ������״̬[���ܺ�: 0x20](ʵʱ�����) **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x20;
	pkt.iDevSn = controllerSN; 
	ret = pkt.run(udp);

	success =0;
	if (ret == 1)
	{
		//��ȡ��Ϣ�ɹ�...
		success =1;
		log("1.4 ��ѯ������״̬ �ɹ�...");

		//	  	���һ����¼����Ϣ		
		displayRecordInformation(pkt.recv);


		//	������Ϣ		
		int doorStatus[4];
		//28	1�����Ŵ�(0��ʾ����, 1��ʾ��)	1	0x00
		doorStatus[1-1] = pkt.recv[28];
		//29	2�����Ŵ�(0��ʾ����, 1��ʾ��)	1	0x00
		doorStatus[2-1] = pkt.recv[29];
		//30	3�����Ŵ�(0��ʾ����, 1��ʾ��)	1	0x00
		doorStatus[3-1] = pkt.recv[30];
		//31	4�����Ŵ�(0��ʾ����, 1��ʾ��)	1	0x00
		doorStatus[4-1] = pkt.recv[31];

		int pbStatus[4];
		//32	1���Ű�ť(0��ʾ�ɿ�, 1��ʾ����)	1	0x00
		pbStatus[1-1] = pkt.recv[32];
		//33	2���Ű�ť(0��ʾ�ɿ�, 1��ʾ����)	1	0x00
		pbStatus[2-1] = pkt.recv[33];
		//34	3���Ű�ť(0��ʾ�ɿ�, 1��ʾ����)	1	0x00
		pbStatus[3-1] = pkt.recv[34];
		//35	4���Ű�ť(0��ʾ�ɿ�, 1��ʾ����)	1	0x00
		pbStatus[4-1] = pkt.recv[35];
		//36	���Ϻ�
		//����0 �޹���
		//������0, �й���(������ʱ��, �����������, ��Ҫ������ά��)	1	
		int errCode = pkt.recv[36];
		//37	��������ǰʱ��
		//ʱ	1	0x21
		//38	��	1	0x30
		//39	��	1	0x58

		//40-43	��ˮ��	4	
		long long  sequenceId=0;
		memcpy(&sequenceId, &(pkt.recv[40]),4);

		//48
		//������Ϣ1(����ʵ��ʹ���з���)
		//���̰�����Ϣ	1	
		//49	�̵���״̬	1	
		int relayStatus = pkt.recv[49];
		//50	�Ŵ�״̬��8-15bitλ[��/ǿ������]
		//Bit0  ǿ������
		//Bit1  ��		
		int otherInputStatus = pkt.recv[50];
		if ((otherInputStatus & 0x1) > 0)
		{
			//ǿ������
		}
		if ((otherInputStatus & 0x2) > 0)
		{
			//��
		}

		//51	V5.46�汾֧�� ��������ǰ��	1	0x13
		//52	V5.46�汾֧�� ��	1	0x06
		//53	V5.46�汾֧�� ��	1	0x22

		char controllerTime[]="2000-01-01 00:00:00"; //��������ǰʱ��
		sprintf(controllerTime,"20%02X-%02X-%02X %02X:%02X:%02X", 
			pkt.recv[51],pkt.recv[52],pkt.recv[53],pkt.recv[37],pkt.recv[38],pkt.recv[39]);
	}
	else
	{
		log("1.4 ��ѯ������״̬ ʧ��?????...");
		return -1;
	}

	//1.5	��ȡ����ʱ��(���ܺ�: 0x32) **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x32;
	pkt.iDevSn = controllerSN; 
	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		char controllerTime[]="2000-01-01 00:00:00"; //��������ǰʱ��
		sprintf(controllerTime,"%02X%02X-%02X-%02X %02X:%02X:%02X", 
			pkt.recv[8],pkt.recv[9],pkt.recv[10],pkt.recv[11],pkt.recv[12],pkt.recv[13],pkt.recv[14]);

		log("1.5 ��ȡ����ʱ�� �ɹ�...");
		//log(controllerTime);
		success =1;
	}

	//1.6	��������ʱ��[���ܺ�: 0x30] **********************************************************************************
	//�����Ե�ǰʱ��У׼������.....
	pkt.Reset();
	pkt.functionID = 0x30;
	pkt.iDevSn = controllerSN; 

	time_t nowtime;     
	struct tm* ptm;     
	time(&nowtime);     
	ptm = localtime(&nowtime);     
	pkt.data[0] =GetHex((int)(( ptm->tm_year + 1900-( ptm->tm_year + 1900)%100)/100)); 
	pkt.data[1] =GetHex((int)(( ptm->tm_year + 1900)%100)); //st.GetMonth()); 
	pkt.data[2] =GetHex(ptm->tm_mon + 1); 
	pkt.data[3] =GetHex( ptm->tm_mday); 
	pkt.data[4] =GetHex(ptm->tm_hour); 
	pkt.data[5] =GetHex(ptm->tm_min); 
	pkt.data[6] = GetHex(ptm->tm_sec); 
	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		if(memcmp(&(pkt.data[0]), &(pkt.recv[8]),7)==0)
		{
			log("1.6 ��������ʱ�� �ɹ�...");
			success =1;
		}	
	}

	//1.7	��ȡָ�������ŵļ�¼[���ܺ�: 0xB0] **********************************************************************************
	//(ȡ������ 0x00000001�ļ�¼)
	int  recordIndexToGet =0;
	pkt.Reset();
	pkt.functionID = 0xB0;
	pkt.iDevSn = controllerSN; 

	//	(����
	//���=0, ��ȡ������һ����¼��Ϣ
	//���=0xffffffff��ȡ�����һ����¼����Ϣ)
	//��¼�����������������˳�������, ���ɴ�0xffffff = 16,777,215 (����1ǧ��) . ���ڴ洢�ռ�����, ��������ֻ�ᱣ�������20�����¼. �������ų���20���, �ɵ�������λ�ļ�¼�ͻᱻ����, ������ʱ��ѯ��Щ�����ŵļ�¼, ���صļ�¼���ͽ���0xff, ��ʾ��������.
	recordIndexToGet =1;
	memcpy(&(pkt.data[0]), &recordIndexToGet, 4);

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		log("1.7 ��ȡ����Ϊ1�ż�¼����Ϣ	 �ɹ�...");
		//	  	����Ϊ1�ż�¼����Ϣ		
		displayRecordInformation(pkt.recv);

		success =1;
	}

	//. �������� (ȡ�����һ����¼ ͨ�������� 0x00000000) [��ָ���ʺ��� ˢ����¼����20��ʱ������ʹ��]
	pkt.Reset();
	pkt.functionID = 0xB0;
	pkt.iDevSn = controllerSN; 
	recordIndexToGet =0;
	memcpy(&(pkt.data[0]), &recordIndexToGet, 4);
	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		log("1.7 ��ȡ����һ����¼����Ϣ	 �ɹ�...");
		//	  	����һ����¼����Ϣ		
		displayRecordInformation(pkt.recv);
        success =1;
	}

	//�������� (ȡ���µ�һ����¼ ͨ������ 0xffffffff)
	pkt.Reset();
	pkt.functionID = 0xB0;
	pkt.iDevSn = controllerSN; 
	recordIndexToGet =0xffffffff;
	memcpy(&(pkt.data[0]), &recordIndexToGet, 4);
	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		log("1.7 ��ȡ���¼�¼����Ϣ	 �ɹ�...");
		//	  	���¼�¼����Ϣ		
				displayRecordInformation(pkt.recv);
		success =1;
	}

	int recordIndexGotToRead;

	////1.8	�����Ѷ�ȡ���ļ�¼������[���ܺ�: 0xB2] **********************************************************************************
	//pkt.Reset();
	//pkt.functionID = 0xB2;
	//pkt.iDevSn = controllerSN; 
	//// (��Ϊ�Ѷ�ȡ���ļ�¼������Ϊ5)
	//int recordIndexGot =0x5;
	//memcpy(&(pkt.data[0]), &recordIndexGot, 4);

	////12	��ʶ(��ֹ������)	1	0x55 [�̶�]
	//memcpy(&(pkt.data[4]), &(WGPacketShort::SpecialFlag), 4);

	//ret = pkt.run(udp);
	//success =0;
	//if (ret >0)
	//{
	//	if (pkt.recv[8] == 1)
	//	{
	//		log("1.8 �����Ѷ�ȡ���ļ�¼������	 �ɹ�...");
	//		success =1;
	//	}
	//}

	////1.9	��ȡ�Ѷ�ȡ���ļ�¼������[���ܺ�: 0xB4] **********************************************************************************
	//pkt.Reset();
	//pkt.functionID = 0xB4;
	//pkt.iDevSn = controllerSN; 
	// recordIndexGotToRead =0x0;
	//ret = pkt.run(udp);
	//success =0;
	//if (ret >0)
	//{
	//	memcpy(&(recordIndexGotToRead), &(pkt.recv[8]),4);
	//	log("1.9 ��ȡ�Ѷ�ȡ���ļ�¼������	 �ɹ�...");
	//	success =1;
	//}

	//1.9	��ȡ��¼����
	//1. ͨ�� 0xB4ָ�� ��ȡ�Ѷ�ȡ���ļ�¼������ recordIndex
	//2. ͨ�� 0xB0ָ�� ��ȡָ�������ŵļ�¼  ��recordIndex + 1��ʼ��ȡ��¼�� ֱ����¼Ϊ��Ϊֹ
	//3. ͨ�� 0xB2ָ�� �����Ѷ�ȡ���ļ�¼������  ���õ�ֵΪ����ȡ����ˢ����¼������
	//���������������裬 ������ȡ��¼�Ĳ������
    log("1.9 ��ȡ��¼����	 ��ʼ...");
	pkt.Reset();
	pkt.functionID = 0xB4;
	pkt.iDevSn = controllerSN; 
	int recordIndexGotToRead2 =0x0;
	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		memcpy(&(recordIndexGotToRead), &(pkt.recv[8]),4);
		pkt.Reset();
		pkt.functionID = 0xB0;
		pkt.iDevSn = controllerSN; 
		int recordIndexToGetStart = recordIndexGotToRead + 1;
		int recordIndexValidGet = 0;
		int cnt=0;
		do
		{
			memcpy(&(pkt.data[0]), &recordIndexToGetStart, 4);
			ret = pkt.run(udp);
			success =0;
			if (ret >0)
			{
				success =1;

				//12	��¼����
				//0=�޼�¼
				//1=ˢ����¼
				//2=�Ŵ�,��ť, �豸����, Զ�̿��ż�¼
				//3=������¼	1	
				//0xFF=��ʾָ������λ�ļ�¼�ѱ����ǵ���.  ��ʹ������0, ȡ������һ����¼������ֵ
				int recordType = pkt.recv[12];
				if (recordType == 0)
				{
					break; //û�и����¼
				}
				if (recordType == 0xff)
				{
					success = 0;  //����������Ч  ������������ֵ
					//ȡ����һ����¼������λ
                    pkt.iDevSn = controllerSN; 
					recordIndexToGet =0;
					memcpy(&(pkt.data[0]), &recordIndexToGet, 4);
					ret = pkt.run(udp);
					success =0;
					if (ret >0)
                    {
                        log("1.7 ��ȡ����һ����¼����Ϣ	 �ɹ�...");
						int recordIndex =0;
		                memcpy(&recordIndex, &(pkt.recv[8]),4);
                        recordIndexGotToRead = recordIndex;
                        recordIndexToGetStart = recordIndexGotToRead;
                        continue;
                    }
                    success = 0;  
					break; 
				}

				recordIndexValidGet = recordIndexToGetStart;
                //.......���յ��ļ�¼���洢����
			    displayRecordInformation(pkt.recv);
				//*****
				//###############
			}
			else
			{
				//��ȡʧ��
				break;
			}
			recordIndexToGetStart++;
		}while(cnt++ < 200000);
		if (success >0)
		{
			//ͨ�� 0xB2ָ�� �����Ѷ�ȡ���ļ�¼������  ���õ�ֵΪ����ȡ����ˢ����¼������
			pkt.Reset();
			pkt.functionID = 0xB2;
			pkt.iDevSn = controllerSN; 
			memcpy(&(pkt.data[0]), &recordIndexValidGet, 4);

			//12	��ʶ(��ֹ������)	1	0x55 [�̶�]
	        memcpy(&(pkt.data[4]), &(WGPacketShort::SpecialFlag), 4);

			ret = pkt.run(udp);
			success =0;
			if (ret >0)
			{
				if (pkt.recv[8] == 1)
				{
					//��ȫ��ȡ�ɹ�....
					log("1.9 ��ȫ��ȡ�ɹ�	 �ɹ�...");
					success =1;
				}
			}

		}
	}

	//1.10	Զ�̿���[���ܺ�: 0x40] **********************************************************************************
	int doorNO =1;
	pkt.Reset();
	pkt.functionID = 0x40;
	pkt.iDevSn = controllerSN; 
	pkt.data[0] = (doorNO & 0xff); //2013-11-03 20:56:33
	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		if (pkt.recv[8] == 1)
		{
			//��Ч����.....
			log("1.10 Զ�̿���	 �ɹ�...");
			success =1;
		}
	}

	//1.11	Ȩ����ӻ��޸�[���ܺ�: 0x50] **********************************************************************************
	//���ӿ���0D D7 37 00, ͨ����ǰ��������������
	pkt.Reset();
	pkt.functionID = 0x50;
	pkt.iDevSn = controllerSN; 
	//0D D7 37 00 Ҫ��ӻ��޸ĵ�Ȩ���еĿ��� = 0x0037D70D = 3659533 (ʮ����)
	long long cardNOOfPrivilege =0x0037D70D;
	memcpy(&(pkt.data[0]), &cardNOOfPrivilege, 4);
	//20 10 01 01 ��ʼ����:  2010��01��01��   (�������2001��)
	pkt.data[4] = 0x20;
	pkt.data[5] = 0x10;
	pkt.data[6] = 0x01;
	pkt.data[7] = 0x01;
	//20 29 12 31 ��ֹ����:  2029��12��31��
	pkt.data[8] = 0x20;
	pkt.data[9] = 0x29;
	pkt.data[10] = 0x12;
	pkt.data[11] = 0x31;
	//01 ����ͨ�� һ���� [�Ե���, ˫��, ���ſ�������Ч] 
	pkt.data[12] = 0x01;
	//01 ����ͨ�� ������ [��˫��, ���ſ�������Ч]
	pkt.data[13] = 0x01;  //�����ֹ2����, ��ֻҪ��Ϊ 0x00
	//01 ����ͨ�� ������ [�����ſ�������Ч]
	pkt.data[14] = 0x01;
	//01 ����ͨ�� �ĺ��� [�����ſ�������Ч]
	pkt.data[15] = 0x01;

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		if (pkt.recv[8] == 1)
		{
			//��ʱ ˢ����Ϊ= 0x0037D70D = 3659533 (ʮ����)�Ŀ�, 1���ż̵�������.
			log("1.11 Ȩ����ӻ��޸�	 �ɹ�...");
			success =1;
		}
	}

	//1.12	Ȩ��ɾ��(����ɾ��)[���ܺ�: 0x52] **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x52;
	pkt.iDevSn = controllerSN; 
	//Ҫɾ����Ȩ�޿���0D D7 37 00  = 0x0037D70D = 3659533 (ʮ����)
	long long cardNOOfPrivilegeToDelete =0x0037D70D;
	memcpy(&(pkt.data[0]), &cardNOOfPrivilegeToDelete, 4);

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		if (pkt.recv[8] == 1)
		{
			//��ʱ ˢ����Ϊ= 0x0037D70D = 3659533 (ʮ����)�Ŀ�, 1���ż̵������ᶯ��.
			log("1.12 Ȩ��ɾ��(����ɾ��)	 �ɹ�...");
			success =1;
		}
	}

	//1.13	Ȩ�����(ȫ�����)[���ܺ�: 0x54] **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x54;
	pkt.iDevSn = controllerSN; 
	memcpy(&(pkt.data[0]), &(WGPacketShort::SpecialFlag), 4);

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		if (pkt.recv[8] == 1)
		{
			//��ʱ��ճɹ�
			log("1.13 Ȩ�����(ȫ�����)	 �ɹ�...");
			success =1;
		}
	}

	//1.14	Ȩ��������ȡ[���ܺ�: 0x58] **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x58;
	pkt.iDevSn = controllerSN; 
	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		int privilegeCount =0;
		memcpy(&privilegeCount, &(pkt.recv[8]),4);
		log("1.14 Ȩ��������ȡ	 �ɹ�...");

		success =1;
	}


	//�ٴ����Ϊ��ѯ���� 1.11	Ȩ����ӻ��޸�[���ܺ�: 0x50] **********************************************************************************
	//���ӿ���0D D7 37 00, ͨ����ǰ��������������
	pkt.Reset();
	pkt.functionID = 0x50;
	pkt.iDevSn = controllerSN; 
	//0D D7 37 00 Ҫ��ӻ��޸ĵ�Ȩ���еĿ��� = 0x0037D70D = 3659533 (ʮ����)
	//long long 
		cardNOOfPrivilege =0x0037D70D;
	memcpy(&(pkt.data[0]), &cardNOOfPrivilege, 4);
	//20 10 01 01 ��ʼ����:  2010��01��01��   (�������2001��)
	pkt.data[4] = 0x20;
	pkt.data[5] = 0x10;
	pkt.data[6] = 0x01;
	pkt.data[7] = 0x01;
	//20 29 12 31 ��ֹ����:  2029��12��31��
	pkt.data[8] = 0x20;
	pkt.data[9] = 0x29;
	pkt.data[10] = 0x12;
	pkt.data[11] = 0x31;
	//01 ����ͨ�� һ���� [�Ե���, ˫��, ���ſ�������Ч] 
	pkt.data[12] = 0x01;
	//01 ����ͨ�� ������ [��˫��, ���ſ�������Ч]
	pkt.data[13] = 0x01;  //�����ֹ2����, ��ֻҪ��Ϊ 0x00
	//01 ����ͨ�� ������ [�����ſ�������Ч]
	pkt.data[14] = 0x01;
	//01 ����ͨ�� �ĺ��� [�����ſ�������Ч]
	pkt.data[15] = 0x01;

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		if (pkt.recv[8] == 1)
		{
			//��ʱ ˢ����Ϊ= 0x0037D70D = 3659533 (ʮ����)�Ŀ�, 1���ż̵�������.
			log("1.11 Ȩ����ӻ��޸�	 �ɹ�...");
			success =1;
		}
	}

	//1.15	Ȩ�޲�ѯ[���ܺ�: 0x5A] **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x5A;
	pkt.iDevSn = controllerSN; 
	// (�鿨��Ϊ 0D D7 37 00��Ȩ��)
	long long cardNOOfPrivilegeToQuery =0x0037D70D;
	memcpy(&(pkt.data[0]), &cardNOOfPrivilegeToQuery, 4);

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{

		long long cardNOOfPrivilegeToGet=0;
		memcpy(&cardNOOfPrivilegeToGet, &(pkt.recv[8]),4);
		if (cardNOOfPrivilegeToGet == 0)
		{
			//û��Ȩ��ʱ: (���Ų���Ϊ0)
			log ("1.15      û��Ȩ����Ϣ: (���Ų���Ϊ0)");
		}
		else
		{
			//����Ȩ����Ϣ...
			log ("1.15     ��Ȩ����Ϣ...");
		}
		log("1.15 Ȩ�޲�ѯ	 �ɹ�...");
		success =1;
	}

	//1.16  ��ȡָ�������ŵ�Ȩ��[���ܺ�: 0x5C] **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x5C;
	pkt.iDevSn = controllerSN; 
	long long QueryIndex = 1; //������(��1��ʼ);
	memcpy(&(pkt.data[0]), &QueryIndex, 4);

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{

		long long cardNOOfPrivilegeToGet=0;
		memcpy(&cardNOOfPrivilegeToGet, &(pkt.recv[8]),4);
		if (4294967295 == cardNOOfPrivilegeToGet) //FFFFFFFF��Ӧ��4294967295
		{
			log ("1.16      û��Ȩ����Ϣ: (Ȩ����ɾ��)");
		}
		else if (cardNOOfPrivilegeToGet == 0)
		{
			//û��Ȩ��ʱ: (���Ų���Ϊ0)
			log ("1.16       û��Ȩ����Ϣ: (���Ų���Ϊ0)--��������֮��û��Ȩ����");
		}
		else
		{
			//����Ȩ����Ϣ...
			 log ("1.16      ��Ȩ����Ϣ...");
		}
		log("1.16 ��ȡָ�������ŵ�Ȩ��	 �ɹ�...");
		success =1;
	}


	//1.17	�����ſ��Ʋ���(����/��ʱ) [���ܺ�: 0x80] **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x80;
	pkt.iDevSn = controllerSN; 
	//(����2���� ����  ������ʱ 3��)
	pkt.data[0] = 0x02; //2����
	pkt.data[1] = 0x03; //����
	pkt.data[2] = 0x03; //������ʱ

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
//2013-11-06 15:33:16		if (memcmp(&(pkt.data[0]), &(pkt.recv[8]),4) == 0)
		if (memcmp(&(pkt.data[0]), &(pkt.recv[8]),3) == 0) //2013-11-06 15:33:23  �Ƚ�����
		{
			//�ɹ�ʱ, ����ֵ������һ��
			log("1.17 �����ſ��Ʋ���	 �ɹ�...");
			success =1;			
		}
		else
		{
			//ʧ��
		}
	}

	//1.21	Ȩ�ް���С����˳�����[���ܺ�: 0x56] ������Ȩ������1000, ����8�� **********************************************************************************
        //�˹���ʵ�� ��ȫ����ȫ��Ȩ��, �û��������֮ǰ��Ȩ��. ֻ�ǽ��ϴ���Ȩ��˳��ӵ�1�����ε����һ���ϴ����. �����;�жϵĻ�, ����ԭȨ��Ϊ��
        //����Ȩ�������³���50��, ����ʹ�ô�ָ��

        log("1.21	Ȩ�ް���С����˳�����[���ܺ�: 0x56]	��ʼ...");
        log("       1����Ȩ��...");

        //��10000������Ϊ��, �˴��򻯵�����, ֱ������50001��ʼ��10000����. �û�������Ҫ��Ҫ�ϴ��Ŀ���������
        int cardCount = 10000;  //2015-06-09 20:20:20 ��������
        long cardArray[10000];
        for (int i = 0; i < cardCount; i++)
        {
            cardArray[i] = 50001+i;
        }

        for (int i = 0; i < cardCount; i++)
        {
            pkt.Reset();
            pkt.functionID = 0x56;
			pkt.iDevSn = controllerSN; 

            cardNOOfPrivilege = cardArray[i];
	    	memcpy(&(pkt.data[0]), &cardNOOfPrivilege, 4);
                
            //����������ʱ ͳһ, ��������ÿ�����Ĳ�ͬ�����޸�
            //20 10 01 01 ��ʼ����:  2010��01��01��   (�������2001��)
            pkt.data[4] = 0x20;
            pkt.data[5] = 0x10;
            pkt.data[6] = 0x01;
            pkt.data[7] = 0x01;
            //20 29 12 31 ��ֹ����:  2029��12��31��
            pkt.data[8] = 0x20;
            pkt.data[9] = 0x29;
            pkt.data[10] = 0x12;
            pkt.data[11] = 0x31;
            //01 ����ͨ�� һ���� [�Ե���, ˫��, ���ſ�������Ч] 
            pkt.data[12] = 0x01;
            //01 ����ͨ�� ������ [��˫��, ���ſ�������Ч]
            pkt.data[13] = 0x01;  //�����ֹ2����, ��ֻҪ��Ϊ 0x00
            //01 ����ͨ�� ������ [�����ſ�������Ч]
            pkt.data[14] = 0x01;
            //01 ����ͨ�� �ĺ��� [�����ſ�������Ч]
            pkt.data[15] = 0x01;

			memcpy(&(pkt.data[32-8]), &cardCount, 4);//�ܵ�Ȩ����
			int i2=i+1;
			memcpy(&(pkt.data[35-8]), &i2, 4);//��ǰȨ�޵�����λ(��1��ʼ)

            ret = pkt.run(udp);
            success = 0;
            if (ret > 0)
            {
                if (pkt.recv[8] == 1)
                {
                    success = 1;
                }
                if (pkt.recv[8] == 0xE1)
                {
                    log("1.21	Ȩ�ް���С����˳�����[���ܺ�: 0x56]	 =0xE1 ��ʾ����û�д�С��������...???");
                    success = 0;
                    break;
                }
            }
            else
            {
                break;
            }
        }
        if (success == 1)
        {
            log("1.21	Ȩ�ް���С����˳�����[���ܺ�: 0x56]	 �ɹ�...");
        }
        else
        {
            log("1.21	Ȩ�ް���С����˳�����[���ܺ�: 0x56]	 ʧ��...????");
        }
           



	//����ָ��  **********************************************************************************


	// **********************************************************************************

	//����  **********************************************************************************
	udp.close();
	return success;
}
*/
//ControllerIP �����õĿ�����IP��ַ
//controllerSN �����õĿ��������к�
//watchServerIP   Ҫ���õķ�����IP
//watchServerPort Ҫ���õĶ˿�
/*
int testWatchingServer(char *ControllerIP, unsigned int controllerSN, char *watchServerIP,int watchServerPort)  //���շ��������� -- ����
{
	int ret =0;
	int success =0;  //0 ʧ��, 1��ʾ�ɹ�

	ACE_INET_Addr controller_addr (9630, ControllerIP); //�˿�  IP��ַ
	ACE_SOCK_CODgram udp;
	if (0 != udp.open (controller_addr))
	{
		//��������ЧIP
		log("��������ЧIP...");
		return -1;
	}


	WGPacketShort pkt;
	//1.18	���ý��շ�������IP�Ͷ˿� [���ܺ�: 0x90] **********************************************************************************
	//	���շ�������IP: 192.168.168.101  [��ǰ����IP]
	//(��������ÿ�������������, ֻҪ�����շ�������IP��Ϊ0.0.0.0 ������)
	//���շ������Ķ˿�: 61005
	//ÿ��5�뷢��һ��: 05
	pkt.Reset();
	pkt.functionID = 0x90;
	pkt.iDevSn = controllerSN; 

	//������IP: 192.168.168.101
	//pkt.data[0] = 192; 
	//pkt.data[1] = 168; 
	//pkt.data[2] = 168; 
	//pkt.data[3] = 101; 
	ACE_INET_Addr watchServer_addr (watchServerPort, watchServerIP); //�˿�  IP��ַ
	unsigned int iwatchServerIPInfo = watchServer_addr.get_ip_address();
	pkt.data[0] = (iwatchServerIPInfo >> 24) & 0xff;   
	pkt.data[1] = (iwatchServerIPInfo >> 16) & 0xff;   
	pkt.data[2] = (iwatchServerIPInfo >> 8) & 0xff;  
	pkt.data[3] = iwatchServerIPInfo & 0xff; 


	//���շ������Ķ˿�: 61005
	pkt.data[4] =(watchServerPort & 0xff);
	pkt.data[5] =(watchServerPort >>8) & 0xff;

	//ÿ��5�뷢��һ��: 05 (��ʱ�ϴ���Ϣ������Ϊ5�� [��������ʱÿ��5�뷢��һ��  ��ˢ��ʱ��������])
	pkt.data[6] = 5;

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		if (pkt.recv[8] == 1)
		{
			log("1.18 ���ý��շ�������IP�Ͷ˿� 	 �ɹ�...");
			success =1;
		}
	}


	//1.19	��ȡ���շ�������IP�Ͷ˿� [���ܺ�: 0x92] **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x92;
	pkt.iDevSn = controllerSN; 

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		log("1.19 ��ȡ���շ�������IP�Ͷ˿� 	 �ɹ�...");
		success =1;
	}
	udp.close();
	return 1;
}

int WatchingServerRuning (char *watchServerIP,int watchServerPort)
{
	//ע�����ǽ Ҫ����˶˿ڵ����а��������
  ACE_INET_Addr server_addr (static_cast<u_short> (watchServerPort),watchServerIP); 
  ACE_SOCK_Dgram_Bcast udp (server_addr);
  unsigned char buff[WGPacketShort::WGPacketSize];
  size_t buflen = sizeof (buff);
  ssize_t recv_cnt;
  log("������շ��������״̬....");
  unsigned int recordIndex = 0;
  while(true)
  {
      ACE_INET_Addr any_addr;
	  recv_cnt = udp.recv(buff, buflen, any_addr); 
	  if (recv_cnt > 0)
	  {
		if (recv_cnt == WGPacketShort::WGPacketSize)
		{
			if (buff[1]== 0x20) //
			{
				unsigned int sn;
				unsigned int recordIndexGet;
				memcpy(&sn, &( buff[4]),4);
				printf("���յ����Կ�����SN = %d �����ݰ�..\r\n", sn);

				memcpy(&recordIndexGet, &(buff[8]),4);
				if (recordIndex < recordIndexGet)
				{
					recordIndex = recordIndexGet;
					displayRecordInformation(buff);				
				}
			}
		}
	  }
  }
  udp.close ();
  return 0;
}
*/



