// NetSet.cpp : implementation file
//

#include "stdafx.h"
#include "UAS.h"
#include "NetSet.h"
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

// CNetSet dialog

IMPLEMENT_DYNAMIC(CNetSet, CDialog)

CNetSet::CNetSet(CWnd* pParent /*=NULL*/)
	: CDialog(CNetSet::IDD, pParent)
{

}

CNetSet::~CNetSet()
{
}

void CNetSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_ALTER, m_kAlterBtn);
}


BEGIN_MESSAGE_MAP(CNetSet, CDialog)
	ON_BN_CLICKED(IDC_BTN_ALTER, &CNetSet::OnBnClickedBtnAlter)
END_MESSAGE_MAP()


// CNetSet message handlers

void CNetSet::OnBnClickedBtnAlter()
{
	// TODO: Add your control notification handler code here
	HWND   hnd=::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	static BOOL bAlter=TRUE;	
	if(bAlter)
	{
		m_kAlterBtn.SetWindowText(_T("确定"));
		GetDlgItem(IDC_EDT_PORT)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDT_ADDRESS)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDT_NAME)->EnableWindow(TRUE);
		bAlter=FALSE;
		bNetSet=FALSE;
	}
	else
	{		
		//更新网络配置属性
		CString LocalHostName;
		pWnd->GetLocalHostName(LocalHostName);
		pWnd->GetLocalIp(LocalHostName,m_InfoServer.IP);
		GetDlgItem(IDC_IP)->SetWindowText(m_InfoServer.IP);
		GetDlgItem(IDC_EDT_PORT)->GetWindowText(m_InfoServer.Port);
		GetDlgItem(IDC_EDT_ADDRESS)->GetWindowText(m_InfoServer.UserAddress);
		GetDlgItem(IDC_EDT_NAME)->GetWindowText(m_InfoServer.UserName);
		pWnd->GetDlgItem(IDC_STR_LOCALIP)->SetWindowText(m_InfoServer.IP);
		pWnd->GetDlgItem(IDC_STR_LOCAL_PORT)->SetWindowText(m_InfoServer.Port);
		pWnd->GetDlgItem(IDC_STR_LOCAL_ADD)->SetWindowText(m_InfoServer.UserAddress);
		pWnd->GetDlgItem(IDC_STR_LOCAL_NAME)->SetWindowText(m_InfoServer.UserName);	
		if (m_InfoServer.IP=="" || m_InfoServer.Port=="" ||m_InfoServer.UserAddress=="" ||m_InfoServer.UserName=="")
		{
			AfxMessageBox("请检查网络配置是否为空!");
			return;
		}
		//初始化网络配置文件
		FILE *NetFile=NULL;		
		NetFile=fopen("UASNetLog.txt","w");		
		fprintf(NetFile,m_InfoServer.Port);
		fprintf(NetFile,"\n");
		fprintf(NetFile,m_InfoServer.UserName);
		fprintf(NetFile,"\n");
		fprintf(NetFile,m_InfoServer.UserAddress);		
		if (NetFile)
		{
			fclose(NetFile);
		}
		bNetSet=TRUE;
		m_kAlterBtn.SetWindowText(_T("修改"));
		GetDlgItem(IDC_EDT_PORT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_ADDRESS)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_NAME)->EnableWindow(FALSE);	
		bAlter=TRUE;
	}
}
