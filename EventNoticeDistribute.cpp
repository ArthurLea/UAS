// EventNoticeDistribute.cpp : 实现文件
//

#include "stdafx.h"
#include "UAS.h"
#include "EventNoticeDistribute.h"
#include "afxdialogex.h"
#include "UASDlg.h"
#include <time.h>
#include <algorithm>
#include "Common.h"

extern StatusCallID SEventAlarmNoticeDistributeID;
extern InfoNotify NotifyInfo;
//成员变量
extern InfoServer m_InfoServer;
extern InfoClient m_InfoClient;
extern CString ShowTestTitle;
extern CString ShowTestData;
extern CRITICAL_SECTION g_uas;
extern queue<UA_Msg> uas_sendqueue;
// CEventNoticeDistribute 对话框

IMPLEMENT_DYNAMIC(CEventNoticeDistribute, CDialogEx)

CEventNoticeDistribute::CEventNoticeDistribute(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIG_ALARMEVENT_NOTICE_DISTRIBUTE, pParent)
{

}

CEventNoticeDistribute::~CEventNoticeDistribute()
{
}

void CEventNoticeDistribute::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC__ALARMEVENT_SELADRESS, m_selAddress);

}


BEGIN_MESSAGE_MAP(CEventNoticeDistribute, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_NOTICE_DISTRIBUTE, &CEventNoticeDistribute::OnBnClickedButtonNoticeDistribute)
	ON_CBN_SELCHANGE(IDC__ALARMEVENT_SELADRESS, &CEventNoticeDistribute::OnCbnSelchangeSeladress)
END_MESSAGE_MAP()


// CEventNoticeDistribute 消息处理程序


void CEventNoticeDistribute::OnBnClickedButtonNoticeDistribute()
{
	HWND   hnd = ::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd = (CUASDlg*)CWnd::FromHandle(hnd);
	//Create XML message
	CString SN;
	CString DeviceID;
	CString AlarmPriority;
	CString AlarmTime;
	CString AlarmMethod;
	GetDlgItem(IDC_EDIT_EVENT_SN)->GetWindowText(SN);
	GetDlgItem(IDC_EDIT_EVENT_DEVICEID)->GetWindowText(DeviceID);
	GetDlgItem(IDC_EDIT_EVENT_PRIORITY)->GetWindowText(AlarmPriority);
	GetDlgItem(IDC_EDIT_EVENT_ALARMTIME)->GetWindowText(AlarmTime);
	GetDlgItem(IDC_EDIT_EVENT_ALARM_METHOD)->GetWindowText(AlarmMethod);

	CString Xml;
	Xml = "<?xml version=\"1.0\"?>\r\n";
	Xml += "<Notify>\r\n";
	Xml += "<CmdType>Alarm</CmdType>\r\n";
	Xml += "<SN>17430</SN>\r\n";
	Xml += "<DeviceID>" + DeviceID + "</DeviceID>\r\n";
	Xml += "<AlarmPriority>" + AlarmPriority + "</AlarmPriority>\r\n";
	Xml += "<AlarmTime>" + AlarmTime + "</AlarmTime>\r\n";
	Xml += "<AlarmMethod>" + AlarmMethod + "</AlarmMethod>\r\n";
	Xml += "</Notify>\r\n";
	char *destXML = (LPSTR)(LPCTSTR)Xml;
	CSipMsgProcess *SipAlarm = new CSipMsgProcess;
	char *SipXmlAlarm = new char[MAXBUFSIZE];
	memset(SipXmlAlarm, 0, MAXBUFSIZE);
	SipAlarm->SipAlarmEventNotifyMsg(&SipXmlAlarm, m_InfoServer, m_InfoClient, destXML);
	//send message to client
	if (m_InfoClient.Port == "" || m_InfoClient.IP == "")
	{
		delete SipXmlAlarm;
		MessageBox("没有注册的客户端用户", "UAS 提示", MB_OK | MB_ICONINFORMATION);
		return;
	}
	UA_Msg uas_sendtemp;
	strcpy(uas_sendtemp.data, SipXmlAlarm);
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);
	LeaveCriticalSection(&g_uas);
	delete SipXmlAlarm;
	//update log
	ShowTestData = "MESSAGE  ----------->\r\n";
	ShowTestTitle = "AlarmEvent notify Test";
	SEventAlarmNoticeDistributeID.nStatus = AlarmNoticeDistribute;

}

BOOL CEventNoticeDistribute::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_EDIT_EVENT_SN)->SetWindowTextA("1");
	GetDlgItem(IDC_EDIT_EVENT_DEVICEID)->SetWindowTextA("64010000001340000101");
	GetDlgItem(IDC_EDIT_EVENT_PRIORITY)->SetWindowTextA("4");
	GetDlgItem(IDC_EDIT_EVENT_ALARMTIME)->SetWindowTextA("2009-12-04T16:23:32");
	GetDlgItem(IDC_EDIT_EVENT_ALARM_METHOD)->SetWindowTextA("2");

	return TRUE;
}


void CEventNoticeDistribute::OnCbnSelchangeSeladress()
{
	int index = m_selAddress.GetCurSel();
	CString Address = NotifyInfo.Devices[index].Address;
	GetDlgItem(IDC_EDIT_EVENT_DEVICEID)->SetWindowTextA(Address);
}
