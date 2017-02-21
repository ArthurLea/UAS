// VideoQuery.cpp : implementation file
//

#include "stdafx.h"
#include "UAS.h"
#include "VideoQuery.h"
#include "UASDlg.h"
#include "SipMsgProcess.h"
extern InfoNotify NotifyInfo;
extern InfoServer m_InfoServer;
extern InfoClient m_InfoClient;
extern CString ShowTestTitle;
extern CString ShowTestData;
extern StatusCallID SCallId;
extern CRITICAL_SECTION g_uas;
extern queue<UA_Msg> uas_sendqueue;
extern struct hisQuery tHisVidQuery;

// CVideoQuery dialog

IMPLEMENT_DYNAMIC(CVideoQuery, CDialog)

CVideoQuery::CVideoQuery(CWnd* pParent /*=NULL*/)
	: CDialog(CVideoQuery::IDD, pParent)
{

}

CVideoQuery::~CVideoQuery()
{
}

void CVideoQuery::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_LIST, m_HistoryVideoList);
	DDX_Control(pDX, IDC_SELADRESS, m_selAddress);
}


BEGIN_MESSAGE_MAP(CVideoQuery, CDialog)
	ON_BN_CLICKED(IDC_BTN_QUERY, &CVideoQuery::OnBnClickedBtnQuery)
	ON_BN_CLICKED(IDC_BTN_GETURL, &CVideoQuery::OnBnClickedBtnGeturl)
	ON_CBN_SELCHANGE(IDC_COMBO_LIST, &CVideoQuery::OnCbnSelchangeComboList)
	ON_CBN_SELCHANGE(IDC_SELADRESS, &CVideoQuery::OnCbnSelchangeSeladress)
END_MESSAGE_MAP()


// CVideoQuery message handlers

void CVideoQuery::OnBnClickedBtnQuery()
{
	// TODO: Add your control notification handler code here
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	//release the Video information memory
	vector<InfoVideo>().swap(pWnd->m_VideoInfo);	
	m_HistoryVideoList.ResetContent();
	CString UserCode;
	CString CameraAddress;
	CString FileType;
	CString beginNum;
	CString endNum;
	CString BeginTime;
	CString EndTime;
	GetDlgItem(IDC_EDT_PRIVILEGE)->GetWindowText(UserCode);	
	GetDlgItem(IDC_EDT_FILETYPE)->GetWindowText(FileType);
	GetDlgItem(IDC_EDT_BEGINTIME)->GetWindowText(BeginTime);
	GetDlgItem(IDC_EDT_MAXFILENUM)->GetWindowText(beginNum);
	GetDlgItem(IDC_EDT_MAXFILENUM2)->GetWindowText(endNum);
	GetDlgItem(IDC_EDT_ENDTIME)->GetWindowText(EndTime);
	GetDlgItem(IDC_EDT_ADDRESS)->GetWindowText(CameraAddress);
	pWnd->videoAddress=CameraAddress;
	tHisVidQuery.FileType=FileType;
	tHisVidQuery.CameraAddress=CameraAddress;
	tHisVidQuery.UserCode=UserCode;
	tHisVidQuery.HistoryQueryNumbegin=atoi(beginNum.GetBuffer(beginNum.GetLength()));
	tHisVidQuery.HistoryQueryNumend=atoi(endNum.GetBuffer(endNum.GetLength()));
	CString strTemp;
	strTemp="<?xml version=\"1.0\"?>\r\n";
	strTemp+="<Action>\r\n";	
	strTemp+="<Query>\r\n";
	strTemp+="<Variable>FileList</Variable>\r\n";
	strTemp+="<Privilege>"+UserCode+"</Privilege>\r\n";
	//strTemp+="<CameraAddress>"+CameraAddress+"</CameraAddress>\r\n";
	strTemp+="<FileType>"+FileType+"</FileType>\r\n";
	strTemp+="<FromIndex>"+beginNum+"</FromIndex>\r\n";
	strTemp+="<ToIndex>"+endNum+"</ToIndex>\r\n";
	//strTemp+="<MaxFileNum>"+MaxFileNum+"</MaxFileNum>\r\n";
	strTemp+="<BeginTime>"+BeginTime+"</BeginTime>\r\n";
	strTemp+="<EndTime>"+EndTime+"</EndTime>\r\n";	
	strTemp+="</Query>\r\n";
	strTemp+="</Action>\r\n";
	char *xml=(LPSTR)(LPCTSTR)strTemp;
	char *buf=new char[MAXBUFSIZE];
	CSipMsgProcess *sipVideoQuery=new CSipMsgProcess;
	sipVideoQuery->VideoSipXmlMsg(&buf,m_InfoServer,CameraAddress,m_InfoClient,xml);	
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
	ShowTestTitle="History Video Query Test";
	SCallId.nStatus=HistoryQuery;
}

void CVideoQuery::OnBnClickedBtnGeturl()
{
	// TODO: Add your control notification handler code here
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	int nCurVideo=m_HistoryVideoList.GetCurSel();
	CString UserCode;
	GetDlgItem(IDC_EDT_PRIVILEGE)->GetWindowText(UserCode);
	CString BeginTime;
	BeginTime.Format("%s",pWnd->m_VideoInfo[nCurVideo].BeginTime);
	CString EndTime;
	EndTime.Format("%s",pWnd->m_VideoInfo[nCurVideo].EndTime);
	CString FileName;
	FileName.Format("%s",pWnd->m_VideoInfo[nCurVideo].Name);
	CString strTemp;
	strTemp="<?xml version=\"1.0\"?>\r\n";
	strTemp+="<Action>\r\n";	
	//strTemp+="<Query>\r\n";
	strTemp+="<Variable>VODByRTSP</Variable>\r\n";
	strTemp+="<Privilege>"+UserCode+"</Privilege>\r\n";
	strTemp+="<FileType>2</FileType>\r\n";	
	strTemp+="<Name>"+FileName+"</Name>\r\n";
	strTemp+="<BeginTime>"+BeginTime+"</BeginTime>\r\n";
	strTemp+="<EndTime>"+EndTime+"</EndTime>\r\n";
	strTemp+="<MaxBitrate>100</MaxBitrate>\r\n";
	//strTemp+="</Query>\r\n";
	strTemp+="</Action>\r\n";
	char *xml=(LPSTR)(LPCTSTR)strTemp;
	char *buf=new char[MAXBUFSIZE];
	CSipMsgProcess *sipVideoUrl=new CSipMsgProcess;
	sipVideoUrl->GetUrlDOSipXmlMsg(&buf,m_InfoServer,m_InfoClient,xml);	
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
	ShowTestData="DO ----------->\r\n";
	ShowTestTitle="Get History Video URL Test";
	SCallId.nStatus=HistoryPlay;
}

void CVideoQuery::OnCbnSelchangeComboList()
{
	// TODO: Add your control notification handler code here
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	int i=m_HistoryVideoList.GetCurSel();	
	GetDlgItem(IDC_EDT_BEGIN)->SetWindowText(pWnd->m_VideoInfo[i].BeginTime);
	GetDlgItem(IDC_EDT_END)->SetWindowText(pWnd->m_VideoInfo[i].EndTime);
	GetDlgItem(IDC_EDT_FILESIZE)->SetWindowText(pWnd->m_VideoInfo[i].FileSize);
}


void CVideoQuery::OnCbnSelchangeSeladress()
{
	// TODO: 在此添加控件通知处理程序代码
	int index=m_selAddress.GetCurSel();
	CString Address=NotifyInfo.Devices[index].Address;
	GetDlgItem(IDC_EDT_ADDRESS)->SetWindowTextA(Address);
}
