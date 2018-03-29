// Alarm.cpp : implementation file
//

#include "stdafx.h"
#include "UAS.h"
#include "Alarm.h"
#include "UASDlg.h"
#include <time.h>
#include <algorithm>
#include "Common.h"
extern InfoNotify NotifyInfo;
extern StatusCallID SAlarmCallID;
//成员变量
extern InfoServer m_InfoServer;
extern InfoClient m_InfoClient;
extern CString ShowTestTitle;
extern CString ShowTestData;
extern CRITICAL_SECTION g_uas;
extern queue<UA_Msg> uas_sendqueue;

// CAlarm dialog

IMPLEMENT_DYNAMIC(CAlarm, CDialog)

CAlarm::CAlarm(CWnd* pParent /*=NULL*/)
	: CDialog(CAlarm::IDD, pParent)
{
	
}

CAlarm::~CAlarm()
{
}

void CAlarm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SELADRESS, m_selAddress);
	DDX_Control(pDX, IDC_SELalarmtype, m_AlarmTypeSel);
}


BEGIN_MESSAGE_MAP(CAlarm, CDialog)
	ON_BN_CLICKED(IDC_BTN_ALARM_SET, &CAlarm::OnBnClickedBtnAlarmSet)
	ON_CBN_SELCHANGE(IDC_SELADRESS, &CAlarm::OnCbnSelchangeSeladress)
	ON_BN_CLICKED(IDC_BTN_ALARM_CANCEL, &CAlarm::OnBnClickedBtnAlarmCancel)
	ON_CBN_SELCHANGE(IDC_SELalarmtype, &CAlarm::OnCbnSelchangeSelalarmtype)
END_MESSAGE_MAP()

// CAlarm message handlers

void CAlarm::OnBnClickedBtnAlarmSet()
{
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	//Create XML message
	CString DeviceID;
	CString StartAlarmPrivilege;
	CString EndAlarmPrivilege;
	CString StartTime;
	CString EndTime;
	CString AlarmMethod;
	GetDlgItem(IDC_EDT_ADDRESS)->GetWindowText(DeviceID);
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(StartAlarmPrivilege);
	GetDlgItem(IDC_EDT_LEVEL)->GetWindowText(EndAlarmPrivilege);
	GetDlgItem(IDC_EDT_IP)->GetWindowText(StartTime);
	GetDlgItem(IDC_EDT_PORT)->GetWindowText(EndTime);
	GetDlgItem(IDC_EDT_ALARM_TYPE)->GetWindowText(AlarmMethod);

	//判断是否已经预定了此事件
	string nowOperatorEventMsg = StartAlarmPrivilege + EndAlarmPrivilege + AlarmMethod + DeviceID 
		+ StartTime + AlarmMethod;
	Common::nowOperatorEventMsg = nowOperatorEventMsg;//做一次缓存
	vector<string>::iterator it = find(Common::curAlreadyReserveEvent.begin(),
		Common::curAlreadyReserveEvent.end(), nowOperatorEventMsg);
	if (it != Common::curAlreadyReserveEvent.end())//存在这样的消息，表示已经预定此报警事件
	{
		AfxMessageBox("已经预定了此报警事件...", MB_OK | MB_ICONERROR);
		return;
	}
	else//不存在，便是没有预定这样的报警事件
	{
		CString XmlAlarmSet;
		XmlAlarmSet = "<?xml version=\"1.0\"?>\r\n";
		XmlAlarmSet += "<Query>\r\n";
		XmlAlarmSet += "<CmdType>AlarmSubscribe</CmdType>\r\n";
		XmlAlarmSet += "<SN>17430</SN>\r\n";
		XmlAlarmSet += "<DeviceID>" + DeviceID + "</DeviceID>\r\n";
		XmlAlarmSet += "<StartAlarmPriority>" + StartAlarmPrivilege + "</StartAlarmPriority>\r\n";
		XmlAlarmSet += "<EndAlarmPriority>" + EndAlarmPrivilege + "</EndAlarmPriority>\r\n";
		XmlAlarmSet += "<StartTime>" + StartTime + "</StartTime>\r\n";
		XmlAlarmSet += "<EndTime>" + EndTime + "</EndTime>\r\n";
		XmlAlarmSet += "<AlarmMethod>" + AlarmMethod + "</AlarmMethod>\r\n";
		XmlAlarmSet += "</Query>\r\n";
		char *destXMLAlarmSet = (LPSTR)(LPCTSTR)XmlAlarmSet;
		CSipMsgProcess *SipAlarm = new CSipMsgProcess;
		char *SipXmlAlarm = new char[MAXBUFSIZE];
		memset(SipXmlAlarm, 0, MAXBUFSIZE);
		SipAlarm->SipSubscribeMsg(&SipXmlAlarm, m_InfoServer, m_InfoClient, destXMLAlarmSet);
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
		ShowTestData = "SUBSCRIBE  ----------->\r\n";
		ShowTestTitle = "Alarm Subscribe Test";
		SAlarmCallID.nStatus = Alarm;

		//预定成功后在进行存储，在SipParse中的Alarm中进行存储
		//Common::curAlreadyReserveEvent.push_back(nowReservingEventMsg);
		//GetDlgItem(IDC_BTN_ALARM_CANCEL)->EnableWindow(TRUE);//打开取消报警预定按钮
	}
}

void CAlarm::OnCbnSelchangeSeladress()
{
	int index=m_selAddress.GetCurSel();
	CString Address=NotifyInfo.Devices[index].Address;
	GetDlgItem(IDC_EDT_ADDRESS)->SetWindowTextA(Address);
}

void CAlarm::OnBnClickedBtnAlarmCancel()
{
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	//Create XML message
	CString DeviceID;
	CString StartAlarmPrivilege;
	CString EndAlarmPrivilege;
	CString StartTime;
	CString EndTime;
	CString AlarmMethod;
	GetDlgItem(IDC_EDT_ADDRESS)->GetWindowText(DeviceID);
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(StartAlarmPrivilege);
	GetDlgItem(IDC_EDT_LEVEL)->GetWindowText(EndAlarmPrivilege);
	GetDlgItem(IDC_EDT_IP)->GetWindowText(StartTime);
	GetDlgItem(IDC_EDT_PORT)->GetWindowText(EndTime);
	GetDlgItem(IDC_EDT_ALARM_TYPE)->GetWindowText(AlarmMethod);

	//判断是否已经预定了此事件
	string nowOperatorEventMsg = StartAlarmPrivilege + EndAlarmPrivilege + AlarmMethod + DeviceID
		+ StartTime + AlarmMethod;
	Common::nowOperatorEventMsg = nowOperatorEventMsg;//做一次缓存
	vector<string>::iterator it = find(Common::curAlreadyReserveEvent.begin(),
		Common::curAlreadyReserveEvent.end(), nowOperatorEventMsg);
	if (it != Common::curAlreadyReserveEvent.end())//表示预定了此报警事件
	{
		//需要在成功取消后再删除
		//删除这个预警事件Msg的缓存
		//Common::curAlreadyReserveEvent.erase(it);

		CString XmlAlarmSet;
		XmlAlarmSet = "<?xml version=\"1.0\"?>\r\n";
		XmlAlarmSet += "<Query>\r\n";
		XmlAlarmSet += "<CmdType>AlarmSubscribe</CmdType>\r\n";
		XmlAlarmSet += "<SN>17430</SN>\r\n";
		XmlAlarmSet += "<DeviceID>" + DeviceID + "</DeviceID>\r\n";
		XmlAlarmSet += "<StartAlarmPriority>" + StartAlarmPrivilege + "</StartAlarmPriority>\r\n";
		XmlAlarmSet += "<EndAlarmPriority>" + EndAlarmPrivilege + "</EndAlarmPriority>\r\n";
		XmlAlarmSet += "<StartTime>" + StartTime + "</StartTime>\r\n";
		XmlAlarmSet += "<EndTime>" + EndTime + "</EndTime>\r\n";
		XmlAlarmSet += "<AlarmMethod>" + AlarmMethod + "</AlarmMethod>\r\n";
		XmlAlarmSet += "</Query>\r\n";
		char *destXMLAlarmSet = (LPSTR)(LPCTSTR)XmlAlarmSet;
		CSipMsgProcess *SipAlarm=new CSipMsgProcess;
		char *SipXmlAlarm=new char[MAXBUFSIZE];
		memset(SipXmlAlarm,0,MAXBUFSIZE);
		SipAlarm->SipSubscribeMsgCancel(&SipXmlAlarm,m_InfoServer,m_InfoClient,destXMLAlarmSet);
		//send message to client
		if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
		{		
			delete SipXmlAlarm;
			MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);		
			return;
		}
		UA_Msg uas_sendtemp;
		strcpy(uas_sendtemp.data,SipXmlAlarm);
		EnterCriticalSection(&g_uas);
		uas_sendqueue.push(uas_sendtemp);
		LeaveCriticalSection(&g_uas);
		delete SipXmlAlarm;
		//update log
		ShowTestData="SUBSCRIBE CANCEL  ----------->\r\n";
		ShowTestTitle="Alarm Subscribe Cancel Test";
		SAlarmCallID.nStatus = Alarm;

		Common::FLAG_NowCancleReserving = true;
	}
	else
	{
		AfxMessageBox("您还没有预定此报警事件...", MB_OK | MB_ICONERROR);
		return;
	}
}

void CAlarm::OnCbnSelchangeSelalarmtype()
{
	int index=m_AlarmTypeSel.GetCurSel();
	CString AlarmType=arrAlarmType[index];
	GetDlgItem(IDC_EDT_ALARM_TYPE)->SetWindowTextA(AlarmType);
}

BOOL CAlarm::OnInitDialog()
{
	CDialog::OnInitDialog();

	arrAlarmType.push_back("1");
	m_AlarmTypeSel.InsertString(0,"高温报警");

	arrAlarmType.push_back("2");
	m_AlarmTypeSel.InsertString(1,"低温报警");

	arrAlarmType.push_back("3");
	m_AlarmTypeSel.InsertString(2,"视频丢失报警");

	arrAlarmType.push_back("4");
	m_AlarmTypeSel.InsertString(3,"移动侦测报警");

	arrAlarmType.push_back("5");
	m_AlarmTypeSel.InsertString(4,"遮挡侦测报警");

	arrAlarmType.push_back("6");
	m_AlarmTypeSel.InsertString(5,"输入开关量报警");

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
