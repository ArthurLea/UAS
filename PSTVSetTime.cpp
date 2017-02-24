// PSTVSetTime.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "UAS.h"
#include "UASDlg.h"
#include "PSTVSetTime.h"
#include "afxdialogex.h"
#include "SipMsgProcess.h"
#include <time.h>
#include <algorithm>
// CPSTVSetTime �Ի���

//��Ա����
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


// CPSTVSetTime ��Ϣ�������


BOOL CPSTVSetTime::OnInitDialog()
{
	CDialog::OnInitDialog(); 
	GetDlgItem(IDC_EDIT_PRIVILEGE)->SetWindowTextA("192016809088");
	return 0;
}

void CPSTVSetTime::OnBnClickedButtonNtptime()
{
	//�����Ǵ�NTP(����ʱ��Э��)��ͨ����������ȡ����ʱ���ʵ��
}


void CPSTVSetTime::OnBnClickedButtonCurtime()
{
	time_t CurrentTime;
	CurrentTime = time(NULL);
	struct tm *pts;
	pts = localtime(&CurrentTime);//�����ȡʱ�������ͨ����ȡ����ʱ��
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
	XmlTimeSet+="<Privilege>" + privilege + "</Privilege>\r\n";//�û���Ȩ����
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
		MessageBox("û��ע��Ŀͻ����û�", "UAS ��ʾ", MB_OK | MB_ICONINFORMATION);
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
