// PTZ.cpp : implementation file
//

#include "stdafx.h"
#include "UAS.h"
#include "PTZ.h"
#include "UASDlg.h"

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
extern ptzQuery tPTZQuery;

// CPTZ dialog

IMPLEMENT_DYNAMIC(CPTZ, CDialog)

CPTZ::CPTZ(CWnd* pParent /*=NULL*/)
	: CDialog(CPTZ::IDD, pParent)
{

}

CPTZ::~CPTZ()
{
}

void CPTZ::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SELADRESS, m_selAddress);
}


BEGIN_MESSAGE_MAP(CPTZ, CDialog)
	ON_BN_CLICKED(IDC_BTN_TEST, &CPTZ::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_PRE, &CPTZ::OnBnClickedBtnPre)
	ON_CBN_SELCHANGE(IDC_SELADRESS, &CPTZ::OnCbnSelchangeSeladress)
	ON_BN_CLICKED(IDC_PTZUP, &CPTZ::OnBnClickedPtzup)
	ON_BN_CLICKED(IDC_PTZLEFT, &CPTZ::OnBnClickedPtzleft)
	ON_BN_CLICKED(IDC_PTZRIGHT, &CPTZ::OnBnClickedPtzright)
	ON_BN_CLICKED(IDC_PZTDOWN, &CPTZ::OnBnClickedPztdown)
	ON_BN_CLICKED(IDC_PTZUP2, &CPTZ::OnBnClickedPtzup2)
	ON_BN_CLICKED(IDC_PTZUP3, &CPTZ::OnBnClickedPtzup3)
	ON_BN_CLICKED(IDC_PTZUP4, &CPTZ::OnBnClickedPtzup4)
	ON_BN_CLICKED(IDC_PTZUP5, &CPTZ::OnBnClickedPtzup5)
	ON_BN_CLICKED(IDC_PTZUP6, &CPTZ::OnBnClickedPtzup6)
	ON_BN_CLICKED(IDC_PTZUP7, &CPTZ::OnBnClickedPtzup7)
END_MESSAGE_MAP()


// CPTZ message handlers

void CPTZ::OnBnClickedBtnTest()
{
	// TODO: Add your control notification handler code here		
	//get information and create XML message
	CString UserCode;
	CString PTZCommand;
	CString Address;	
	CString Protocol;	
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	GetDlgItem(IDC_EDT_PTZ)->GetWindowText(PTZCommand);
	GetDlgItem(IDC_EDT_ADD)->GetWindowText(Address);
	//GetDlgItem(IDC_EDT_PTL)->GetWindowText(Protocol);
	CString XmlPTZ;
	XmlPTZ="<?xml version=\"1.0\"?>\r\n";
	XmlPTZ+="<Action>\r\n";	
	XmlPTZ+="<Control>\r\n";	
	XmlPTZ+="<Variable>PTZCommand</Variable>\r\n";	
	XmlPTZ+="<Privilege>"+UserCode+"</Privilege>\r\n";
	XmlPTZ+="<Command>"+PTZCommand+"</Command>\r\n";
	//XmlPTZ+="<Address>"+Address+"</Address>\r\n";
	XmlPTZ+="</Control>\r\n";
	XmlPTZ+="</Action>\r\n";	
	//WaitForSingleObject(hMutex_uas,INFINITE);
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	char *destXMLPTZ= (LPSTR)(LPCTSTR)XmlPTZ;		
	CSipMsgProcess *SipPTZ=new CSipMsgProcess;	
	char *SipXmlPTZ=new char[MAXBUFSIZE];
	memset(SipXmlPTZ,0,MAXBUFSIZE);	
	//SipPTZ->SipXmlMsg(&SipXmlPTZ,m_InfoServer,m_InfoClient,destXMLPTZ);
	SipPTZ->SipPtzMsg(&SipXmlPTZ,m_InfoServer,pWnd->inviteAddress,m_InfoClient,destXMLPTZ);
	//send message to client
	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
	{		
		delete SipXmlPTZ;
		MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);		
		return;
	}	
	//pWnd->SendData(SipXmlPTZ);
	UA_Msg uas_sendtemp;	
	strcpy(uas_sendtemp.data,SipXmlPTZ);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);	
	//pWnd->ShowSendData(SipXmlPTZ);	
	delete SipXmlPTZ;
	SipXmlPTZ=NULL;
	//update log
	ShowTestData=" DO   ------------->\r\n";	
	ShowTestTitle="PTZ Test";
	SCallId.nStatus=PTZ;
	//ReleaseMutex(hMutex_uas);
}

void CPTZ::OnBnClickedBtnPre()
{
	// TODO: Add your control notification handler code here
	CString beginNum;
	CString endNum;
	CString UserCode;
	CString Address;
	GetDlgItem(IDC_EDT_BEGINNUM)->GetWindowText(beginNum);
	//GetDlgItem(IDC_EDT_ENDNUM)->GetWindowText(endNum);
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	GetDlgItem(IDC_EDT_ADD)->GetWindowText(Address);
	//tPTZQuery.ptzQueryNumbegin=beginNum;
	//tPTZQuery.ptzQueryNumend=endNum;

	tPTZQuery.ptzQueryReceivePresetNum = beginNum;
	tPTZQuery.UserCode=UserCode;
	tPTZQuery.Address=Address;
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	CString XmlPreBitQuery;
	XmlPreBitQuery="<?xml version=\"1.0\"?>\r\n";
	XmlPreBitQuery+="<Action>\r\n";	
	XmlPreBitQuery+="<Query>\r\n";	
	XmlPreBitQuery+="<Variable>PresetList</Variable>\r\n";
	XmlPreBitQuery+="<Privilege>"+UserCode+"</Privilege>\r\n";
	//XmlPreBitQuery+="<FromIndex>"+beginNum+"</FromIndex>\r\n";
	//XmlPreBitQuery+="<ToIndex>"+endNum+"</ToIndex>\r\n";
	XmlPreBitQuery+="<ReceivePresetNum>"+ beginNum+"</ReceivePresetNum>\r\n";
	XmlPreBitQuery+="</Query>\r\n";
	XmlPreBitQuery+="</Action>\r\n";
	char *destXML= (LPSTR)(LPCTSTR)XmlPreBitQuery;		
	CSipMsgProcess *Sip=new CSipMsgProcess;	
	char *SipXml=new char[MAXBUFSIZE];
	memset(SipXml,0,MAXBUFSIZE);	
	Sip->PreSetBitSipXmlMsg(&SipXml,m_InfoServer,m_InfoClient,Address/*pWnd->inviteAddress*/,destXML);
	//send message to client
	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
	{		
		delete SipXml;
		MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);		
		return;
	}
	UA_Msg uas_sendtemp;	
	strcpy(uas_sendtemp.data,SipXml);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);		
	delete SipXml;
	SipXml=NULL;
	//update log
	ShowTestData=" DO   ------------->\r\n";	
	ShowTestTitle="预置位查询";
	SCallId.nStatus=PreBitSet;

}


void CPTZ::OnCbnSelchangeSeladress()
{
	int index=m_selAddress.GetCurSel();
	CString Address=NotifyInfo.Devices[index].Address;
	GetDlgItem(IDC_EDT_ADD)->SetWindowTextA(Address);
}


void CPTZ::OnBnClickedPtzup()
{
	// TODO: 在此添加控件通知处理程序代码
	static BOOL bAlter=TRUE;
	CString UserCode;
	CString PTZCommand;
	if(bAlter)
	{
		GetDlgItem(IDC_PTZUP)->SetWindowText(_T("停"));
		PTZCommand="0x0401";
		bAlter=FALSE;
	}
	else
	{
		GetDlgItem(IDC_PTZUP)->SetWindowText(_T("上"));
		PTZCommand="0x0402";
		bAlter=TRUE;
	}
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	CString XmlPTZ;
	XmlPTZ="<?xml version=\"1.0\"?>\r\n";
	XmlPTZ+="<Action>\r\n";	
	XmlPTZ+="<Control>\r\n";	
	XmlPTZ+="<Variable>PTZCommand</Variable>\r\n";	
	XmlPTZ+="<Privilege>"+UserCode+"</Privilege>\r\n";
	XmlPTZ+="<Command>"+PTZCommand+"</Command>\r\n";
	XmlPTZ+="</Control>\r\n";
	XmlPTZ+="</Action>\r\n";
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	char *destXMLPTZ= (LPSTR)(LPCTSTR)XmlPTZ;		
	CSipMsgProcess *SipPTZ=new CSipMsgProcess;	
	char *SipXmlPTZ=new char[MAXBUFSIZE];
	memset(SipXmlPTZ,0,MAXBUFSIZE);
	SipPTZ->SipPtzMsg(&SipXmlPTZ,m_InfoServer,pWnd->inviteAddress,m_InfoClient,destXMLPTZ);
	//send message to client
	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
	{		
		delete SipXmlPTZ;
		MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);		
		return;
	}	;
	UA_Msg uas_sendtemp;	
	strcpy(uas_sendtemp.data,SipXmlPTZ);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);	

	delete SipXmlPTZ;
	SipXmlPTZ=NULL;
	//update log
	ShowTestData=" DO   ------------->\r\n";	
	ShowTestTitle="PTZ Test Up";
	SCallId.nStatus=PTZ;
}


void CPTZ::OnBnClickedPtzleft()
{
	// TODO: 在此添加控件通知处理程序代码
	static BOOL bAlter=TRUE;
	CString UserCode;
	CString PTZCommand;
	if(bAlter)
	{
		GetDlgItem(IDC_PTZLEFT)->SetWindowText(_T("停"));
		PTZCommand="0x0503";
		bAlter=FALSE;
	}
	else
	{
		GetDlgItem(IDC_PTZLEFT)->SetWindowText(_T("左"));
		PTZCommand="0x0504";
		bAlter=TRUE;
	}
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	CString XmlPTZ;
	XmlPTZ="<?xml version=\"1.0\"?>\r\n";
	XmlPTZ+="<Action>\r\n";	
	XmlPTZ+="<Control>\r\n";	
	XmlPTZ+="<Variable>PTZCommand</Variable>\r\n";	
	XmlPTZ+="<Privilege>"+UserCode+"</Privilege>\r\n";
	XmlPTZ+="<Command>"+PTZCommand+"</Command>\r\n";
	XmlPTZ+="</Control>\r\n";
	XmlPTZ+="</Action>\r\n";
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	char *destXMLPTZ= (LPSTR)(LPCTSTR)XmlPTZ;		
	CSipMsgProcess *SipPTZ=new CSipMsgProcess;	
	char *SipXmlPTZ=new char[MAXBUFSIZE];
	memset(SipXmlPTZ,0,MAXBUFSIZE);
	SipPTZ->SipPtzMsg(&SipXmlPTZ,m_InfoServer,pWnd->inviteAddress,m_InfoClient,destXMLPTZ);
	//send message to client
	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
	{		
		delete SipXmlPTZ;
		MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);		
		return;
	}	;
	UA_Msg uas_sendtemp;	
	strcpy(uas_sendtemp.data,SipXmlPTZ);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);	

	delete SipXmlPTZ;
	SipXmlPTZ=NULL;
	//update log
	ShowTestData=" DO   ------------->\r\n";	
	ShowTestTitle="PTZ Test Left";
	SCallId.nStatus=PTZ;
}


void CPTZ::OnBnClickedPtzright()
{
	// TODO: 在此添加控件通知处理程序代码
	static BOOL bAlter=TRUE;
	CString UserCode;
	CString PTZCommand;
	if(bAlter)
	{
		GetDlgItem(IDC_PTZRIGHT)->SetWindowText(_T("停"));
		PTZCommand="0x0501";
		bAlter=FALSE;
	}
	else
	{
		GetDlgItem(IDC_PTZRIGHT)->SetWindowText(_T("右"));
		PTZCommand="0x0502";
		bAlter=TRUE;
	}
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	CString XmlPTZ;
	XmlPTZ="<?xml version=\"1.0\"?>\r\n";
	XmlPTZ+="<Action>\r\n";	
	XmlPTZ+="<Control>\r\n";	
	XmlPTZ+="<Variable>PTZCommand</Variable>\r\n";	
	XmlPTZ+="<Privilege>"+UserCode+"</Privilege>\r\n";
	XmlPTZ+="<Command>"+PTZCommand+"</Command>\r\n";
	XmlPTZ+="</Control>\r\n";
	XmlPTZ+="</Action>\r\n";
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	char *destXMLPTZ= (LPSTR)(LPCTSTR)XmlPTZ;		
	CSipMsgProcess *SipPTZ=new CSipMsgProcess;	
	char *SipXmlPTZ=new char[MAXBUFSIZE];
	memset(SipXmlPTZ,0,MAXBUFSIZE);
	SipPTZ->SipPtzMsg(&SipXmlPTZ,m_InfoServer,pWnd->inviteAddress,m_InfoClient,destXMLPTZ);
	//send message to client
	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
	{		
		delete SipXmlPTZ;
		MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);		
		return;
	}	;
	UA_Msg uas_sendtemp;	
	strcpy(uas_sendtemp.data,SipXmlPTZ);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);	

	delete SipXmlPTZ;
	SipXmlPTZ=NULL;
	//update log
	ShowTestData=" DO   ------------->\r\n";	
	ShowTestTitle="PTZ Test Right";
	SCallId.nStatus=PTZ;
}


void CPTZ::OnBnClickedPztdown()
{
	// TODO: 在此添加控件通知处理程序代码
	static BOOL bAlter=TRUE;
	CString UserCode;
	CString PTZCommand;
	if(bAlter)
	{
		GetDlgItem(IDC_PZTDOWN)->SetWindowText(_T("停"));
		PTZCommand="0x0403";
		bAlter=FALSE;
	}
	else
	{
		GetDlgItem(IDC_PZTDOWN)->SetWindowText(_T("下"));
		PTZCommand="0x0404";
		bAlter=TRUE;
	}
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	CString XmlPTZ;
	XmlPTZ="<?xml version=\"1.0\"?>\r\n";
	XmlPTZ+="<Action>\r\n";	
	XmlPTZ+="<Control>\r\n";	
	XmlPTZ+="<Variable>PTZCommand</Variable>\r\n";	
	XmlPTZ+="<Privilege>"+UserCode+"</Privilege>\r\n";
	XmlPTZ+="<Command>"+PTZCommand+"</Command>\r\n";
	XmlPTZ+="</Control>\r\n";
	XmlPTZ+="</Action>\r\n";
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	char *destXMLPTZ= (LPSTR)(LPCTSTR)XmlPTZ;		
	CSipMsgProcess *SipPTZ=new CSipMsgProcess;	
	char *SipXmlPTZ=new char[MAXBUFSIZE];
	memset(SipXmlPTZ,0,MAXBUFSIZE);
	SipPTZ->SipPtzMsg(&SipXmlPTZ,m_InfoServer,pWnd->inviteAddress,m_InfoClient,destXMLPTZ);
	//send message to client
	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
	{		
		delete SipXmlPTZ;
		MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);		
		return;
	}	;
	UA_Msg uas_sendtemp;	
	strcpy(uas_sendtemp.data,SipXmlPTZ);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);	

	delete SipXmlPTZ;
	SipXmlPTZ=NULL;
	//update log
	ShowTestData=" DO   ------------->\r\n";	
	ShowTestTitle="PTZ Test Down";
	SCallId.nStatus=PTZ;
}


void CPTZ::OnBnClickedPtzup2()
{
	// TODO: 在此添加控件通知处理程序代码
	static BOOL bAlter=TRUE;
	CString UserCode;
	CString PTZCommand;
	if(bAlter)
	{
		GetDlgItem(IDC_PTZUP2)->SetWindowText(_T("停"));
		PTZCommand="0x0101";
		bAlter=FALSE;
	}
	else
	{
		GetDlgItem(IDC_PTZUP2)->SetWindowText(_T("减小光圈"));
		PTZCommand="0x0102";
		bAlter=TRUE;
	}
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	CString XmlPTZ;
	XmlPTZ="<?xml version=\"1.0\"?>\r\n";
	XmlPTZ+="<Action>\r\n";	
	XmlPTZ+="<Control>\r\n";	
	XmlPTZ+="<Variable>PTZCommand</Variable>\r\n";	
	XmlPTZ+="<Privilege>"+UserCode+"</Privilege>\r\n";
	XmlPTZ+="<Command>"+PTZCommand+"</Command>\r\n";
	XmlPTZ+="</Control>\r\n";
	XmlPTZ+="</Action>\r\n";
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	char *destXMLPTZ= (LPSTR)(LPCTSTR)XmlPTZ;		
	CSipMsgProcess *SipPTZ=new CSipMsgProcess;	
	char *SipXmlPTZ=new char[MAXBUFSIZE];
	memset(SipXmlPTZ,0,MAXBUFSIZE);
	SipPTZ->SipPtzMsg(&SipXmlPTZ,m_InfoServer,pWnd->inviteAddress,m_InfoClient,destXMLPTZ);
	//send message to client
	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
	{		
		delete SipXmlPTZ;
		MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);		
		return;
	}	;
	UA_Msg uas_sendtemp;	
	strcpy(uas_sendtemp.data,SipXmlPTZ);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);	

	delete SipXmlPTZ;
	SipXmlPTZ=NULL;
	//update log
	ShowTestData=" DO   ------------->\r\n";	
	ShowTestTitle="PTZ Test 减小光圈";
	SCallId.nStatus=PTZ;
}


void CPTZ::OnBnClickedPtzup3()
{
	// TODO: 在此添加控件通知处理程序代码
	static BOOL bAlter=TRUE;
	CString UserCode;
	CString PTZCommand;
	if(bAlter)
	{
		GetDlgItem(IDC_PTZUP3)->SetWindowText(_T("停"));
		PTZCommand="0x0104";
		bAlter=FALSE;
	}
	else
	{
		GetDlgItem(IDC_PTZUP3)->SetWindowText(_T("增大光圈"));
		PTZCommand="0x0103";
		bAlter=TRUE;
	}
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	CString XmlPTZ;
	XmlPTZ="<?xml version=\"1.0\"?>\r\n";
	XmlPTZ+="<Action>\r\n";	
	XmlPTZ+="<Control>\r\n";	
	XmlPTZ+="<Variable>PTZCommand</Variable>\r\n";	
	XmlPTZ+="<Privilege>"+UserCode+"</Privilege>\r\n";
	XmlPTZ+="<Command>"+PTZCommand+"</Command>\r\n";
	XmlPTZ+="</Control>\r\n";
	XmlPTZ+="</Action>\r\n";
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	char *destXMLPTZ= (LPSTR)(LPCTSTR)XmlPTZ;		
	CSipMsgProcess *SipPTZ=new CSipMsgProcess;	
	char *SipXmlPTZ=new char[MAXBUFSIZE];
	memset(SipXmlPTZ,0,MAXBUFSIZE);
	SipPTZ->SipPtzMsg(&SipXmlPTZ,m_InfoServer,pWnd->inviteAddress,m_InfoClient,destXMLPTZ);
	//send message to client
	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
	{		
		delete SipXmlPTZ;
		MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);		
		return;
	}	;
	UA_Msg uas_sendtemp;	
	strcpy(uas_sendtemp.data,SipXmlPTZ);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);	

	delete SipXmlPTZ;
	SipXmlPTZ=NULL;
	//update log
	ShowTestData=" DO   ------------->\r\n";	
	ShowTestTitle="PTZ Test 增大光圈";
	SCallId.nStatus=PTZ;
}


void CPTZ::OnBnClickedPtzup4()
{
	// TODO: 在此添加控件通知处理程序代码
	static BOOL bAlter=TRUE;
	CString UserCode;
	CString PTZCommand;
	if(bAlter)
	{
		GetDlgItem(IDC_PTZUP4)->SetWindowText(_T("停"));
		PTZCommand="0x0201";
		bAlter=FALSE;
	}
	else
	{
		GetDlgItem(IDC_PTZUP4)->SetWindowText(_T("近聚焦"));
		PTZCommand="0x0202";
		bAlter=TRUE;
	}
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	CString XmlPTZ;
	XmlPTZ="<?xml version=\"1.0\"?>\r\n";
	XmlPTZ+="<Action>\r\n";	
	XmlPTZ+="<Control>\r\n";	
	XmlPTZ+="<Variable>PTZCommand</Variable>\r\n";	
	XmlPTZ+="<Privilege>"+UserCode+"</Privilege>\r\n";
	XmlPTZ+="<Command>"+PTZCommand+"</Command>\r\n";
	XmlPTZ+="</Control>\r\n";
	XmlPTZ+="</Action>\r\n";
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	char *destXMLPTZ= (LPSTR)(LPCTSTR)XmlPTZ;		
	CSipMsgProcess *SipPTZ=new CSipMsgProcess;	
	char *SipXmlPTZ=new char[MAXBUFSIZE];
	memset(SipXmlPTZ,0,MAXBUFSIZE);
	SipPTZ->SipPtzMsg(&SipXmlPTZ,m_InfoServer,pWnd->inviteAddress,m_InfoClient,destXMLPTZ);
	//send message to client
	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
	{		
		delete SipXmlPTZ;
		MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);		
		return;
	}	;
	UA_Msg uas_sendtemp;	
	strcpy(uas_sendtemp.data,SipXmlPTZ);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);	

	delete SipXmlPTZ;
	SipXmlPTZ=NULL;
	//update log
	ShowTestData=" DO   ------------->\r\n";	
	ShowTestTitle="PTZ Test 近聚焦";
	SCallId.nStatus=PTZ;
}


void CPTZ::OnBnClickedPtzup5()
{
	// TODO: 在此添加控件通知处理程序代码
	static BOOL bAlter=TRUE;
	CString UserCode;
	CString PTZCommand;
	if(bAlter)
	{
		GetDlgItem(IDC_PTZUP5)->SetWindowText(_T("停"));
		PTZCommand="0x0203";
		bAlter=FALSE;
	}
	else
	{
		GetDlgItem(IDC_PTZUP5)->SetWindowText(_T("远聚焦"));
		PTZCommand="0x0204";
		bAlter=TRUE;
	}
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	CString XmlPTZ;
	XmlPTZ="<?xml version=\"1.0\"?>\r\n";
	XmlPTZ+="<Action>\r\n";	
	XmlPTZ+="<Control>\r\n";	
	XmlPTZ+="<Variable>PTZCommand</Variable>\r\n";	
	XmlPTZ+="<Privilege>"+UserCode+"</Privilege>\r\n";
	XmlPTZ+="<Command>"+PTZCommand+"</Command>\r\n";
	XmlPTZ+="</Control>\r\n";
	XmlPTZ+="</Action>\r\n";
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	char *destXMLPTZ= (LPSTR)(LPCTSTR)XmlPTZ;		
	CSipMsgProcess *SipPTZ=new CSipMsgProcess;	
	char *SipXmlPTZ=new char[MAXBUFSIZE];
	memset(SipXmlPTZ,0,MAXBUFSIZE);
	SipPTZ->SipPtzMsg(&SipXmlPTZ,m_InfoServer,pWnd->inviteAddress,m_InfoClient,destXMLPTZ);
	//send message to client
	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
	{		
		delete SipXmlPTZ;
		MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);		
		return;
	}	;
	UA_Msg uas_sendtemp;	
	strcpy(uas_sendtemp.data,SipXmlPTZ);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);	

	delete SipXmlPTZ;
	SipXmlPTZ=NULL;
	//update log
	ShowTestData=" DO   ------------->\r\n";	
	ShowTestTitle="PTZ Test 远聚焦";
	SCallId.nStatus=PTZ;
}


void CPTZ::OnBnClickedPtzup6()
{
	// TODO: 在此添加控件通知处理程序代码
	static BOOL bAlter=TRUE;
	CString UserCode;
	CString PTZCommand;
	if(bAlter)
	{
		GetDlgItem(IDC_PTZUP6)->SetWindowText(_T("停"));
		PTZCommand="0x0303";
		bAlter=FALSE;
	}
	else
	{
		GetDlgItem(IDC_PTZUP6)->SetWindowText(_T("放大"));
		PTZCommand="0x0304";
		bAlter=TRUE;
	}
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	CString XmlPTZ;
	XmlPTZ="<?xml version=\"1.0\"?>\r\n";
	XmlPTZ+="<Action>\r\n";	
	XmlPTZ+="<Control>\r\n";	
	XmlPTZ+="<Variable>PTZCommand</Variable>\r\n";	
	XmlPTZ+="<Privilege>"+UserCode+"</Privilege>\r\n";
	XmlPTZ+="<Command>"+PTZCommand+"</Command>\r\n";
	XmlPTZ+="</Control>\r\n";
	XmlPTZ+="</Action>\r\n";
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	char *destXMLPTZ= (LPSTR)(LPCTSTR)XmlPTZ;		
	CSipMsgProcess *SipPTZ=new CSipMsgProcess;	
	char *SipXmlPTZ=new char[MAXBUFSIZE];
	memset(SipXmlPTZ,0,MAXBUFSIZE);
	SipPTZ->SipPtzMsg(&SipXmlPTZ,m_InfoServer,pWnd->inviteAddress,m_InfoClient,destXMLPTZ);
	//send message to client
	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
	{		
		delete SipXmlPTZ;
		MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);		
		return;
	}	;
	UA_Msg uas_sendtemp;	
	strcpy(uas_sendtemp.data,SipXmlPTZ);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);	

	delete SipXmlPTZ;
	SipXmlPTZ=NULL;
	//update log
	ShowTestData=" DO   ------------->\r\n";	
	ShowTestTitle="PTZ Test 放大";
	SCallId.nStatus=PTZ;
}


void CPTZ::OnBnClickedPtzup7()
{
	// TODO: 在此添加控件通知处理程序代码
	static BOOL bAlter=TRUE;
	CString UserCode;
	CString PTZCommand;
	if(bAlter)
	{
		GetDlgItem(IDC_PTZUP7)->SetWindowText(_T("停"));
		PTZCommand="0x0301";
		bAlter=FALSE;
	}
	else
	{
		GetDlgItem(IDC_PTZUP7)->SetWindowText(_T("缩小"));
		PTZCommand="0x0302";
		bAlter=TRUE;
	}
	GetDlgItem(IDC_EDT_USERCODE)->GetWindowText(UserCode);
	CString XmlPTZ;
	XmlPTZ="<?xml version=\"1.0\"?>\r\n";
	XmlPTZ+="<Action>\r\n";	
	XmlPTZ+="<Control>\r\n";	
	XmlPTZ+="<Variable>PTZCommand</Variable>\r\n";	
	XmlPTZ+="<Privilege>"+UserCode+"</Privilege>\r\n";
	XmlPTZ+="<Command>"+PTZCommand+"</Command>\r\n";
	XmlPTZ+="</Control>\r\n";
	XmlPTZ+="</Action>\r\n";
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	char *destXMLPTZ= (LPSTR)(LPCTSTR)XmlPTZ;		
	CSipMsgProcess *SipPTZ=new CSipMsgProcess;	
	char *SipXmlPTZ=new char[MAXBUFSIZE];
	memset(SipXmlPTZ,0,MAXBUFSIZE);
	SipPTZ->SipPtzMsg(&SipXmlPTZ,m_InfoServer,pWnd->inviteAddress,m_InfoClient,destXMLPTZ);
	//send message to client
	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
	{		
		delete SipXmlPTZ;
		MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);		
		return;
	}	;
	UA_Msg uas_sendtemp;	
	strcpy(uas_sendtemp.data,SipXmlPTZ);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);	

	delete SipXmlPTZ;
	SipXmlPTZ=NULL;
	//update log
	ShowTestData=" DO   ------------->\r\n";	
	ShowTestTitle="PTZ Test 缩小";
	SCallId.nStatus=PTZ;
}
