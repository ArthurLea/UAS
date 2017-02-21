// DlgEqualLevel.cpp : 实现文件
//

#include "stdafx.h"
#include "UAS.h"
#include "DlgEqualLevel.h"
#include "afxdialogex.h"
#include "UASDlg.h"
extern InfoServer m_InfoServerEqual;
extern InfoServer m_InfoServer;

extern SOCKET m_socket_Equal;
CRITICAL_SECTION g_uac;
HANDLE h_UAC_Recv;
HANDLE h_UAC_Dispatch;
HANDLE h_UAC_Send;
queue<UA_Msg> uac_recvqueue;
queue<UA_Msg> uac_sendqueue;
UA_Msg uac_curqueue;
UA_Msg uac_curSendMsg;

// CDlgEqualLevel 对话框

IMPLEMENT_DYNAMIC(CDlgEqualLevel, CDialogEx)

CDlgEqualLevel::CDlgEqualLevel(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgEqualLevel::IDD, pParent)
{

}

CDlgEqualLevel::~CDlgEqualLevel()
{
}

void CDlgEqualLevel::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgEqualLevel, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgEqualLevel::OnBnClickedOk)
	ON_BN_CLICKED(IDC_REGSITERE, &CDlgEqualLevel::OnBnClickedRegsitere)
	ON_BN_CLICKED(IDC_NOTIFYE, &CDlgEqualLevel::OnBnClickedNotifye)
END_MESSAGE_MAP()


// CDlgEqualLevel 消息处理程序


void CDlgEqualLevel::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	GetDlgItem(IDC_IPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDT_PORTE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDT_ADDRESSE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDT_NAMEE)->EnableWindow(FALSE);

	GetDlgItem(IDC_IPE)->GetWindowText(m_InfoServerEqual.IP);
	GetDlgItem(IDC_EDT_PORTE)->GetWindowText(m_InfoServerEqual.Port);
	GetDlgItem(IDC_EDT_ADDRESSE)->SetWindowText(m_InfoServerEqual.UserAddress);
	GetDlgItem(IDC_EDT_NAMEE)->SetWindowText(m_InfoServerEqual.UserName);
	if (m_InfoServerEqual.IP=="" || m_InfoServerEqual.Port=="" ||m_InfoServerEqual.UserAddress=="" ||m_InfoServerEqual.UserName=="")
	{
		AfxMessageBox("请检查网络配置是否为空!");
		return;
	}
	/*CDialogEx::OnOK();*/
}


void CDlgEqualLevel::OnBnClickedRegsitere()
{
// 	TODO: 在此添加控件通知处理程序代码
// 		HWND   hnd=::FindWindow(NULL, _T("UAS"));
// 		CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
// 		int n=atoi(m_InfoServerEqual.Port);
// 		int nflag=-1;
// 		nflag=pWnd->InitSocketEqual(n);
// 		if ( nflag!=0 )
// 		{
// 			return;
// 		}
// 		RECVPARAM *pRecvParam=new RECVPARAM;
// 		pRecvParam->sock=m_socket_Equal;
// 		pRecvParam->hwnd=m_hWnd;
// 		InitializeCriticalSection(&g_uac);
// 		h_UAC_Recv=CreateThread(NULL,0,RecvMsg,(LPVOID)pRecvParam,0,NULL);
// 		h_UAC_Dispatch=CreateThread(NULL,0,DispatchRecvMsg,NULL,0,NULL);
// 		h_UAC_Send=CreateThread(NULL,0,SendMsg,NULL,0,NULL);		
// 		//CloseHandle(hThread);
// 		pWnd->ShowSendData("\t----UDP communication is listening----\r\n");
// 		//open TCP socket
// 		int nTCP_Port=atoi(TCP_Port);
// 		if ( !m_TCPSocket.Create(nTCP_Port) )
// 			return;
// 		if( !m_TCPSocket.Listen())
// 			return;	
// 		ShowSendData("*************** TCP communication is listening ****************\r\n");
// 		//SIP Register
// 		char *data=new char[MAXBUFSIZE];
// 		memset(data,0,MAXBUFSIZE);
// 		CSipMsgProcess *sip;
// 		sip=new CSipMsgProcess;
// 		sip->SipRegisterCreate(&data,m_InfoServerEqual,m_InfoServer);
// 		//sip->SipRegisterWithAuthCreate(&data,m_InfoServer,m_InfoClient);
// 		//SendData(data);		
// 		UA_Msg uac_sendtemp;
// 		strcpy(uac_sendtemp.data,data);
// 		EnterCriticalSection(&g_uac);
// 		uac_sendqueue.push(uac_sendtemp);
// 		LeaveCriticalSection(&g_uac);
// 		//ShowSendData(data);
// 		delete data;		
// 		//ShowTestLogTitle="Register Test";
// 		//update log	
// 		//ShowTestLogData+="REGISTER ------->  \r\n";		
// 		//开启SIP按钮不可用
// 		EnableWindow(IDC_REGSITERE,FALSE);
// 		//m_NetSet.m_kAlterBtn.EnableWindow(FALSE);
// 		EnableWindow(IDC_NOTIFYE,TRUE);
// 		SetTimer(3,5000,NULL);
}


void CDlgEqualLevel::OnBnClickedNotifye()
{
	// TODO: 在此添加控件通知处理程序代码
}
