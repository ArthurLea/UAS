// CoderSet.cpp : implementation file
//

#include "stdafx.h"
#include "UAS.h"
#include "CoderSet.h"
#include "UASDlg.h"

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
extern CRITICAL_SECTION g_uas;
extern queue<UA_Msg> uas_sendqueue;

// CCoderSet dialog

IMPLEMENT_DYNAMIC(CCoderSet, CDialog)

CCoderSet::CCoderSet(CWnd* pParent /*=NULL*/)
	: CDialog(CCoderSet::IDD, pParent)
{

}

CCoderSet::~CCoderSet()
{
}

void CCoderSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCoderSet, CDialog)
	ON_BN_CLICKED(IDC_BTN_SET, &CCoderSet::OnBnClickedBtnSet)
END_MESSAGE_MAP()


// CCoderSet message handlers

void CCoderSet::OnBnClickedBtnSet()
{
	// TODO: Add your control notification handler code here
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	//get information and create XML message
	CString UserCode;
	CString Format;
	CString FrameRate;
	CString BitRate;
	CString Priority;
	CString GOP;
	CString address;
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	GetDlgItem(IDC_EDT_FORMAT)->GetWindowText(Format);
	GetDlgItem(IDC_EDT_FRAME)->GetWindowText(FrameRate);
	GetDlgItem(IDC_EDT_BITRATE)->GetWindowText(BitRate);
	GetDlgItem(IDC_EDT_PRIORITY)->GetWindowText(Priority);
	GetDlgItem(IDC_EDT_GOP)->GetWindowText(GOP);	
	GetDlgItem(IDC_EDT_ADDRESS)->GetWindowText(address);
	pWnd->encodeAddress=address;
	CString XmlCoderSet;
	XmlCoderSet="<?xml version=\"1.0\"?>\r\n";
	XmlCoderSet+="<Action>\r\n";
	XmlCoderSet+="<Notify>\r\n";
	XmlCoderSet+="<Variable>EncoderSet</Variable>\r\n";	
	XmlCoderSet+="<Privilege>"+UserCode+"</Privilege>\r\n";
	XmlCoderSet+="<Format>"+Format+"</Format>\r\n";
	XmlCoderSet+="<FrameRate>"+FrameRate+"</FrameRate>\r\n";
	XmlCoderSet+="<BitRate>"+BitRate+"</BitRate>\r\n";
	XmlCoderSet+="<Priority>"+Priority+"</Priority>\r\n";
	XmlCoderSet+="<GOP>"+GOP+"</GOP>\r\n";	
	XmlCoderSet+="</Notify>\r\n";
	XmlCoderSet+="</Action>\r\n";	
	char *destXMLCoderSet = (LPSTR)(LPCTSTR)XmlCoderSet;		
	CSipMsgProcess *SipEncoder=new CSipMsgProcess;	
	char *SipXmlEncoder=new char[MAXBUFSIZE];
	memset(SipXmlEncoder,0,MAXBUFSIZE);
	SipEncoder->SipEncoderSetMsg(&SipXmlEncoder,m_InfoServer,address,m_InfoClient,destXMLCoderSet);
	//send message to client
	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
	{		
		delete SipXmlEncoder;
		MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);
		return;
	}	
	//pWnd->SendData(SipXmlEncoder);
	UA_Msg uas_sendtemp;	
	strcpy(uas_sendtemp.data,SipXmlEncoder);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);
	//pWnd->ShowSendData(SipXmlEncoder);	
	delete SipXmlEncoder;
	//update log	
	ShowTestData=" DO  ---------->\r\n";
	ShowTestTitle="Encoder Set Test";
	SCallId.nStatus=EncoderSet;
}
