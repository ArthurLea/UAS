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
//��Ա����
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

	//�ж��Ƿ��Ѿ�Ԥ���˴��¼�
	string nowOperatorEventMsg = StartAlarmPrivilege + EndAlarmPrivilege + AlarmMethod + DeviceID 
		+ StartTime + AlarmMethod;
	Common::nowOperatorEventMsg = nowOperatorEventMsg;//��һ�λ���
	vector<string>::iterator it = find(Common::curAlreadyReserveEvent.begin(),
		Common::curAlreadyReserveEvent.end(), nowOperatorEventMsg);
	if (it != Common::curAlreadyReserveEvent.end())//������������Ϣ����ʾ�Ѿ�Ԥ���˱����¼�
	{
		AfxMessageBox("�Ѿ�Ԥ���˴˱����¼�...", MB_OK | MB_ICONERROR);
		return;
	}
	else//�����ڣ�����û��Ԥ�������ı����¼�
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
			MessageBox("û��ע��Ŀͻ����û�", "UAS ��ʾ", MB_OK | MB_ICONINFORMATION);
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

		//Ԥ���ɹ����ڽ��д洢����SipParse�е�Alarm�н��д洢
		//Common::curAlreadyReserveEvent.push_back(nowReservingEventMsg);
		//GetDlgItem(IDC_BTN_ALARM_CANCEL)->EnableWindow(TRUE);//��ȡ������Ԥ����ť
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

	//�ж��Ƿ��Ѿ�Ԥ���˴��¼�
	string nowOperatorEventMsg = StartAlarmPrivilege + EndAlarmPrivilege + AlarmMethod + DeviceID
		+ StartTime + AlarmMethod;
	Common::nowOperatorEventMsg = nowOperatorEventMsg;//��һ�λ���
	vector<string>::iterator it = find(Common::curAlreadyReserveEvent.begin(),
		Common::curAlreadyReserveEvent.end(), nowOperatorEventMsg);
	if (it != Common::curAlreadyReserveEvent.end())//��ʾԤ���˴˱����¼�
	{
		//��Ҫ�ڳɹ�ȡ������ɾ��
		//ɾ�����Ԥ���¼�Msg�Ļ���
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
			MessageBox("û��ע��Ŀͻ����û�","UAS ��ʾ",MB_OK|MB_ICONINFORMATION);		
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
		AfxMessageBox("����û��Ԥ���˱����¼�...", MB_OK | MB_ICONERROR);
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
	m_AlarmTypeSel.InsertString(0,"���±���");

	arrAlarmType.push_back("2");
	m_AlarmTypeSel.InsertString(1,"���±���");

	arrAlarmType.push_back("3");
	m_AlarmTypeSel.InsertString(2,"��Ƶ��ʧ����");

	arrAlarmType.push_back("4");
	m_AlarmTypeSel.InsertString(3,"�ƶ���ⱨ��");

	arrAlarmType.push_back("5");
	m_AlarmTypeSel.InsertString(4,"�ڵ���ⱨ��");

	arrAlarmType.push_back("6");
	m_AlarmTypeSel.InsertString(5,"���뿪��������");

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}
