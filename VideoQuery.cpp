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
	DDX_Control(pDX, IDC_EDT_BEGIN, m_BeginTime);
	DDX_Control(pDX, IDC_EDT_END, m_EndTime);
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
	CString SN;
	CString DeviceID;
	CString StartTime;
	CString EndTime;
	CString Secrecy;
	CString Type;
	CString RecorderID;
	GetDlgItem(IDC_EDT_PRIVILEGE)->GetWindowText(SN);	
	GetDlgItem(IDC_EDT_FILETYPE)->GetWindowText(Type);
	GetDlgItem(IDC_EDT_ADDRESS)->GetWindowText(DeviceID);
	GetDlgItem(IDC_EDT_BEGINTIME)->GetWindowText(StartTime);
	GetDlgItem(IDC_EDT_ENDTIME)->GetWindowText(EndTime);
	GetDlgItem(IDC_EDT_SECRECY)->GetWindowText(Secrecy);
	GetDlgItem(IDC_EDT_MAXFILENUM2)->GetWindowText(RecorderID);
	pWnd->videoAddress= DeviceID;
	tHisVidQuery.FileType= Type;
	tHisVidQuery.CameraAddress= DeviceID;
	tHisVidQuery.UserCode= SN;
	//tHisVidQuery.HistoryQueryNumbegin=atoi(beginNum.GetBuffer(beginNum.GetLength()));
	//tHisVidQuery.HistoryQueryNumend=atoi(endNum.GetBuffer(endNum.GetLength()));
	CString strTemp;
	strTemp ="<?xml version=\"1.0\"?>\r\n";
	strTemp +="<Query>\r\n";
	strTemp +="<CmdType>RecordInfo</CmdType>\r\n";
	strTemp += "<SN>" + SN + "</SN>\r\n";
	strTemp +="<DeviceID>"+ DeviceID +"</DeviceID>\r\n";
	strTemp += "<StartTime>" + StartTime + "</StartTime>\r\n";
	strTemp +="<EndTime>"+EndTime+"</EndTime>\r\n";	
	strTemp += "<FilePath>" + DeviceID + "</FilePath >\r\n";
	strTemp +="<Address>address 1</Address >\r\n";
	strTemp += "<Secrecy>" + Secrecy + "</Secrecy>\r\n";
	strTemp += "<Type>" + Type + "</Type>\r\n";
	strTemp +="<RecorderID>"+ RecorderID +"</RecorderID>\r\n";
	strTemp +="</Query>\r\n";
	char *xml=(LPSTR)(LPCTSTR)strTemp;
	char *buf=new char[MAXBUFSIZE];
	CSipMsgProcess *sipVideoQuery=new CSipMsgProcess;
	sipVideoQuery->VideoSipXmlMsg(&buf,m_InfoServer, DeviceID,m_InfoClient,xml);
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
	//BeginTime.Format("%s",pWnd->m_VideoInfo[nCurVideo].BeginTime);
	m_BeginTime.GetWindowTextA(BeginTime);
	CString EndTime;
	//EndTime.Format("%s",pWnd->m_VideoInfo[nCurVideo].EndTime);
	m_EndTime.GetWindowTextA(EndTime);
	CString FileName;
	//FileName.Format("%s",pWnd->m_VideoInfo[nCurVideo].Name);
	m_HistoryVideoList.GetWindowTextA(FileName);
	CString strTemp;
	strTemp="<?xml version=\"1.0\"?>\r\n";
	strTemp+="<Action>\r\n";	
	strTemp+="<Query>\r\n";
	strTemp+="<CmdType>VOD</CmdType>\r\n";
	strTemp+="<Privilege>"+UserCode+"</Privilege>\r\n";
	strTemp+="<FileType>2</FileType>\r\n";	
	strTemp+="<Name>"+FileName+"</Name>\r\n";
	strTemp+="<BeginTime>"+BeginTime+"</BeginTime>\r\n";
	strTemp+="<EndTime>"+EndTime+"</EndTime>\r\n";
	strTemp+="<MaxBitrate>100</MaxBitrate>\r\n";
	strTemp+="</Query>\r\n";
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
	//HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	//CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	//int i=m_HistoryVideoList.GetCurSel();	
	//GetDlgItem(IDC_EDT_BEGIN)->SetWindowText(pWnd->m_VideoInfo[i].BeginTime);
	//GetDlgItem(IDC_EDT_END)->SetWindowText(pWnd->m_VideoInfo[i].EndTime);
	//GetDlgItem(IDC_EDT_FILESIZE)->SetWindowText(pWnd->m_VideoInfo[i].FileSize);
}


void CVideoQuery::OnCbnSelchangeSeladress()
{
	// TODO: 在此添加控件通知处理程序代码
	int index=m_selAddress.GetCurSel();
	CString Address=NotifyInfo.Devices[index].Address;
	GetDlgItem(IDC_EDT_ADDRESS)->SetWindowTextA(Address);
}

BOOL CVideoQuery::OnInitDialog()
{
	CDialog::OnInitDialog();
	GetDlgItem(IDC_EDT_PRIVILEGE)->SetWindowText("43");
	GetDlgItem(IDC_EDT_FILETYPE)->SetWindowText("time");
	GetDlgItem(IDC_EDT_BEGINTIME)->SetWindowText("2016-05-20T13:50:50Z");
	GetDlgItem(IDC_EDT_ENDTIME)->SetWindowText("2017-06-05T14:50:50Z");
	GetDlgItem(IDC_EDT_SECRECY)->SetWindowText("0");
	GetDlgItem(IDC_EDT_MAXFILENUM2)->SetWindowText("设备地址");
	GetDlgItem(IDC_EDT_ADDRESS)->SetWindowText("设备地址");
	return 0;
}
