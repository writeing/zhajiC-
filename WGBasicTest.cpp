/**
* $Id: WGBasicTest.cpp 2015-04-29 22:20:40 karl CSN 陈绍宁 $
*
* 门禁控制器 短报文协议 测试案例
* V1.1 版本  2013-11-05 15:02:02  
*            基本功能:  查询控制器状态 
*                       读取日期时间
*                       设置日期时间
*                       获取指定索引号的记录
*                       设置已读取过的记录索引号
*                       获取已读取过的记录索引号
*                       远程开门
*                       权限添加或修改
*                       权限删除(单个删除)
*                       权限清空(全部清掉)
*                       权限总数读取
*                       权限查询
*                       设置门控制参数(在线/延时)
*                       读取门控制参数(在线/延时)

*                       设置接收服务器的IP和端口
*                       读取接收服务器的IP和端口
*                       
*
*                       接收服务器的实现 (在61005端口接收数据) -- 此项功能 一定要注意防火墙设置 必须是允许接收数据的.
* V1.2 2013-11-07 12:40:49
*                 修改日期设置时的验证
*                 将数据包长度, Type, 控制器端口, 特殊标识固定化在WGPacketShort中
* V2.5 2015-04-29 20:41:30 采用 V6.56驱动版本 型号由0x19改为0x17
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
class WGPacketShort {				//短报文协议
public:
	const static unsigned int	 WGPacketSize = 64;			    //报文长度
	//2015-04-29 22:22:41 const static unsigned char	 Type = 0x19;					//类型
	const static unsigned char	 Type = 0x17;		//2015-04-29 22:22:50			//类型
	
	const static unsigned int    SpecialFlag =0x55AAAA55;       //特殊标识 防止误操作

	unsigned char	 functionID;		    //功能号
	unsigned int	 iDevSn;                //设备序列号 4字节
	unsigned char    data[56];              //56字节的数据 [含流水号]
	unsigned int     ControllerPort;        //控制器端口
	unsigned char    recv[WGPacketSize];    //接收到的数据
	WGPacketShort(void)
	{
		Reset();
	}
	void Reset()  //数据复位
	{
		memset(data,0,sizeof(data));
	}
	void toByte(char* buff, size_t buflen) //生成64字节指令包
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
	int run(ACE_SOCK_CODgram udp)  //通过指定的UDP发送指令 接收返回信息
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
					//流水号
					unsigned int  sequenceIdReceived=0;
					memcpy(&sequenceIdReceived, &(recv[40]),4);

					if ((recv[0]== Type) //类型一致
						&& (recv[1]== functionID) //功能号一致
						&& (sequenceIdReceived == currentSequenceId) )  //序列号对应
					{
						return 1;
					}
					else
					{
						errcnt++;
					}
				}
			}
		} while(tries-- >0); //重试三次

		return -1;
	}
	static unsigned int sequenceIdSent()// 最后发出流水号
	{
		return sequenceId; // 最后发出流水号
	}
private:
	static  unsigned int     sequenceId;     //序列号	
};
unsigned int WGPacketShort::sequenceId = 0;  //流水号值

unsigned char GetHex(int val) //获取Hex值, 主要用于日期时间格式
{
	return ((val % 10) + (((val -(val % 10)) / 10)%10) *16);
}
void log(char* info)  //日志信息
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
int testBasicFunction(char *ControllerIP, unsigned int controllerSN);  //基本功能测试
int testWatchingServer(char *ControllerIP, unsigned int controllerSN, char *watchServerIP,int watchServerPort);  //接收服务器测试
int WatchingServerRuning (char *watchServerIP,int watchServerPort);   //2013-11-05 13:08:39 进入服务器监控状态

#include<iostream>
using namespace std;
#include <windows.h>

//typedef void(*FUNPTR_CALLBACK)(int nID, int eventType, void* param);



char* RecordDetails[] =
        {
//记录原因 (类型中 SwipePass 表示通过; SwipeNOPass表示禁止通过; ValidEvent 有效事件(如按钮 门磁 超级密码开门); Warn 报警事件)
//代码  类型   英文描述  中文描述
"1","SwipePass","Swipe","刷卡开门",
"2","SwipePass","Swipe Close","刷卡关",
"3","SwipePass","Swipe Open","刷卡开",
"4","SwipePass","Swipe Limited Times","刷卡开门(带限次)",
"5","SwipeNOPass","Denied Access: PC Control","刷卡禁止通过: 电脑控制",
"6","SwipeNOPass","Denied Access: No PRIVILEGE","刷卡禁止通过: 没有权限",
"7","SwipeNOPass","Denied Access: Wrong PASSWORD","刷卡禁止通过: 密码不对",
"8","SwipeNOPass","Denied Access: AntiBack","刷卡禁止通过: 反潜回",
"9","SwipeNOPass","Denied Access: More Cards","刷卡禁止通过: 多卡",
"10","SwipeNOPass","Denied Access: First Card Open","刷卡禁止通过: 首卡",
"11","SwipeNOPass","Denied Access: Door Set NC","刷卡禁止通过: 门为常闭",
"12","SwipeNOPass","Denied Access: InterLock","刷卡禁止通过: 互锁",
"13","SwipeNOPass","Denied Access: Limited Times","刷卡禁止通过: 受刷卡次数限制",
"14","SwipeNOPass","Denied Access: Limited Person Indoor","刷卡禁止通过: 门内人数限制",
"15","SwipeNOPass","Denied Access: Invalid Timezone","刷卡禁止通过: 卡过期或不在有效时段",
"16","SwipeNOPass","Denied Access: In Order","刷卡禁止通过: 按顺序进出限制",
"17","SwipeNOPass","Denied Access: SWIPE GAP LIMIT","刷卡禁止通过: 刷卡间隔约束",
"18","SwipeNOPass","Denied Access","刷卡禁止通过: 原因不明",
"19","SwipeNOPass","Denied Access: Limited Times","刷卡禁止通过: 刷卡次数限制",
"20","ValidEvent","Push Button","按钮开门",
"21","ValidEvent","Push Button Open","按钮开",
"22","ValidEvent","Push Button Close","按钮关",
"23","ValidEvent","Door Open","门打开[门磁信号]",
"24","ValidEvent","Door Closed","门关闭[门磁信号]",
"25","ValidEvent","Super Password Open Door","超级密码开门",
"26","ValidEvent","Super Password Open","超级密码开",
"27","ValidEvent","Super Password Close","超级密码关",
"28","Warn","Controller Power On","控制器上电",
"29","Warn","Controller Reset","控制器复位",
"30","Warn","Push Button Invalid: Disable","按钮不开门: 按钮禁用",
"31","Warn","Push Button Invalid: Forced Lock","按钮不开门: 强制关门",
"32","Warn","Push Button Invalid: Not On Line","按钮不开门: 门不在线",
"33","Warn","Push Button Invalid: InterLock","按钮不开门: 互锁",
"34","Warn","Threat","胁迫报警",
"35","Warn","Threat Open","胁迫报警开",
"36","Warn","Threat Close","胁迫报警关",
"37","Warn","Open too long","门长时间未关报警[合法开门后]",
"38","Warn","Forced Open","强行闯入报警",
"39","Warn","Fire","火警",
"40","Warn","Forced Close","强制关门",
"41","Warn","Guard Against Theft","防盗报警",
"42","Warn","7*24Hour Zone","烟雾煤气温度报警",
"43","Warn","Emergency Call","紧急呼救报警",
"44","RemoteOpen","Remote Open Door","操作员远程开门",
"45","RemoteOpen","Remote Open Door By USB Reader","发卡器确定发出的远程开门"
        };
 char* getReasonDetailChinese(int Reason) //中文
        {
            if (Reason > 45)
            {
                return "";
            }
            if (Reason <= 0)
            {
                return "";
            }
            return RecordDetails[(Reason - 1) * 4 + 3]; //中文信息
        }

        char* getReasonDetailEnglish(int Reason) //英文描述
        {
            if (Reason > 45)
            {
                return "";
            }
            if (Reason <= 0)
            {
                return "";
            }
            return RecordDetails[(Reason - 1) * 4 + 2]; //英文信息
        }
        /// <summary>
        /// 显示记录信息
        /// </summary>
        /// <param name="recv"></param>
        void displayRecordInformation(unsigned char* recv)
        {
			//	  	最后一条记录的信息		
		//8-11	最后一条记录的索引号
		//(=0表示没有记录)	4	0x00000000
		int recordIndex =0;
		memcpy(&recordIndex, &(recv[8]),4);

		//12	记录类型
		//0=无记录
		//1=刷卡记录
		//2=门磁,按钮, 设备启动, 远程开门记录
		//3=报警记录	1	
		int recordType = recv[12];

		//13	有效性(0 表示不通过, 1表示通过)	1	
		int recordValid = recv[13];

		//14	门号(1,2,3,4)	1	
		int recordDoorNO = recv[14];

		//15	进门/出门(1表示进门, 2表示出门)	1	0x01
		int recordInOrOut = recv[15];

		//16-19	卡号(类型是刷卡记录时)
		//或编号(其他类型记录)	4	
		long long recordCardNO = 0;
		memcpy(&recordCardNO, &(recv[16]),4);

		//20-26	刷卡时间:
		//年月日时分秒 (采用BCD码)见设置时间部分的说明
		char recordTime[]="2000-01-01 00:00:00";
		sprintf(recordTime,"%02X%02X-%02X-%02X %02X:%02X:%02X", 
			recv[20],recv[21],recv[22],recv[23],recv[24],recv[25],recv[26]);

		//2012.12.11 10:49:59	7	
		//27	记录原因代码(可以查 “刷卡记录说明.xls”文件的ReasonNO)
		//处理复杂信息才用	1	
		int reason = recv[27];

              //0=无记录
            //1=刷卡记录
            //2=门磁,按钮, 设备启动, 远程开门记录
            //3=报警记录	1	
            //0xFF=表示指定索引位的记录已被覆盖掉了.  请使用索引0, 取回最早一条记录的索引值
            if (recordType == 0)
            {
				 printf("索引位=%u  无记录\r\n", recordIndex);
            }
            else if (recordType == 0xff)
            {
                log(" 指定索引位的记录已被覆盖掉了,请使用索引0, 取回最早一条记录的索引值");
            }
            else if (recordType == 1) //2015-06-10 08:49:31 显示记录类型为卡号的数据
            {
                //卡号
                printf("索引位=%u\r\n", recordIndex);
                printf("  卡号 = %u\r\n", recordCardNO);
                printf("  门号 = %u\r\n", recordDoorNO);
                printf("  进出 = %s\r\n", recordInOrOut == 1 ? "进门" : "出门");
                printf("  有效 = %s\r\n", recordValid == 1 ? "通过" : "禁止");
                printf("  时间 = %s\r\n", recordTime);
                printf("  描述 = %s\r\n", getReasonDetailChinese(reason));
            }
            else if (recordType == 2)
            {
                //其他处理
                //门磁,按钮, 设备启动, 远程开门记录
                printf("索引位=%u  非刷卡记录\r\n ", recordIndex);
                printf("  编号 = %u\r\n", recordCardNO);
                printf("  门号 = %u\r\n", recordDoorNO);
                printf("  时间 = %s\r\n", recordTime);
                printf("  描述 = %s\r\n", getReasonDetailChinese(reason));
         }
            else if (recordType == 3)
            {
                //其他处理
                //报警记录
               
                printf("索引位=%u  报警记录\r\n ", recordIndex);
                printf("  编号 = %u\r\n", recordCardNO);
                printf("  门号 = %u\r\n", recordDoorNO);
                printf("  时间 = %s\r\n", recordTime);
                printf("  描述 = %s\r\n", getReasonDetailChinese(reason));
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
			//请输入有效IP			
			cout << "请输入有效IP" << endl;
		}
	}
	void setSerUdp(ACE_SOCK_Dgram_Bcast udp)
	{
		serUdp = udp;
	}
};
ControlInfo MC[20];

void log(char *info,int data)  //日志信息
{
	//txtInfo.Text += string.Format("{0}\r\n", info);
	//txtInfo.AppendText(string.Format("{0}\r\n", info));
	//txtInfo.AppendText(string.Format("{0} {1}\r\n", DateTime.Now.ToString("HH:mm:ss"), info)); //2015-11-03 20:55:49 显示时间
	//txtInfo.ScrollToCaret();//滚动到光标处
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
	log("获取设备号...");
	if (ret > 0)
	{		
		controllerSN = (int)byteToLong(pkt.recv, 4, 4);		
		log("1.1 获取设备号.....controllerSN = ", controllerSN);
		//get sn
	}
	return controllerSN;
}
//ControllerIP 控制器IP地址
//controllerSN 控制器序列号
int zht_InitPort(int id, int iPort, int gPort, char* ControllerIP)
{
	int controllerSN = 0;
	int ret = 0;
	int success = 0;  //0 失败, 1表示成功    
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
	cout << "关闭完成" << endl;
	return 0;
}
int zht_SetTime(int hComm, int id, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nWeekDay)
{
	int ret = 0;
	int success = 0;  //0 失败, 1表示成功  
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
	log("设置日期时间...");
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
			log("1.6 设置日期时间 成功...");
			success = 1;
		}
		else
		{
			log("1.6 设置日期时间 失败...");
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
//	int success = 0;  //0 失败, 1表示成功
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
//	//1.18	设置接收服务器的IP和端口 [功能号: 0x90] **********************************************************************************
//	//(如果不想让控制器发出数据, 只要将接收服务器的IP设为0.0.0.0 就行了)
//	//接收服务器的端口: 61005
//	//每隔5秒发送一次: 05
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
//	//接收服务器的端口: 61005
//	pkt.data[4] = (byte)((watchServerPort & 0xff));
//	pkt.data[5] = (byte)((watchServerPort >> 8) & 0xff);
//
//	//每隔5秒发送一次: 05 (定时上传信息的周期为5秒 [正常运行时每隔5秒发送一次  有刷卡时立即发送])
//	pkt.data[6] = 5;
//	ret = pkt.run();
//	success = 0;
//	log("设置接收服务器的IP和端口");
//	if (ret > 0)
//	{
//		if (pkt.recv[8] == 1)
//		{
//			log("1.18 设置接收服务器的IP和端口 	 成功...");
//			success = 1;
//		}
//	}
//	else
//	{
//		return -1;
//	}
//
//
//	//1.19	读取接收服务器的IP和端口 [功能号: 0x92] **********************************************************************************
//	pkt.Reset();
//	pkt.functionID = 0x92;
//
//	ret = pkt.run();
//	success = 0;
//	log("读取接收服务器的IP和端口");
//	if (ret > 0)
//	{
//		if (byte.Parse(strIP[0]) == pkt.recv[8] && byte.Parse(strIP[1]) == pkt.recv[9] && byte.Parse(strIP[2]) == pkt.recv[10] && byte.Parse(strIP[3]) == pkt.recv[11])
//		{
//			if (pkt.recv[12] == (byte)((watchServerPort & 0xff)) && pkt.recv[13] == (byte)((watchServerPort >> 8) & 0xff))
//			{
//				log("1.19 读取接收服务器的IP和端口 	 成功...");
//				success = 1;
//				return 0;
//			}
//		}
//	}
//	return -1;
//}
//ControllerIP 被设置的控制器IP地址
//controllerSN 被设置的控制器序列号
//watchServerIP   要设置的服务器IP
//watchServerPort 要设置的端口
int setRevIpandRevPort(WGPacketShort pkt , int id)  //接收服务器测试 -- 设置
{
	int ret = 0;
	int success = 0;  //0 失败, 1表示成功

	//1.18	设置接收服务器的IP和端口 [功能号: 0x90] **********************************************************************************
	//	接收服务器的IP: 192.168.168.101  [当前电脑IP]
	//(如果不想让控制器发出数据, 只要将接收服务器的IP设为0.0.0.0 就行了)
	//接收服务器的端口: 61005
	//每隔5秒发送一次: 05
	pkt.Reset();
	pkt.functionID = 0x90;
	pkt.iDevSn = MC[id].SN;

	//服务器IP: 192.168.168.101
	//pkt.data[0] = 192; 
	//pkt.data[1] = 168; 
	//pkt.data[2] = 168; 
	//pkt.data[3] = 101; 
	int watchServerPort = MC[id].localPort;
	char *watchServerIP = MC[id].localIP;

	ACE_INET_Addr watchServer_addr(watchServerPort, watchServerIP); //端口  IP地址
	unsigned int iwatchServerIPInfo = watchServer_addr.get_ip_address();
	pkt.data[0] = (iwatchServerIPInfo >> 24) & 0xff;
	pkt.data[1] = (iwatchServerIPInfo >> 16) & 0xff;
	pkt.data[2] = (iwatchServerIPInfo >> 8) & 0xff;
	pkt.data[3] = iwatchServerIPInfo & 0xff;


	//接收服务器的端口: 61005
	pkt.data[4] = (watchServerPort & 0xff);
	pkt.data[5] = (watchServerPort >> 8) & 0xff;
	cout << "服务器端口是:" << watchServerPort << endl;
	unsigned char lPort = pkt.data[4];
	unsigned char hPort = pkt.data[5];
	//每隔5秒发送一次: 05 (定时上传信息的周期为5秒 [正常运行时每隔5秒发送一次  有刷卡时立即发送])
	pkt.data[6] = 5;

	ret = pkt.run(MC[id].udp);
	success = 0;
	if (ret >0)
	{
		if (pkt.recv[8] == 1)
		{
			log("1.18 设置接收服务器的IP和端口 	 成功...");
			success = 1;
		}
		else
		{
			return -1;
		}
	}
	//1.19	读取接收服务器的IP和端口 [功能号: 0x92] **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x92;
	pkt.iDevSn = MC[id].SN;

	ret = pkt.run(MC[id].udp);
	success = 0;
	log("1.19 读取接收服务器的IP和端口");
	if (ret >0)
	{
		if (((iwatchServerIPInfo >> 24) & 0xff) == pkt.recv[8] && ((iwatchServerIPInfo >> 16) & 0xff) == pkt.recv[9] && ((iwatchServerIPInfo >> 8) & 0xff) == pkt.recv[10] && ((iwatchServerIPInfo) & 0xff) == pkt.recv[11])
		{
			if (pkt.recv[12] == lPort && pkt.recv[13] == hPort)
			{
				log("1.19 读取接收服务器的IP和端口 	 成功...");
				success = 1;
				return 0;
			}
		}
		log("1.19 读取接收服务器的IP和端口 	 成功...");
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
	//组合一个dict
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
	//注意防火墙 要允许此端口的所有包进入才行
	ACE_INET_Addr server_addr(static_cast<u_short> (watchServerPort), watchServerIP);
	ACE_SOCK_Dgram_Bcast udp(server_addr);
	MC[id].setSerUdp(udp);
	unsigned char buff[WGPacketShort::WGPacketSize];
	size_t buflen = sizeof(buff);
	ssize_t recv_cnt;
	log("进入接收服务器监控状态....");
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
					printf("接收到来自控制器SN = %d 的数据包..\r\n", sn);

					/*memcpy(&recordIndexGet, &(buff[8]), 4);
					if (recordIndex < recordIndexGet)
					{
						recordIndex = recordIndexGet;
						displayRecordInformation(buff);
					}
					*/
					//接收到数据，根据ID值进行返回
					memset(root, '\0', 200);
					index ++;
					if(index == 10)
					{
						zht_ClosePort(MC[id].SN, 1);
					}
					//TIME
					char controllerTime[] = "2000.01.01 00:00:00"; //控制器当前时间
					sprintf(controllerTime, "20%02X.%02X.%02X %02X:%02X:%02X",
						buff[51], buff[52], buff[53], buff[37], buff[38], buff[39]);
					toJson(root, "Time", controllerTime,0);
					//ID
					memset(temp, '\0', 10);
					_itoa(id, temp, 10);
					//sprintf(temp, "%d", id);
					toJson(root, "nID", temp, 0);
					
					//14	门号(1,2,3,4)	1	
					int recordDoorNO = buff[14];
					memset(temp, '\0', 10);
					_itoa(recordDoorNO, temp, 10);
					//sprintf(temp, "%d", id);
					toJson(root, "ChannelID", temp, 0);

					//16-19	卡号(类型是刷卡记录时)
					//或编号(其他类型记录)	4	
					int recordCardNO = 0;
					memcpy(&recordCardNO, &(buff[16]), 4);
					memset(temp, '\0', 10);
					_itoa(recordCardNO, temp, 10);					
					toJson(root, "UID", temp, 0);


					//15	进门/出门(1表示进门, 2表示出门)	1	0x01
					int recordInOrOut = buff[15];
					memset(temp, '\0', 10);					
					if (recordInOrOut == 1)
					{//进门
						memcpy(temp, "IN", 2);
					}
					else
					{
						memcpy(temp, "OUT", 3);
					}
					toJson(root, "direction", temp, 0);

					//13	有效性(0 表示不通过, 1表示通过)	1	
					int recordValid = buff[13];
					memset(temp, '\0', 10);
					if (recordValid == 1)
					{//通过
						toJson(root, "Count", "2", 1); //结束
					}
					else
					{//未通过
						toJson(root, "Count", "1", 1); //结束
					}
					//json封装完成
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
	int success = 0;  //0 失败, 1表示成功  
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
	int success = 0;  //0 失败, 1表示成功  
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
		log("1.11 权限添加或修改 = ", cardNOOfPrivilege);
		if (cardNOOfPrivilege == 0)
			break;
		success = 0;
		pkt.Reset();
		pkt.functionID = 0x50;
		pkt.iDevSn = hComm;
		//0D D7 37 00 要添加或修改的权限中的卡号 = 0x0037D70D = 3659533 (十进制)
		//int cardNOOfPrivilege = 0x0037D70D;
		memcpy(&(pkt.data[0]), &cardNOOfPrivilege, 4);
		//20 10 01 01 起始日期:  2010年01月01日   (必须大于2001年)
		pkt.data[4] = 0x20;
		pkt.data[5] = 0x10;
		pkt.data[6] = 0x01;
		pkt.data[7] = 0x01;
		//20 29 12 31 截止日期:  2029年12月31日
		pkt.data[8] = 0x20;
		pkt.data[9] = 0x29;
		pkt.data[10] = 0x12;
		pkt.data[11] = 0x31;
		//01 允许通过 一号门 [对单门, 双门, 四门控制器有效] 
		pkt.data[12] = 0x01;
		//01 允许通过 二号门 [对双门, 四门控制器有效]
		pkt.data[13] = 0x01;  //如果禁止2号门, 则只要设为 0x00
							  //01 允许通过 三号门 [对四门控制器有效]
		pkt.data[14] = 0x01;
		//01 允许通过 四号门 [对四门控制器有效]
		pkt.data[15] = 0x01;

		ret = pkt.run(MC[id].udp);
		success = 0;
		log("1.11 权限添加或修改");
		if (ret >0)
		{
			if (pkt.recv[8] == 1)
			{
				//这时 刷卡号为= 0x0037D70D = 3659533 (十进制)的卡, 1号门继电器动作.
				log("1.11 权限添加或修改	 成功...");
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
	//本案例未作搜索控制器  及 设置IP的工作  (直接由IP设置工具来完成)
	//本案例中测试说明
	//控制器SN  = 422101164
	//控制器IP  = 192.168.168.123
	//电脑  IP  = 192.168.168.101
	//用于作为接收服务器的IP (本电脑IP 192.168.168.101), 接收服务器端口 (61005)

int ACE_TMAIN (int, ACE_TCHAR *[]) //主程序口
{
	int ret =0;
	char ip[32] = "192.168.121.150";
	
	////log("请输入控制器IP:");
	//cout << ("请输入控制器IP:  ");
	//cin >> ip;
	int port = 60000;
	////log("请输入控制器IP:");
	//cout << ("请输入控制器port:  ");
	//cin >> port;
	int SN = zht_InitPort(1, 61000, port, ip);	
	HANDLE h; //线程句柄
    h = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)createBackcall,NULL,1,0); //创建子线程
    ResumeThread(h);  //启动子线程

    if (ret != 0)
    {
        cout << "pthread_create error: error_code=" << ret << endl;
    }
	getchar();
	zht_SetTime(SN, 1, 12, 11, 23, 12, 58, 32, 1);
	zht_AddtoWhitelist(SN, 1, (char *)"0001254545;9632145690;9632145190;9232145690;0632145690;1632145690;");
	//zht_SetCallbackAddr(SN, 1, back, localip);
	////log("请输入控制器IP:");
	//cout << ("请输入控制器IP:  ");
	//cin >> localip;
	getchar();
	zht_ClosePort(SN, 1);
	//int sn =0;
	//cout<<("请输入控制器SN(9位数):  ");
	//cin>>sn;;
	//
	//char ip[32];
	////log("请输入控制器IP:");
	//cout<< ("请输入控制器IP:  ");
	//cin >>ip;
	//	
	//ret = testBasicFunction(ip,sn); //基本功能测试
	//if (ret !=1)
	//{
	//  log("基本功能测试失败, 按 X 回车键退出...");
	//  char stop;
	//  cin >>stop;
	//  return 0;
 //   }
	//
	////接收服务器测试
	//char watchServerIP[32]; // = "192.168.168.101";
	//cout<< ("请输入接收服务器的IP:  ");
	//cin >>watchServerIP;
 //   int  watchServerPort = 61005;
	////cout<<("请输入接收服务器端口:  ");
	////cin>>watchServerPort;;

	//ret = testWatchingServer(ip,sn,watchServerIP, watchServerPort); //接收服务器设置

	//ret = WatchingServerRuning(watchServerIP, watchServerPort); //服务器运行....
	//
	log("测试结束, 按回车键退出...");
	getchar();
	return 1;
}
/*
int testBasicFunction(char *ControllerIP, unsigned int controllerSN)  //基本功能测试
{
	int ret =0;
	int success =0;  //0 失败, 1表示成功

	ACE_INET_Addr controller_addr (9630, ControllerIP); //端口  IP地址
	ACE_SOCK_CODgram udp;
	if (0 != udp.open (controller_addr))
	{
		//请输入有效IP
		log("请输入有效IP...");
		return -1;
	}

	//创建短报文 pkt
	WGPacketShort pkt;  


	//1.4	查询控制器状态[功能号: 0x20](实时监控用) **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x20;
	pkt.iDevSn = controllerSN; 
	ret = pkt.run(udp);

	success =0;
	if (ret == 1)
	{
		//读取信息成功...
		success =1;
		log("1.4 查询控制器状态 成功...");

		//	  	最后一条记录的信息		
		displayRecordInformation(pkt.recv);


		//	其他信息		
		int doorStatus[4];
		//28	1号门门磁(0表示关上, 1表示打开)	1	0x00
		doorStatus[1-1] = pkt.recv[28];
		//29	2号门门磁(0表示关上, 1表示打开)	1	0x00
		doorStatus[2-1] = pkt.recv[29];
		//30	3号门门磁(0表示关上, 1表示打开)	1	0x00
		doorStatus[3-1] = pkt.recv[30];
		//31	4号门门磁(0表示关上, 1表示打开)	1	0x00
		doorStatus[4-1] = pkt.recv[31];

		int pbStatus[4];
		//32	1号门按钮(0表示松开, 1表示按下)	1	0x00
		pbStatus[1-1] = pkt.recv[32];
		//33	2号门按钮(0表示松开, 1表示按下)	1	0x00
		pbStatus[2-1] = pkt.recv[33];
		//34	3号门按钮(0表示松开, 1表示按下)	1	0x00
		pbStatus[3-1] = pkt.recv[34];
		//35	4号门按钮(0表示松开, 1表示按下)	1	0x00
		pbStatus[4-1] = pkt.recv[35];
		//36	故障号
		//等于0 无故障
		//不等于0, 有故障(先重设时间, 如果还有问题, 则要返厂家维护)	1	
		int errCode = pkt.recv[36];
		//37	控制器当前时间
		//时	1	0x21
		//38	分	1	0x30
		//39	秒	1	0x58

		//40-43	流水号	4	
		long long  sequenceId=0;
		memcpy(&sequenceId, &(pkt.recv[40]),4);

		//48
		//特殊信息1(依据实际使用中返回)
		//键盘按键信息	1	
		//49	继电器状态	1	
		int relayStatus = pkt.recv[49];
		//50	门磁状态的8-15bit位[火警/强制锁门]
		//Bit0  强制锁门
		//Bit1  火警		
		int otherInputStatus = pkt.recv[50];
		if ((otherInputStatus & 0x1) > 0)
		{
			//强制锁门
		}
		if ((otherInputStatus & 0x2) > 0)
		{
			//火警
		}

		//51	V5.46版本支持 控制器当前年	1	0x13
		//52	V5.46版本支持 月	1	0x06
		//53	V5.46版本支持 日	1	0x22

		char controllerTime[]="2000-01-01 00:00:00"; //控制器当前时间
		sprintf(controllerTime,"20%02X-%02X-%02X %02X:%02X:%02X", 
			pkt.recv[51],pkt.recv[52],pkt.recv[53],pkt.recv[37],pkt.recv[38],pkt.recv[39]);
	}
	else
	{
		log("1.4 查询控制器状态 失败?????...");
		return -1;
	}

	//1.5	读取日期时间(功能号: 0x32) **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x32;
	pkt.iDevSn = controllerSN; 
	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		char controllerTime[]="2000-01-01 00:00:00"; //控制器当前时间
		sprintf(controllerTime,"%02X%02X-%02X-%02X %02X:%02X:%02X", 
			pkt.recv[8],pkt.recv[9],pkt.recv[10],pkt.recv[11],pkt.recv[12],pkt.recv[13],pkt.recv[14]);

		log("1.5 读取日期时间 成功...");
		//log(controllerTime);
		success =1;
	}

	//1.6	设置日期时间[功能号: 0x30] **********************************************************************************
	//按电脑当前时间校准控制器.....
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
			log("1.6 设置日期时间 成功...");
			success =1;
		}	
	}

	//1.7	获取指定索引号的记录[功能号: 0xB0] **********************************************************************************
	//(取索引号 0x00000001的记录)
	int  recordIndexToGet =0;
	pkt.Reset();
	pkt.functionID = 0xB0;
	pkt.iDevSn = controllerSN; 

	//	(特殊
	//如果=0, 则取回最早一条记录信息
	//如果=0xffffffff则取回最后一条记录的信息)
	//记录索引号正常情况下是顺序递增的, 最大可达0xffffff = 16,777,215 (超过1千万) . 由于存储空间有限, 控制器上只会保留最近的20万个记录. 当索引号超过20万后, 旧的索引号位的记录就会被覆盖, 所以这时查询这些索引号的记录, 返回的记录类型将是0xff, 表示不存在了.
	recordIndexToGet =1;
	memcpy(&(pkt.data[0]), &recordIndexToGet, 4);

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		log("1.7 获取索引为1号记录的信息	 成功...");
		//	  	索引为1号记录的信息		
		displayRecordInformation(pkt.recv);

		success =1;
	}

	//. 发出报文 (取最早的一条记录 通过索引号 0x00000000) [此指令适合于 刷卡记录超过20万时环境下使用]
	pkt.Reset();
	pkt.functionID = 0xB0;
	pkt.iDevSn = controllerSN; 
	recordIndexToGet =0;
	memcpy(&(pkt.data[0]), &recordIndexToGet, 4);
	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		log("1.7 获取最早一条记录的信息	 成功...");
		//	  	最早一条记录的信息		
		displayRecordInformation(pkt.recv);
        success =1;
	}

	//发出报文 (取最新的一条记录 通过索引 0xffffffff)
	pkt.Reset();
	pkt.functionID = 0xB0;
	pkt.iDevSn = controllerSN; 
	recordIndexToGet =0xffffffff;
	memcpy(&(pkt.data[0]), &recordIndexToGet, 4);
	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		log("1.7 获取最新记录的信息	 成功...");
		//	  	最新记录的信息		
				displayRecordInformation(pkt.recv);
		success =1;
	}

	int recordIndexGotToRead;

	////1.8	设置已读取过的记录索引号[功能号: 0xB2] **********************************************************************************
	//pkt.Reset();
	//pkt.functionID = 0xB2;
	//pkt.iDevSn = controllerSN; 
	//// (设为已读取过的记录索引号为5)
	//int recordIndexGot =0x5;
	//memcpy(&(pkt.data[0]), &recordIndexGot, 4);

	////12	标识(防止误设置)	1	0x55 [固定]
	//memcpy(&(pkt.data[4]), &(WGPacketShort::SpecialFlag), 4);

	//ret = pkt.run(udp);
	//success =0;
	//if (ret >0)
	//{
	//	if (pkt.recv[8] == 1)
	//	{
	//		log("1.8 设置已读取过的记录索引号	 成功...");
	//		success =1;
	//	}
	//}

	////1.9	获取已读取过的记录索引号[功能号: 0xB4] **********************************************************************************
	//pkt.Reset();
	//pkt.functionID = 0xB4;
	//pkt.iDevSn = controllerSN; 
	// recordIndexGotToRead =0x0;
	//ret = pkt.run(udp);
	//success =0;
	//if (ret >0)
	//{
	//	memcpy(&(recordIndexGotToRead), &(pkt.recv[8]),4);
	//	log("1.9 获取已读取过的记录索引号	 成功...");
	//	success =1;
	//}

	//1.9	提取记录操作
	//1. 通过 0xB4指令 获取已读取过的记录索引号 recordIndex
	//2. 通过 0xB0指令 获取指定索引号的记录  从recordIndex + 1开始提取记录， 直到记录为空为止
	//3. 通过 0xB2指令 设置已读取过的记录索引号  设置的值为最后读取到的刷卡记录索引号
	//经过上面三个步骤， 整个提取记录的操作完成
    log("1.9 提取记录操作	 开始...");
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

				//12	记录类型
				//0=无记录
				//1=刷卡记录
				//2=门磁,按钮, 设备启动, 远程开门记录
				//3=报警记录	1	
				//0xFF=表示指定索引位的记录已被覆盖掉了.  请使用索引0, 取回最早一条记录的索引值
				int recordType = pkt.recv[12];
				if (recordType == 0)
				{
					break; //没有更多记录
				}
				if (recordType == 0xff)
				{
					success = 0;  //此索引号无效  重新设置索引值
					//取最早一条记录的索引位
                    pkt.iDevSn = controllerSN; 
					recordIndexToGet =0;
					memcpy(&(pkt.data[0]), &recordIndexToGet, 4);
					ret = pkt.run(udp);
					success =0;
					if (ret >0)
                    {
                        log("1.7 获取最早一条记录的信息	 成功...");
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
                //.......对收到的记录作存储处理
			    displayRecordInformation(pkt.recv);
				//*****
				//###############
			}
			else
			{
				//提取失败
				break;
			}
			recordIndexToGetStart++;
		}while(cnt++ < 200000);
		if (success >0)
		{
			//通过 0xB2指令 设置已读取过的记录索引号  设置的值为最后读取到的刷卡记录索引号
			pkt.Reset();
			pkt.functionID = 0xB2;
			pkt.iDevSn = controllerSN; 
			memcpy(&(pkt.data[0]), &recordIndexValidGet, 4);

			//12	标识(防止误设置)	1	0x55 [固定]
	        memcpy(&(pkt.data[4]), &(WGPacketShort::SpecialFlag), 4);

			ret = pkt.run(udp);
			success =0;
			if (ret >0)
			{
				if (pkt.recv[8] == 1)
				{
					//完全提取成功....
					log("1.9 完全提取成功	 成功...");
					success =1;
				}
			}

		}
	}

	//1.10	远程开门[功能号: 0x40] **********************************************************************************
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
			//有效开门.....
			log("1.10 远程开门	 成功...");
			success =1;
		}
	}

	//1.11	权限添加或修改[功能号: 0x50] **********************************************************************************
	//增加卡号0D D7 37 00, 通过当前控制器的所有门
	pkt.Reset();
	pkt.functionID = 0x50;
	pkt.iDevSn = controllerSN; 
	//0D D7 37 00 要添加或修改的权限中的卡号 = 0x0037D70D = 3659533 (十进制)
	long long cardNOOfPrivilege =0x0037D70D;
	memcpy(&(pkt.data[0]), &cardNOOfPrivilege, 4);
	//20 10 01 01 起始日期:  2010年01月01日   (必须大于2001年)
	pkt.data[4] = 0x20;
	pkt.data[5] = 0x10;
	pkt.data[6] = 0x01;
	pkt.data[7] = 0x01;
	//20 29 12 31 截止日期:  2029年12月31日
	pkt.data[8] = 0x20;
	pkt.data[9] = 0x29;
	pkt.data[10] = 0x12;
	pkt.data[11] = 0x31;
	//01 允许通过 一号门 [对单门, 双门, 四门控制器有效] 
	pkt.data[12] = 0x01;
	//01 允许通过 二号门 [对双门, 四门控制器有效]
	pkt.data[13] = 0x01;  //如果禁止2号门, 则只要设为 0x00
	//01 允许通过 三号门 [对四门控制器有效]
	pkt.data[14] = 0x01;
	//01 允许通过 四号门 [对四门控制器有效]
	pkt.data[15] = 0x01;

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		if (pkt.recv[8] == 1)
		{
			//这时 刷卡号为= 0x0037D70D = 3659533 (十进制)的卡, 1号门继电器动作.
			log("1.11 权限添加或修改	 成功...");
			success =1;
		}
	}

	//1.12	权限删除(单个删除)[功能号: 0x52] **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x52;
	pkt.iDevSn = controllerSN; 
	//要删除的权限卡号0D D7 37 00  = 0x0037D70D = 3659533 (十进制)
	long long cardNOOfPrivilegeToDelete =0x0037D70D;
	memcpy(&(pkt.data[0]), &cardNOOfPrivilegeToDelete, 4);

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		if (pkt.recv[8] == 1)
		{
			//这时 刷卡号为= 0x0037D70D = 3659533 (十进制)的卡, 1号门继电器不会动作.
			log("1.12 权限删除(单个删除)	 成功...");
			success =1;
		}
	}

	//1.13	权限清空(全部清掉)[功能号: 0x54] **********************************************************************************
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
			//这时清空成功
			log("1.13 权限清空(全部清掉)	 成功...");
			success =1;
		}
	}

	//1.14	权限总数读取[功能号: 0x58] **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x58;
	pkt.iDevSn = controllerSN; 
	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		int privilegeCount =0;
		memcpy(&privilegeCount, &(pkt.recv[8]),4);
		log("1.14 权限总数读取	 成功...");

		success =1;
	}


	//再次添加为查询操作 1.11	权限添加或修改[功能号: 0x50] **********************************************************************************
	//增加卡号0D D7 37 00, 通过当前控制器的所有门
	pkt.Reset();
	pkt.functionID = 0x50;
	pkt.iDevSn = controllerSN; 
	//0D D7 37 00 要添加或修改的权限中的卡号 = 0x0037D70D = 3659533 (十进制)
	//long long 
		cardNOOfPrivilege =0x0037D70D;
	memcpy(&(pkt.data[0]), &cardNOOfPrivilege, 4);
	//20 10 01 01 起始日期:  2010年01月01日   (必须大于2001年)
	pkt.data[4] = 0x20;
	pkt.data[5] = 0x10;
	pkt.data[6] = 0x01;
	pkt.data[7] = 0x01;
	//20 29 12 31 截止日期:  2029年12月31日
	pkt.data[8] = 0x20;
	pkt.data[9] = 0x29;
	pkt.data[10] = 0x12;
	pkt.data[11] = 0x31;
	//01 允许通过 一号门 [对单门, 双门, 四门控制器有效] 
	pkt.data[12] = 0x01;
	//01 允许通过 二号门 [对双门, 四门控制器有效]
	pkt.data[13] = 0x01;  //如果禁止2号门, 则只要设为 0x00
	//01 允许通过 三号门 [对四门控制器有效]
	pkt.data[14] = 0x01;
	//01 允许通过 四号门 [对四门控制器有效]
	pkt.data[15] = 0x01;

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		if (pkt.recv[8] == 1)
		{
			//这时 刷卡号为= 0x0037D70D = 3659533 (十进制)的卡, 1号门继电器动作.
			log("1.11 权限添加或修改	 成功...");
			success =1;
		}
	}

	//1.15	权限查询[功能号: 0x5A] **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x5A;
	pkt.iDevSn = controllerSN; 
	// (查卡号为 0D D7 37 00的权限)
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
			//没有权限时: (卡号部分为0)
			log ("1.15      没有权限信息: (卡号部分为0)");
		}
		else
		{
			//具体权限信息...
			log ("1.15     有权限信息...");
		}
		log("1.15 权限查询	 成功...");
		success =1;
	}

	//1.16  获取指定索引号的权限[功能号: 0x5C] **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x5C;
	pkt.iDevSn = controllerSN; 
	long long QueryIndex = 1; //索引号(从1开始);
	memcpy(&(pkt.data[0]), &QueryIndex, 4);

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{

		long long cardNOOfPrivilegeToGet=0;
		memcpy(&cardNOOfPrivilegeToGet, &(pkt.recv[8]),4);
		if (4294967295 == cardNOOfPrivilegeToGet) //FFFFFFFF对应于4294967295
		{
			log ("1.16      没有权限信息: (权限已删除)");
		}
		else if (cardNOOfPrivilegeToGet == 0)
		{
			//没有权限时: (卡号部分为0)
			log ("1.16       没有权限信息: (卡号部分为0)--此索引号之后没有权限了");
		}
		else
		{
			//具体权限信息...
			 log ("1.16      有权限信息...");
		}
		log("1.16 获取指定索引号的权限	 成功...");
		success =1;
	}


	//1.17	设置门控制参数(在线/延时) [功能号: 0x80] **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x80;
	pkt.iDevSn = controllerSN; 
	//(设置2号门 在线  开门延时 3秒)
	pkt.data[0] = 0x02; //2号门
	pkt.data[1] = 0x03; //在线
	pkt.data[2] = 0x03; //开门延时

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
//2013-11-06 15:33:16		if (memcmp(&(pkt.data[0]), &(pkt.recv[8]),4) == 0)
		if (memcmp(&(pkt.data[0]), &(pkt.recv[8]),3) == 0) //2013-11-06 15:33:23  比较三个
		{
			//成功时, 返回值与设置一致
			log("1.17 设置门控制参数	 成功...");
			success =1;			
		}
		else
		{
			//失败
		}
	}

	//1.21	权限按从小到大顺序添加[功能号: 0x56] 适用于权限数过1000, 少于8万 **********************************************************************************
        //此功能实现 完全更新全部权限, 用户不用清空之前的权限. 只是将上传的权限顺序从第1个依次到最后一个上传完成. 如果中途中断的话, 仍以原权限为主
        //建议权限数更新超过50个, 即可使用此指令

        log("1.21	权限按从小到大顺序添加[功能号: 0x56]	开始...");
        log("       1万条权限...");

        //以10000个卡号为例, 此处简化的排序, 直接是以50001开始的10000个卡. 用户按照需要将要上传的卡号排序存放
        int cardCount = 10000;  //2015-06-09 20:20:20 卡总数量
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
                
            //其他参数简化时 统一, 可以依据每个卡的不同进行修改
            //20 10 01 01 起始日期:  2010年01月01日   (必须大于2001年)
            pkt.data[4] = 0x20;
            pkt.data[5] = 0x10;
            pkt.data[6] = 0x01;
            pkt.data[7] = 0x01;
            //20 29 12 31 截止日期:  2029年12月31日
            pkt.data[8] = 0x20;
            pkt.data[9] = 0x29;
            pkt.data[10] = 0x12;
            pkt.data[11] = 0x31;
            //01 允许通过 一号门 [对单门, 双门, 四门控制器有效] 
            pkt.data[12] = 0x01;
            //01 允许通过 二号门 [对双门, 四门控制器有效]
            pkt.data[13] = 0x01;  //如果禁止2号门, 则只要设为 0x00
            //01 允许通过 三号门 [对四门控制器有效]
            pkt.data[14] = 0x01;
            //01 允许通过 四号门 [对四门控制器有效]
            pkt.data[15] = 0x01;

			memcpy(&(pkt.data[32-8]), &cardCount, 4);//总的权限数
			int i2=i+1;
			memcpy(&(pkt.data[35-8]), &i2, 4);//当前权限的索引位(从1开始)

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
                    log("1.21	权限按从小到大顺序添加[功能号: 0x56]	 =0xE1 表示卡号没有从小到大排序...???");
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
            log("1.21	权限按从小到大顺序添加[功能号: 0x56]	 成功...");
        }
        else
        {
            log("1.21	权限按从小到大顺序添加[功能号: 0x56]	 失败...????");
        }
           



	//其他指令  **********************************************************************************


	// **********************************************************************************

	//结束  **********************************************************************************
	udp.close();
	return success;
}
*/
//ControllerIP 被设置的控制器IP地址
//controllerSN 被设置的控制器序列号
//watchServerIP   要设置的服务器IP
//watchServerPort 要设置的端口
/*
int testWatchingServer(char *ControllerIP, unsigned int controllerSN, char *watchServerIP,int watchServerPort)  //接收服务器测试 -- 设置
{
	int ret =0;
	int success =0;  //0 失败, 1表示成功

	ACE_INET_Addr controller_addr (9630, ControllerIP); //端口  IP地址
	ACE_SOCK_CODgram udp;
	if (0 != udp.open (controller_addr))
	{
		//请输入有效IP
		log("请输入有效IP...");
		return -1;
	}


	WGPacketShort pkt;
	//1.18	设置接收服务器的IP和端口 [功能号: 0x90] **********************************************************************************
	//	接收服务器的IP: 192.168.168.101  [当前电脑IP]
	//(如果不想让控制器发出数据, 只要将接收服务器的IP设为0.0.0.0 就行了)
	//接收服务器的端口: 61005
	//每隔5秒发送一次: 05
	pkt.Reset();
	pkt.functionID = 0x90;
	pkt.iDevSn = controllerSN; 

	//服务器IP: 192.168.168.101
	//pkt.data[0] = 192; 
	//pkt.data[1] = 168; 
	//pkt.data[2] = 168; 
	//pkt.data[3] = 101; 
	ACE_INET_Addr watchServer_addr (watchServerPort, watchServerIP); //端口  IP地址
	unsigned int iwatchServerIPInfo = watchServer_addr.get_ip_address();
	pkt.data[0] = (iwatchServerIPInfo >> 24) & 0xff;   
	pkt.data[1] = (iwatchServerIPInfo >> 16) & 0xff;   
	pkt.data[2] = (iwatchServerIPInfo >> 8) & 0xff;  
	pkt.data[3] = iwatchServerIPInfo & 0xff; 


	//接收服务器的端口: 61005
	pkt.data[4] =(watchServerPort & 0xff);
	pkt.data[5] =(watchServerPort >>8) & 0xff;

	//每隔5秒发送一次: 05 (定时上传信息的周期为5秒 [正常运行时每隔5秒发送一次  有刷卡时立即发送])
	pkt.data[6] = 5;

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		if (pkt.recv[8] == 1)
		{
			log("1.18 设置接收服务器的IP和端口 	 成功...");
			success =1;
		}
	}


	//1.19	读取接收服务器的IP和端口 [功能号: 0x92] **********************************************************************************
	pkt.Reset();
	pkt.functionID = 0x92;
	pkt.iDevSn = controllerSN; 

	ret = pkt.run(udp);
	success =0;
	if (ret >0)
	{
		log("1.19 读取接收服务器的IP和端口 	 成功...");
		success =1;
	}
	udp.close();
	return 1;
}

int WatchingServerRuning (char *watchServerIP,int watchServerPort)
{
	//注意防火墙 要允许此端口的所有包进入才行
  ACE_INET_Addr server_addr (static_cast<u_short> (watchServerPort),watchServerIP); 
  ACE_SOCK_Dgram_Bcast udp (server_addr);
  unsigned char buff[WGPacketShort::WGPacketSize];
  size_t buflen = sizeof (buff);
  ssize_t recv_cnt;
  log("进入接收服务器监控状态....");
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
				printf("接收到来自控制器SN = %d 的数据包..\r\n", sn);

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



