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
//事件类型
enum{Register,NodeType,Invite,PTZ,PreBitSet,HistoryQuery,CatalogQuery,DeviceInfQuery,FlowQuery,HistoryPlay,EncoderSet,Alarm,TimeSet,TimeGet};
//服务端的信息
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
//客户端的信息
struct InfoClient
{
	CString UserName;
	CString UserAddress;
	CString IP;
	CString Port;	
};
//校验CallID及状态机
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
	//网络收包或发包数据缓冲区
	char data[MAXBUFSIZE];
};
//历史视频文件信息
struct InfoVideo
{
	char Name[150];
	char BeginTime[50];
	char EndTime[50];
	char FileSize[20];
};
//设备信息
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
//推送信息
//历史视频文件信息
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