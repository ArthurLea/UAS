// DeviceInfQuery.cpp : 实现文件
//

#include "stdafx.h"
#include "UAS.h"
#include "DeviceInfQuery.h"
#include "afxdialogex.h"
#include "UASDlg.h"
#include "SipMsgProcess.h"

// CDeviceInfQuery 对话框
extern InfoNotify NotifyInfo;
extern InfoServer m_InfoServer;
extern InfoClient m_InfoClient;
extern CString ShowTestTitle;
extern CString ShowTestData;
extern StatusCallID SCallId;
extern CRITICAL_SECTION g_uas;
extern queue<UA_Msg> uas_sendqueue;

IMPLEMENT_DYNAMIC(CDeviceInfQuery, CDialogEx)

CDeviceInfQuery::CDeviceInfQuery(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDeviceInfQuery::IDD, pParent)
{

}

CDeviceInfQuery::~CDeviceInfQuery()
{
}

void CDeviceInfQuery::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SELADRESS, m_selAddress);
}


BEGIN_MESSAGE_MAP(CDeviceInfQuery, CDialogEx)
	ON_BN_CLICKED(IDC_DEVICEINFQUERY, &CDeviceInfQuery::OnBnClickedDeviceinfquery)
	ON_CBN_SELCHANGE(IDC_SELADRESS, &CDeviceInfQuery::OnCbnSelchangeSeladress)
END_MESSAGE_MAP()

// CDeviceInfQuery 消息处理程序

void CDeviceInfQuery::OnBnClickedDeviceinfquery()
{
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	//release the Video information memory
	vector<InfoVideo>().swap(pWnd->m_VideoInfo);	
	//m_HistoryVideoList.ResetContent();
	CString Privilege;//权限功能码
	CString Address;
	CString FileType;
	CString MaxFileNum;
	CString BeginTime;
	CString EndTime;
	GetDlgItem(IDC_EDT_PRIVILEGE)->GetWindowText(Privilege);
	GetDlgItem(IDC_EDT_ADDRESS)->GetWindowText(Address);
	pWnd->deviceInfAddress=Address;
	CString strTemp;
	strTemp="<?xml version=\"1.0\"?>\r\n";
	strTemp+="<Action>\r\n";	
	strTemp+="<Query>\r\n";
	strTemp+="<Variable>DeviceInfo</Variable>\r\n";
	strTemp+="<Privilege>"+ Privilege +"</Privilege>\r\n";
	strTemp+="</Query>\r\n";
	strTemp+="</Action>\r\n";
	char *xml=(LPSTR)(LPCTSTR)strTemp;
	char *buf=new char[MAXBUFSIZE];
	CSipMsgProcess *sipDeviceInfQuery=new CSipMsgProcess;
	sipDeviceInfQuery->DeviceInfQuerySipXmlMsg(&buf,m_InfoServer,Address,m_InfoClient,xml);	
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
	ShowTestTitle="Device Inf Query Test";
	SCallId.nStatus=DeviceInfQuery;
}

void CDeviceInfQuery::OnCbnSelchangeSeladress()
{
	// TODO: 在此添加控件通知处理程序代码
	int index=m_selAddress.GetCurSel();
	CString Address=NotifyInfo.Devices[index].Address;
	GetDlgItem(IDC_EDT_ADDRESS)->SetWindowTextA(Address);
}
