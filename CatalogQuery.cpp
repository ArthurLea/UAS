// CatalogQuery.cpp : 实现文件
//

#include "stdafx.h"
#include "UAS.h"
#include "CatalogQuery.h"
#include "afxdialogex.h"
#include "UASDlg.h"
#include "SipMsgProcess.h"
// CCatalogQuery 对话框

extern InfoNotify NotifyInfo;
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
	DDX_Control(pDX, IDC_SUBNOTE, m_selAddress);
}


BEGIN_MESSAGE_MAP(CCatalogQuery, CDialogEx)
	ON_BN_CLICKED(IDC_DEVICEINFQUERY2, &CCatalogQuery::OnBnClickedDeviceinotequery)
	ON_BN_CLICKED(IDC_QUERY, &CCatalogQuery::OnBnClickedQuery)
	ON_CBN_SELCHANGE(IDC_SUBNOTE, &CCatalogQuery::OnCbnSelchangeSubnote)
END_MESSAGE_MAP()


// CCatalogQuery 消息处理程序
void CCatalogQuery::OnBnClickedQuery()
{
	HWND   hnd = ::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd = (CUASDlg*)CWnd::FromHandle(hnd);
	CString SN;
	CString DeviceID;
	GetDlgItem(IDC_EDT_PRIVILEGE)->GetWindowText(SN);
	GetDlgItem(IDC_EDT_ADDRESS)->GetWindowText(DeviceID);
	pWnd->catalogAddress = DeviceID;
	CString strTemp;
	strTemp = "<?xml version=\"1.0\"?>\r\n";
	strTemp += "<Query>\r\n";
	strTemp += "<CmdType>Catalog</CmdType>\r\n";
	strTemp += "<SN>" + SN + "</SN>\r\n";
	strTemp += "<DeviceID>" + DeviceID + "</DeviceID>\r\n";
	strTemp += "</Query>\r\n";
	char *xml = (LPSTR)(LPCTSTR)strTemp;
	char *buf = new char[MAXBUFSIZE];
	CSipMsgProcess *sipCatalogQuery = new CSipMsgProcess;
	sipCatalogQuery->CatalogQuerySipXmlMsg(&buf, m_InfoServer, DeviceID, m_InfoClient, xml);
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
	ShowTestData = "Message --------->\r\n";
	ShowTestTitle = "Catalog Query Test";
	SCallId.nStatus = CatalogQuery;
}

BOOL CCatalogQuery::OnInitDialog()
{
	CDialog::OnInitDialog();
	GetDlgItem(IDC_EDT_ADDRESS)->SetWindowTextA(m_InfoClient.UserAddress);
	GetDlgItem(IDC_EDT_PRIVILEGE)->SetWindowTextA("17430"); 
	GetDlgItem(IDC_EDT_DEVICE_SN)->SetWindowTextA("248"); 

	return TRUE;
}

void CCatalogQuery::OnBnClickedDeviceinotequery()
{
	HWND   hnd = ::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd = (CUASDlg*)CWnd::FromHandle(hnd);
	CString SN;
	CString DeviceID;
	GetDlgItem(IDC_EDT_DEVICE_SN)->GetWindowText(SN);
	GetDlgItem(IDC_EDIT_CHILDID)->GetWindowText(DeviceID);
	pWnd->catalogAddress = DeviceID;
	CString strTemp;
	strTemp = "<?xml version=\"1.0\"?>\r\n";
	strTemp += "<Query>\r\n";
	strTemp += "<CmdType>DeviceStatus</CmdType>\r\n";
	strTemp += "<SN>" + SN + "</SN>\r\n";
	strTemp += "<DeviceID>" + DeviceID + "</DeviceID>\r\n";
	strTemp += "</Query>\r\n";
	char *xml = (LPSTR)(LPCTSTR)strTemp;
	char *buf = new char[MAXBUFSIZE];
	CSipMsgProcess *sipDeviceStatusQuery = new CSipMsgProcess;
	sipDeviceStatusQuery->DeviceStatusQueryXmlMsg(&buf, m_InfoServer, DeviceID, m_InfoClient, xml);
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



void CCatalogQuery::OnCbnSelchangeSubnote()
{
	int index = m_selAddress.GetCurSel();
	CString Address = NotifyInfo.Devices[index].Address;
	GetDlgItem(IDC_EDIT_CHILDID)->SetWindowTextA(Address); 
}
