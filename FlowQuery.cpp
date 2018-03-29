// FlowQuery.cpp : 实现文件
//

#include "stdafx.h"
#include "UAS.h"
#include "FlowQuery.h"
#include "afxdialogex.h"
#include "UASDlg.h"
#include "SipMsgProcess.h"

// CFlowQuery 对话框
extern InfoNotify NotifyInfo;
extern InfoServer m_InfoServer;
extern InfoClient m_InfoClient;
extern CString ShowTestTitle;
extern CString ShowTestData;
extern StatusCallID SCallId;
extern CRITICAL_SECTION g_uas;
extern queue<UA_Msg> uas_sendqueue;

IMPLEMENT_DYNAMIC(CFlowQuery, CDialogEx)

CFlowQuery::CFlowQuery(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFlowQuery::IDD, pParent)
{

}

CFlowQuery::~CFlowQuery()
{
}

void CFlowQuery::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SELADRESS, m_selAddress);
	DDX_Control(pDX, IDC_EDT_ADDRESS, m_CatQueAddress);
	DDX_Control(pDX, IDC_EDIT_FLOWQUERY, m_FlowQueryInfo);
}


BEGIN_MESSAGE_MAP(CFlowQuery, CDialogEx)
	ON_BN_CLICKED(IDC_FLOWQUERY, &CFlowQuery::OnBnClickedFlowquery)
	ON_CBN_SELCHANGE(IDC_SELADRESS, &CFlowQuery::OnCbnSelchangeSeladress)
END_MESSAGE_MAP()


// CFlowQuery 消息处理程序


void CFlowQuery::OnBnClickedFlowquery()
{
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	//release the Video information memory
	vector<InfoVideo>().swap(pWnd->m_VideoInfo);	
	//m_HistoryVideoList.ResetContent();
	CString UserCode;//权限功能码
	CString Address;
	CString FileType;
	CString MaxFileNum;
	CString BeginTime;
	CString EndTime;
	GetDlgItem(IDC_EDT_PRIVILEGE)->GetWindowText(UserCode);	
	GetDlgItem(IDC_EDT_ADDRESS)->GetWindowText(Address);
	pWnd->flowAddress=Address;
	CString strTemp;
	strTemp="<?xml version=\"1.0\"?>\r\n";
	strTemp+="<Action>\r\n";	
	strTemp+="<Query>\r\n";
	strTemp+="<CmdType>BandWidth</CmdType>\r\n";
	strTemp+="<Privilege>"+UserCode+"</Privilege>\r\n";
    // 	strTemp+="<Address>"+Address+"</Address>\r\n";
    // 	strTemp+="<FromIndex>"+CString("1")+"</FromIndex>\r\n";
    // 	strTemp+="<ToIndex>"+CString("200")+"</ToIndex>\r\n";
	// 	strTemp+="<FileType>"+FileType+"</FileType>\r\n";
	// 	strTemp+="<MaxFileNum>"+MaxFileNum+"</MaxFileNum>\r\n";
	// 	strTemp+="<BeginTime>"+BeginTime+"</BeginTime>\r\n";
	// 	strTemp+="<EndTime>"+EndTime+"</EndTime>\r\n";	
	strTemp+="</Query>\r\n";
	strTemp+="</Action>\r\n";
	char *xml=(LPSTR)(LPCTSTR)strTemp;
	char *buf=new char[MAXBUFSIZE];
	CSipMsgProcess *sipFlowQuery=new CSipMsgProcess;
	sipFlowQuery->FlowQuerySipXmlMsg(&buf,m_InfoServer,Address,m_InfoClient,xml);	
	//send message to client
	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
	{		
		delete buf;		
		MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);
		return;
	}
	//pWnd->SendData(buf);		
	UA_Msg uas_sendtemp;	
	strcpy(uas_sendtemp.data,buf);	
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);		
	LeaveCriticalSection(&g_uas);	
	//pWnd->ShowSendData(buf);
	delete buf;
	//update log
	ShowTestData="DO --------->\r\n";	
	ShowTestTitle="Flow Query Test";
	SCallId.nStatus = FlowQuery;
}


void CFlowQuery::OnCbnSelchangeSeladress()
{
	int index=m_selAddress.GetCurSel();
	CString Address=NotifyInfo.Devices[index].Address;
	GetDlgItem(IDC_EDT_ADDRESS)->SetWindowTextA(Address);
}
