// CatalogQuery.cpp : 实现文件
//

#include "stdafx.h"
#include "UAS.h"
#include "CatalogQuery.h"
#include "afxdialogex.h"
#include "UASDlg.h"
#include "SipMsgProcess.h"
// CCatalogQuery 对话框

extern InfoServer m_InfoServer;
extern InfoClient m_InfoClient;
extern CString ShowTestTitle;
extern CString ShowTestData;
extern StatusCallID SCallId;
extern CRITICAL_SECTION g_uas;
extern queue<UA_Msg> uas_sendqueue;

IMPLEMENT_DYNAMIC(CCatalogQuery, CDialogEx)

CCatalogQuery::CCatalogQuery(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCatalogQuery::IDD, pParent)
{

}

CCatalogQuery::~CCatalogQuery()
{
}

void CCatalogQuery::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SUBNOTE, m_subNoteAddress);
}


BEGIN_MESSAGE_MAP(CCatalogQuery, CDialogEx)
	ON_BN_CLICKED(IDC_QUERY, &CCatalogQuery::OnBnClickedQuery)
//	ON_BN_CLICKED(IDC_DEVICEINFQUERY2, &CCatalogQuery::OnBnClickedDeviceinfquery2)
//ON_BN_CLICKED(IDC_DEVICEINFQUERY2, &CCatalogQuery::OnBnClickedDeviceinfquery2)
ON_BN_CLICKED(IDC_DEVICEINFQUERY2, &CCatalogQuery::OnBnClickedDeviceinotequery)
END_MESSAGE_MAP()


// CCatalogQuery 消息处理程序


void CCatalogQuery::OnBnClickedQuery()
{
	// TODO: 在此添加控件通知处理程序代码
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
	pWnd->catalogAddress=Address;
	CString strTemp;
	strTemp="<?xml version=\"1.0\"?>\r\n";
	strTemp+="<Action>\r\n";	
	strTemp+="<Query>\r\n";
	strTemp+="<Variable>ItemList</Variable>\r\n";
	strTemp+="<Privilege>"+UserCode+"</Privilege>\r\n";
	strTemp+="<Address>"+Address+"</Address>\r\n";
	strTemp+="<FromIndex>"+CString("1")+"</FromIndex>\r\n";
	strTemp+="<ToIndex>"+CString("200")+"</ToIndex>\r\n";
// 	strTemp+="<FileType>"+FileType+"</FileType>\r\n";
// 	strTemp+="<MaxFileNum>"+MaxFileNum+"</MaxFileNum>\r\n";
// 	strTemp+="<BeginTime>"+BeginTime+"</BeginTime>\r\n";
// 	strTemp+="<EndTime>"+EndTime+"</EndTime>\r\n";	
	strTemp+="</Query>\r\n";
	strTemp+="</Action>\r\n";
	char *xml=(LPSTR)(LPCTSTR)strTemp;
	char *buf=new char[MAXBUFSIZE];
	CSipMsgProcess *sipCatalogQuery=new CSipMsgProcess;
	sipCatalogQuery->CatalogQuerySipXmlMsg(&buf,m_InfoServer,Address,m_InfoClient,xml);	
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
	ShowTestTitle="Catalog Query Test";
	SCallId.nStatus=CatalogQuery;
}


//void CCatalogQuery::OnBnClickedDeviceinfquery2()
//{
//	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
//	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
//	//release the Video information memory
//	vector<InfoVideo>().swap(pWnd->m_VideoInfo);	
//	//m_HistoryVideoList.ResetContent();
//	CString UserCode;//权限功能码
//	CString Address;
//	CString FileType;
//	CString MaxFileNum;
//	CString BeginTime;
//	CString EndTime;
//	GetDlgItem(IDC_EDT_PRIVILEGE)->GetWindowText(UserCode);	
//	GetDlgItem(IDC_SUBNOTE)->GetWindowText(Address);
//	if (Address.Compare("")==0)
//	{
//		MessageBox("子节点地址编码为空！","UAS 提示",MB_OK|MB_ICONINFORMATION);
//		return;
//	}
//	//pWnd->catalogAddress=Address;
//	CString strTemp;
//	strTemp="<?xml version=\"1.0\"?>\r\n";
//	strTemp+="<Action>\r\n";	
//	strTemp+="<Query>\r\n";
//	strTemp+="<Variable>ItemList</Variable>\r\n";
//	strTemp+="<Privilege>"+UserCode+"</Privilege>\r\n";
//	strTemp+="<Address>"+Address+"</Address>\r\n";
//	strTemp+="<FromIndex>"+CString("1")+"</FromIndex>\r\n";
//	strTemp+="<ToIndex>"+CString("200")+"</ToIndex>\r\n";
//	strTemp+="</Query>\r\n";
//	strTemp+="</Action>\r\n";
//	char *xml=(LPSTR)(LPCTSTR)strTemp;
//	char *buf=new char[MAXBUFSIZE];
//	CSipMsgProcess *sipCatalogQuery=new CSipMsgProcess;
//	sipCatalogQuery->CatalogQuerySipXmlMsg(&buf,m_InfoServer,pWnd->catalogAddress/*Address*/,m_InfoClient,xml);	
//	//send message to client
//	if (m_InfoClient.Port=="" || m_InfoClient.IP=="")
//	{		
//		delete buf;		
//		MessageBox("没有注册的客户端用户","UAS 提示",MB_OK|MB_ICONINFORMATION);
//		return;
//	}
//	//pWnd->SendData(buf);		
//	UA_Msg uas_sendtemp;	
//	strcpy(uas_sendtemp.data,buf);	
//	EnterCriticalSection(&g_uas);
//	uas_sendqueue.push(uas_sendtemp);		
//	LeaveCriticalSection(&g_uas);	
//	//pWnd->ShowSendData(buf);
//	delete buf;
//	//update log
//	ShowTestData="DO --------->\r\n";	
//	ShowTestTitle="subCatalog Query Test";
//	SCallId.nStatus=CatalogQuery;
//}


//void CCatalogQuery::OnBnClickedDeviceinfquery2()
//{
//	// TODO: 在此添加控件通知处理程序代码
//}


void CCatalogQuery::OnBnClickedDeviceinotequery()
{
	HWND   hnd = ::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd = (CUASDlg*)CWnd::FromHandle(hnd);
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
	GetDlgItem(IDC_SUBNOTE)->GetWindowText(Address);
	if (Address.Compare("") == 0)
	{
		MessageBox("子节点地址编码为空！", "UAS 提示", MB_OK | MB_ICONINFORMATION);
		return;
	}
	//pWnd->catalogAddress=Address;
	CString strTemp;
	strTemp = "<?xml version=\"1.0\"?>\r\n";
	strTemp += "<Action>\r\n";
	strTemp += "<Query>\r\n";
	strTemp += "<Variable>ItemList</Variable>\r\n";
	strTemp += "<Privilege>" + Privilege + "</Privilege>\r\n";
	strTemp += "<Address>" + Address + "</Address>\r\n";
	strTemp += "<FromIndex>" + CString("1") + "</FromIndex>\r\n";
	strTemp += "<ToIndex>" + CString("200") + "</ToIndex>\r\n";
	strTemp += "</Query>\r\n";
	strTemp += "</Action>\r\n";
	char *xml = (LPSTR)(LPCTSTR)strTemp;
	char *buf = new char[MAXBUFSIZE];
	CSipMsgProcess *sipCatalogQuery = new CSipMsgProcess;
	sipCatalogQuery->CatalogQuerySipXmlMsg(&buf, m_InfoServer, pWnd->catalogAddress/*Address*/, m_InfoClient, xml);
	//send message to client
	if (m_InfoClient.Port == "" || m_InfoClient.IP == "")
	{
		delete buf;
		MessageBox("没有注册的客户端用户", "UAS 提示", MB_OK | MB_ICONINFORMATION);
		return;
	}
	//pWnd->SendData(buf);		
	UA_Msg uas_sendtemp;
	strcpy(uas_sendtemp.data, buf);
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);
	LeaveCriticalSection(&g_uas);
	//pWnd->ShowSendData(buf);
	delete buf;
	//update log
	ShowTestData = "DO --------->\r\n";
	ShowTestTitle = "subCatalog Query Test";
	SCallId.nStatus = CatalogQuery;
}
