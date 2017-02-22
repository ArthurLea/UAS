#ifndef DXP_H
#define DXP_H
#include "stdafx.h"
#include <queue>
#include <vector>
using namespace std;
#define MAXBUFSIZE 4000
#define  XMLSIZE 3000
#define  NUMSIZE 20
#define  IPSIZE 50


struct RECVPARAM
{
	SOCKET sock;
	HWND hwnd;
};
//�¼�����
enum{Register,NodeType,Invite,PTZ,PreBitSet,HistoryQuery,CatalogQuery,DeviceInfQuery,FlowQuery,HistoryPlay,EncoderSet,Alarm,TimeSet,TimeGet};
//����˵���Ϣ
struct InfoServer
{
	CString UserName;
	CString UserAddress;
	CString IP;
	CString Port;
};
struct ProductMember
{
	char IP[IPSIZE];
};
struct CallID
{
	char Host[IPSIZE];
	char Num[NUMSIZE];
	char Tag[NUMSIZE];
};
struct Authenticate
{
	string realm;
	string nonce;
	string opaque;
	string qop;

	string username;
	string uri;
	string response;
	string cnonce;
	string nc;
	string password;
};
//�ͻ��˵���Ϣ
struct InfoClient
{
	CString UserName;
	CString UserAddress;
	CString IP;
	CString Port;	
};
//У��CallID��״̬��
struct StatusCallID
{
	int  nStatus;
	char CurID[NUMSIZE];
	char CurHost[IPSIZE];
	char CurTag[NUMSIZE];
	char CurToTag[NUMSIZE];
};
struct UA_Msg 
{
	//�����հ��򷢰����ݻ�����
	char data[MAXBUFSIZE];
};
//��ʷ��Ƶ�ļ���Ϣ
struct InfoVideo
{
	char Name[150];
	char BeginTime[50];
	char EndTime[50];
	char FileSize[20];
};
//�豸��Ϣ
struct InfoDvice
{
	CString Name;
	CString Address;
	CString ResType;
	CString ResSubType;
	CString Privilege;
	CString Status;
	CString Longitude;
	CString Latitude;
	CString Elevation;
	CString Roadway;
	CString PileNo;
	CString AreaNo;
	CString OperateType;
	CString UpdateTime;
};
//������Ϣ
//��ʷ��Ƶ�ļ���Ϣ
struct InfoNotify
{
	CString Parent;
	CString TotalSubNum;
	CString TotalOnlineSubNum;
	CString SubNum;
	vector<InfoDvice> Devices;
};
DWORD WINAPI DispatchRecvMsg( LPVOID );
DWORD WINAPI RecvMsg(LPVOID lpParameter);
DWORD WINAPI SendMsg(LPVOID);

#endif