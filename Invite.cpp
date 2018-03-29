// Invite.cpp : implementation file
//

#include "stdafx.h"
#include "UAS.h"
#include "Invite.h"
#include "UASDlg.h"
#include "SipMsgProcess.h"
extern SHELLEXECUTEINFO rtpsend;
extern SHELLEXECUTEINFO vlc;
//extern queue<UA_Msg> uas_recvqueue;
extern queue<UA_Msg> uas_sendqueue;
//extern UA_Msg uas_curqueue;
extern UA_Msg uas_curSendMsg;
//extern HANDLE hMutex_uas;
extern CRITICAL_SECTION g_uas;

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
extern BOOL bCANCEL;
extern BOOL bShowRealTime;
extern BOOL bNodeParent;
extern char strBye[MAXBUFSIZE];
extern char strCancel[MAXBUFSIZE];
//判断是否是心跳信息
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
//成员变量
extern InfoServer m_InfoServer;
extern InfoClient m_InfoClient;
extern CString ShowTestTitle;
extern CString ShowTestData;
// CInvite dialog

IMPLEMENT_DYNAMIC(CInvite, CDialog)

CInvite::CInvite(CWnd* pParent /*=NULL*/)
	: CDialog(CInvite::IDD, pParent)
	, m_selName(0)
{

}

CInvite::~CInvite()
{
}

void CInvite::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SELADRESS, m_selAddress);
}


BEGIN_MESSAGE_MAP(CInvite, CDialog)
	ON_BN_CLICKED(IDC_BTN_BYE, &CInvite::OnBnClickedBtnBye)
	ON_BN_CLICKED(IDC_BTN_PLAY, &CInvite::OnBnClickedBtnPlay)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CInvite::OnBnClickedBtnCancel)
	ON_CBN_SELCHANGE(IDC_SELADRESS, &CInvite::OnCbnSelchangeSeladress)
	ON_BN_CLICKED(IDC_BTN_TEST, &CInvite::OnBnClickedBtnTest)
END_MESSAGE_MAP()


// CInvite message handlers

void CInvite::OnBnClickedBtnTest()
{
	// get information and create XML message
	//每次按下这个按钮，应该重置所有会话状态
	int index = m_selAddress.GetCurSel();

	CString UserCode;
	CString Format;
	CString Video;
	CString Audio;
	CString MaxBitrate;
	CString TransmitMode;
	CString Address;
	CString Multicast;
	CString ReceiveSocket;
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	GetDlgItem(IDC_EDT_FORMAT)->GetWindowText(Format);
	GetDlgItem(IDC_EDT_VIDEO)->GetWindowText(Video);
	GetDlgItem(IDC_EDT_AUDIO)->GetWindowText(Audio);
	GetDlgItem(IDC_EDT_MAXBIT)->GetWindowText(MaxBitrate);
	//GetDlgItem(IDC_EDT_TRANSMODE)->GetWindowText(TransmitMode);
	GetDlgItem(IDC_EDT_PROTOCOL)->GetWindowText(Address);

	GetDlgItem(IDC_EDT_MULTICAST)->GetWindowText(Multicast);
	GetDlgItem(IDC_EDT_SOCKET)->GetWindowText(ReceiveSocket);
	CString XmlInvite;
	XmlInvite = "<?xml version=\"1.0\"?>\r\n";
	XmlInvite += "<Action>\r\n";
	XmlInvite += "<CmdType>RealMedia</CmdType>\r\n";
	XmlInvite += "<Privilege>" + UserCode + "</Privilege>\r\n";
	XmlInvite += "<Format>" + Format + "</Format>\r\n";
	XmlInvite += "<Video>" + Video + "</Video>\r\n";
	//XmlInvite += "<Stream>RTP</Stream>\r\n";
	XmlInvite += "<Audio>" + Audio + "</Audio>\r\n";
	XmlInvite += "<MaxBitrate>" + MaxBitrate + "</MaxBitrate>\r\n";
	XmlInvite+="<Multicast>"+Multicast+"</Multicast>\r\n";
	//XmlInvite+="<TransmitMode>"+TransmitMode+"</TransmitMode>\r\n";	
	XmlInvite += "<ReceiveSocket>" + ReceiveSocket + "</ReceiveSocket>\r\n";
	XmlInvite += "</Action>\r\n";
	HWND   hnd = ::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd = (CUASDlg*)CWnd::FromHandle(hnd);
	char *destXMLInvite = (LPSTR)(LPCTSTR)XmlInvite;
	CSipMsgProcess *SipInvite = new CSipMsgProcess;
	pWnd->inviteAddress = Address;
	char *SipXmlInvite = new char[MAXBUFSIZE];
	memset(SipXmlInvite, 0, MAXBUFSIZE);
	SipInvite->SipInviteMsg(&SipXmlInvite, m_InfoServer, m_InfoClient, destXMLInvite, Address);
	//send invite message to client
	if (m_InfoClient.Port == "" || m_InfoClient.IP == "")
	{
		delete SipXmlInvite;
		SipXmlInvite = NULL;
		MessageBox("没有注册的客户端用户", "UAS 提示", MB_OK | MB_ICONINFORMATION);
		return;
	}
	UA_Msg uas_sendtemp;
	strcpy(uas_sendtemp.data, SipXmlInvite);
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);
	LeaveCriticalSection(&g_uas);
	delete SipXmlInvite;
	SipXmlInvite = NULL;
	//update log	
	ShowTestData = "INVITE   ----------->\r\n";
	ShowTestTitle = "Invite Test";
	sInviteCallID.nStatus = Invite;
	bACK = FALSE;
	bBYE = FALSE;
	bShowRealTime = FALSE;
	pWnd->bSelectKeepLive = FALSE;
}

void CInvite::OnBnClickedBtnCancel()
{
	OnBnClickedBtnTest();//发送INVITE
	CString Address;
	GetDlgItem(IDC_EDT_PROTOCOL)->GetWindowText(Address);
	HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	CSipMsgProcess *SipCancel=new CSipMsgProcess;
	char *SipXmlCancel=new char[MAXBUFSIZE];
	memset(SipXmlCancel,0,MAXBUFSIZE);
	SipCancel->SipCancelMsg(&SipXmlCancel,m_InfoServer,m_InfoClient,Address);

	UA_Msg uas_sendtemp;
	strcpy(uas_sendtemp.data,SipXmlCancel);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);	
	//update log	
	ShowTestData ="CANCEL   ----------->\r\n";		
	ShowTestTitle="Invite Test";					
	//bBYE=TRUE;
	bCANCEL = TRUE;
	pWnd->bSelectKeepLive = TRUE;
}

void CInvite::OnBnClickedBtnBye()
{
	// TODO: Add your control notification handler code here
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(TRUE);
	UA_Msg uas_sendtemp;
	strcpy(uas_sendtemp.data,strBye);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);	
	//update log	
	ShowTestData ="BYE   ----------->\r\n";		
	ShowTestTitle="Invite Test";					
	bBYE = TRUE;
	pWnd->bSelectKeepLive = TRUE;
	TerminateProcess(vlc.hProcess, 0);
	TerminateProcess(rtpsend.hProcess, 0);
}

void CInvite::OnBnClickedBtnPlay()
{
}

BOOL CInvite::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CInvite::OnCbnSelchangeSeladress()
{
	// TODO: 在此添加控件通知处理程序代码
	int index=m_selAddress.GetCurSel();
	CString Address=NotifyInfo.Devices[index].Address;
	GetDlgItem(IDC_EDT_PROTOCOL)->SetWindowTextA(Address);
}




