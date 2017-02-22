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
extern BOOL bNetSet;
extern StatusCallID SCallId;
extern StatusCallID SAlarmCallID;
extern StatusCallID sInviteCallID;
extern CallID InviteKeepAliveID;	
extern char *ByeVia;
extern char *ByeFrom;
extern char *ByeTo;	
extern BOOL bACK;
extern BOOL bBYE;	
extern BOOL bShowRealTime;
extern BOOL bNodeParent;
extern char strBye[MAXBUFSIZE];
//�ж��Ƿ���������Ϣ
extern char *contact;
extern BOOL bSipRegister;
extern BOOL bNodeType;
extern int nOverTime;
extern int nCurrentTime;
extern int nTimeCount;
extern time_t oldTime,currentTime;
extern BOOL bOverTime;	
extern BOOL bFlag;
extern BOOL bVerify;	
extern BOOL bUDPSipConnect;
extern char strPlayUrl[250];
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
	ON_BN_CLICKED(IDC_BTN_TIMESET, &CAlarm::OnBnClickedBtnTimeset)
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
	CString UserCode;
	CString Level;
	CString AlarmType;
	CString Address;
	CString AcceptIP;
	CString AcceptPort;
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	GetDlgItem(IDC_EDT_LEVEL)->GetWindowText(Level);
	GetDlgItem(IDC_EDT_ALARM_TYPE)->GetWindowText(AlarmType);
	GetDlgItem(IDC_EDT_ADDRESS)->GetWindowText(Address);
	GetDlgItem(IDC_EDT_IP)->GetWindowText(AcceptIP);
	GetDlgItem(IDC_EDT_PORT)->GetWindowText(AcceptPort);

	//�ж��Ƿ��Ѿ�Ԥ���˴��¼�
	string nowReservingEventMsg = UserCode + Level + AlarmType + Address + AcceptIP + AcceptPort;
	Common::nowReservingEventMsg = nowReservingEventMsg;//��һ�λ���
	vector<string>::iterator it = find(Common::curAlreadyReserveEvent.begin(),
		Common::curAlreadyReserveEvent.end(), nowReservingEventMsg);
	if (it != Common::curAlreadyReserveEvent.end())//������������Ϣ����ʾ�Ѿ�Ԥ���˱����¼�
	{
		AfxMessageBox("�Ѿ�Ԥ���˴˱����¼�...", MB_OK | MB_ICONERROR);
		return;
	}
	else//�����ڣ�����û��Ԥ�������ı����¼�
	{
		CString XmlAlarmSet;
		XmlAlarmSet = "<?xml version=\"1.0\"?>\r\n";
		XmlAlarmSet += "<Action>\r\n";
		XmlAlarmSet += "<Notify>\r\n";
		XmlAlarmSet += "<Variable>AlarmSubscribe</Variable>\r\n";
		XmlAlarmSet += "<Privilege>" + UserCode + "</Privilege>\r\n";
		XmlAlarmSet += "<Address>" + Address + "</Address>\r\n";
		XmlAlarmSet += "<Level>" + Level + "</Level>\r\n";
		XmlAlarmSet += "<AlarmType>" + AlarmType + "</AlarmType>\r\n";
		XmlAlarmSet += "<AcceptIp>" + AcceptIP + "</AcceptIp>\r\n";
		XmlAlarmSet += "<AcceptPort>" + AcceptPort + "</AcceptPort>\r\n";
		XmlAlarmSet += "</Notify>\r\n";
		XmlAlarmSet += "</Action>\r\n";
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

		GetDlgItem(IDC_BTN_ALARM_CANCEL)->EnableWindow(TRUE);//��ȡ������Ԥ����ť

	}
}

void CAlarm::OnBnClickedBtnTimeset()
{
	time_t CurrentTime;	
	CurrentTime=time(NULL);
	struct tm *pts;
	pts=localtime(&CurrentTime);
	CString strTime;
	strTime.Format("%d-%d-%dT%d:%d:%dZ",pts->tm_year+1900,pts->tm_mon+1,pts->tm_mday,pts->tm_hour,pts->tm_min,pts->tm_sec);
	CString XmlTimeSet;
	XmlTimeSet="<?xml version=\"1.0\"?>\r\n";
	XmlTimeSet+="<Action>\r\n";
	XmlTimeSet+="<Notify>\r\n";
	XmlTimeSet+="<Variable>TimeSet</Variable>\r\n";	
	XmlTimeSet+="<Time>"+strTime+"</Time>\r\n";
	XmlTimeSet+="<Privilege>192016809088</Privilege>\r\n";
	XmlTimeSet+="</Notify>\r\n";
	XmlTimeSet+="</Action>\r\n";	
	char *destXML = (LPSTR)(LPCTSTR)XmlTimeSet;		
	CSipMsgProcess *Sip=new CSipMsgProcess;	
	char *SipXml=new char[MAXBUFSIZE];
	memset(SipXml,0,MAXBUFSIZE);
	Sip->SipXmlMsg(&SipXml,m_InfoServer,m_InfoClient,destXML);
	//send message to client
	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
	{		
		delete SipXml;
		MessageBox("û��ע��Ŀͻ����û�","UAS ��ʾ",MB_OK|MB_ICONINFORMATION);		
		return;
	}	
	//pWnd->SendData(SipXmlAlarm);
	UA_Msg uas_sendtemp;
	strcpy(uas_sendtemp.data,SipXml);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);
	//pWnd->ShowSendData(SipXmlAlarm);	
	delete SipXml;
	//update log	
	ShowTestData="DO  ----------->\r\n";	
	ShowTestTitle="Time Set Test";
	SCallId.nStatus=TimeSet;
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
	CString UserCode;
	CString Level;
	CString AlarmType;
	CString Address;
	CString AcceptIP;
	CString AcceptPort;
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	GetDlgItem(IDC_EDT_LEVEL)->GetWindowText(Level);
	GetDlgItem(IDC_EDT_ALARM_TYPE)->GetWindowText(AlarmType);
	GetDlgItem(IDC_EDT_ADDRESS)->GetWindowText(Address);
	GetDlgItem(IDC_EDT_IP)->GetWindowText(AcceptIP);
	GetDlgItem(IDC_EDT_PORT)->GetWindowText(AcceptPort);
	//�ж��Ƿ��Ѿ�Ԥ���˴��¼�
	string nowReservingEventMsg = UserCode + Level + AlarmType + Address + AcceptIP + AcceptPort;
	Common::nowReservingEventMsg = nowReservingEventMsg;//��һ�λ���
	vector<string>::iterator it = find(Common::curAlreadyReserveEvent.begin(),
		Common::curAlreadyReserveEvent.end(), nowReservingEventMsg);
	if (it != Common::curAlreadyReserveEvent.end())//������������Ϣ����ʾ�Ѿ�Ԥ���˱����¼�
	{
		//ɾ�����Ԥ���¼�Msg�Ļ���
		Common::curAlreadyReserveEvent.erase(it);

		CString XmlAlarmSet;
		XmlAlarmSet="<?xml version=\"1.0\"?>\r\n";
		XmlAlarmSet+="<Action>\r\n";
		XmlAlarmSet+="<Notify>\r\n";
		XmlAlarmSet+="<Variable>AlarmSubscribe</Variable>\r\n";
		XmlAlarmSet+="<Privilege>"+UserCode+"</Privilege>\r\n";
		XmlAlarmSet+="<Address>"+Address+"</Address>\r\n";
		XmlAlarmSet+="<Level>"+Level+"</Level>\r\n";
		XmlAlarmSet+="<AlarmType>"+AlarmType+"</AlarmType>\r\n";	
		XmlAlarmSet+="<AcceptIp>"+AcceptIP+"</AcceptIp>\r\n";
		XmlAlarmSet+="<AcceptPort>"+AcceptPort+"</AcceptPort>\r\n";	
		XmlAlarmSet+="</Notify>\r\n";
		XmlAlarmSet+="</Action>\r\n";
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
