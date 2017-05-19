// PSTVSetTime.cpp : 实现文件
//

#include "stdafx.h"
#include "UAS.h"
#include "UASDlg.h"
#include "PSTVSetTime.h"
#include "afxdialogex.h"
#include "SipMsgProcess.h"
#include <time.h>
#include <algorithm>
#include <WinSock.h>
// CPSTVSetTime 对话框

//成员变量
extern StatusCallID SCallId;
extern InfoServer m_InfoServer;
extern InfoClient m_InfoClient;
extern CRITICAL_SECTION g_uas;
extern queue<UA_Msg> uas_sendqueue;
extern CString ShowTestTitle;
extern CString ShowTestData;

IMPLEMENT_DYNAMIC(CPSTVSetTime, CDialogEx)

CPSTVSetTime::CPSTVSetTime(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIG_PSTVTTIME, pParent)
{

}

CPSTVSetTime::~CPSTVSetTime()
{
}

void CPSTVSetTime::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_TIME, m_Time);
}


BEGIN_MESSAGE_MAP(CPSTVSetTime, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_NTPTIME, &CPSTVSetTime::OnBnClickedButtonNtptime)
	ON_BN_CLICKED(IDC_BUTTON_CurTIME, &CPSTVSetTime::OnBnClickedButtonCurtime)
	ON_BN_CLICKED(IDC_BUTTON_SETTIME, &CPSTVSetTime::OnBnClickedButtonSettime)
END_MESSAGE_MAP()


// CPSTVSetTime 消息处理程序


BOOL CPSTVSetTime::OnInitDialog()
{
	CDialog::OnInitDialog(); 
	GetDlgItem(IDC_EDIT_PRIVILEGE)->SetWindowTextA("192016809088");
	return 0;
}

void CPSTVSetTime::OnBnClickedButtonNtptime()
{
	//这里是从NTP(网络时间协议)，通过互联网获取网络时间的实现
	WORD    wVersionRequested;
	WSADATA wsaData;

	// 初始化版本  
	wVersionRequested = MAKEWORD(1, 1);
	if (0 != WSAStartup(wVersionRequested, &wsaData))
	{
		WSACleanup();
		return;
	}
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		return;
	}

	// 这个IP是中国大陆时间同步服务器地址，可自行修改  
	SOCKET soc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	struct sockaddr_in addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr("202.120.2.101");
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(123);

	NTP_Packet NTP_Send, NTP_Recv;
	NTP_Send.Control_Word = htonl(0x0B000000);
	NTP_Send.root_delay = 0;
	NTP_Send.root_dispersion = 0;
	NTP_Send.reference_identifier = 0;
	NTP_Send.reference_timestamp = 0;
	NTP_Send.originate_timestamp = 0;
	NTP_Send.receive_timestamp = 0;
	NTP_Send.transmit_timestamp_seconds = 0;
	NTP_Send.transmit_timestamp_fractions = 0;

	if (SOCKET_ERROR == sendto(soc, (const char*)&NTP_Send, sizeof(NTP_Send),
		0, (struct sockaddr*)&addrSrv, sizeof(addrSrv)))
	{
		closesocket(soc);
		return;
	}

	int sockaddr_Size = sizeof(addrSrv);
	if (SOCKET_ERROR == recvfrom(soc, (char*)&NTP_Recv, sizeof(NTP_Recv),
		0, (struct sockaddr*)&addrSrv, &sockaddr_Size))
	{
		closesocket(soc);
		return;
	}
	closesocket(soc);
	WSACleanup();

	SYSTEMTIME  newtime;
	float       Splitseconds;
	struct      tm  *lpLocalTime;
	time_t      ntp_time;

	// 获取时间服务器的时间  
	ntp_time = ntohl(NTP_Recv.transmit_timestamp_seconds) - 2208988800;
	lpLocalTime = localtime(&ntp_time);
	if (lpLocalTime == NULL)
	{
		return;
	}
	// 获取新的时间  
	newtime.wYear = lpLocalTime->tm_year + 1900;
	newtime.wMonth = lpLocalTime->tm_mon + 1;
	newtime.wDayOfWeek = lpLocalTime->tm_wday;
	newtime.wDay = lpLocalTime->tm_mday;
	newtime.wHour = lpLocalTime->tm_hour;
	newtime.wMinute = lpLocalTime->tm_min;
	newtime.wSecond = lpLocalTime->tm_sec;

	// 设置时间精度  
	Splitseconds = (float)ntohl(NTP_Recv.transmit_timestamp_fractions);
	Splitseconds = (float)0.000000000200 * Splitseconds;
	Splitseconds = (float)1000.0 * Splitseconds;
	newtime.wMilliseconds = (unsigned   short)Splitseconds;
	CString strTime;
	strTime.Format("%d-%d-%dT%d:%d:%dZ", newtime.wYear, newtime.wMonth, newtime.wDay, newtime.wHour, newtime.wMinute, newtime.wSecond);
	m_Time.SetWindowTextA(strTime);
}


void CPSTVSetTime::OnBnClickedButtonCurtime()
{
	time_t CurrentTime;
	CurrentTime = time(NULL);
	struct tm *pts;
	pts = localtime(&CurrentTime);//这里获取时间仅仅是通过获取本地时间
	CString strTime;
	strTime.Format("%d-%d-%dT%d:%d:%dZ", pts->tm_year + 1900, pts->tm_mon + 1, pts->tm_mday, pts->tm_hour, pts->tm_min, pts->tm_sec);
	m_Time.SetWindowTextA(strTime);
}


void CPSTVSetTime::OnBnClickedButtonSettime()
{
	CString strTime;
	CString privilege;
	m_Time.GetWindowTextA(strTime);
	GetDlgItem(IDC_EDIT_PRIVILEGE)->GetWindowTextA(privilege);
	CString XmlTimeSet;
	XmlTimeSet = "<?xml version=\"1.0\"?>\r\n";
	XmlTimeSet += "<Action>\r\n";
	XmlTimeSet+="<Notify>\r\n";
	XmlTimeSet += "<Variable>TimeSet</Variable>\r\n";
	XmlTimeSet += "<Time>" + strTime + "</Time>\r\n";
	XmlTimeSet+="<Privilege>" + privilege + "</Privilege>\r\n";//用户的权限码
	XmlTimeSet+="</Notify>\r\n";
	XmlTimeSet += "</Action>\r\n";
	char *destXML = (LPSTR)(LPCTSTR)XmlTimeSet;
	CSipMsgProcess *Sip = new CSipMsgProcess;
	char *SipXml = new char[MAXBUFSIZE];
	memset(SipXml, 0, MAXBUFSIZE);
	Sip->SipTimeSetXmlMsg(&SipXml, m_InfoServer, m_InfoClient, destXML);
	//send message to client
	if (m_InfoClient.Port == "" || m_InfoClient.IP == "")
	{
		delete SipXml;
		MessageBox("没有注册的客户端用户", "UAS 提示", MB_OK | MB_ICONINFORMATION);
		return;
	}
	//pWnd->SendData(SipXmlAlarm);
	UA_Msg uas_sendtemp;
	strcpy(uas_sendtemp.data, SipXml);
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);
	LeaveCriticalSection(&g_uas);
	//pWnd->ShowSendData(SipXmlAlarm);	
	delete SipXml;
	//update log	
	ShowTestData = "DO  ----------->\r\n";
	ShowTestTitle = "Time Set Test";
	SCallId.nStatus = TimeSet;
}
