#pragma once
#ifndef  __ZHT__
#define __ZHT__
typedef void(*FUNPTR_CALLBACK)(int nID, int eventType, void* param);
int zht_AddtoWhitelist(int hComm, int id, char *cardid);

int zht_ClosePort(int hComm, int id);
int zht_InitPort(int id, int iPort, int gPort, char* ControllerIP);
int zht_SetCallbackAddr(int hComm, int id, FUNPTR_CALLBACK callback, char *localIP);
int zht_SetTime(int hComm, int id, int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nWeekDay);

#endif // ! __ZHT__
