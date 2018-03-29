// CaptureImage.cpp : 实现文件
//

#include "stdafx.h"
#include "UAS.h"
#include "CaptureImage.h"
#include "afxdialogex.h"
#include "UASDlg.h"

//成员变量
extern InfoServer m_InfoServer;
extern InfoClient m_InfoClient;
extern StatusCallID SCallId;
extern CString ShowTestTitle;
extern CString ShowTestData;
extern CRITICAL_SECTION g_uas;
extern queue<UA_Msg> uas_sendqueue;

// CCaptureImage 对话框

IMPLEMENT_DYNAMIC(CCaptureImage, CDialogEx)

CCaptureImage::CCaptureImage(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_CAPIMG, pParent)
{

}

CCaptureImage::~CCaptureImage()
{
}

const int CCaptureImage::HTTP_FLAG = 1;
const int CCaptureImage::FTP_FLAG = 2;

void CCaptureImage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_PRIVILEGE, m_Privilege);
	DDX_Control(pDX, IDC_COMBO_CAPTYPE, m_CapTureType);
}


BEGIN_MESSAGE_MAP(CCaptureImage, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_DOCAPTURE, &CCaptureImage::OnBnClickedButtonDocapture)
	ON_CBN_SELCHANGE(IDC_COMBO_PRIVILEGE, &CCaptureImage::OnCbnSelchangeComboPrivilege)
	ON_CBN_SELCHANGE(IDC_COMBO_CAPTYPE, &CCaptureImage::OnCbnSelchangeComboCaptype)
END_MESSAGE_MAP()


// CCaptureImage 消息处理程序
BOOL CCaptureImage::OnInitDialog()
{
	CDialog::OnInitDialog(); 

	GetDlgItem(IDC_EDIT_CAMADD)->SetWindowTextA("200400101160000328");

	m_CapTureType.InsertString(0, "0");//抓取实时流的I帧
	m_CapTureType.InsertString(1, "1");//IPC直接抓图
	capTureTypeContent.push_back("抓取实时流的I帧");
	capTureTypeContent.push_back("IPC直接抓图");
	m_CapTureType.SetCurSel(0);
	GetDlgItem(IDC_EDIT_CAPTYPE)->SetWindowTextA(capTureTypeContent[0]);
	/*广东版本*/
	/*
	m_Privilege.InsertString(0, "090");//中心管理员
	m_Privilege.InsertString(1, "091");//中心监控员
	m_Privilege.InsertString(2, "092");//普通监控员（由090,092权限分配）
	m_Privilege.InsertString(3, "093");//分中心管理员
	m_Privilege.InsertString(4, "094");//分中心监控员
	m_Privilege.InsertString(5, "095");//普通监控员（由094权限分配）
	m_Privilege.InsertString(6, "096");//基层站管理员
	m_Privilege.InsertString(7, "097");//普通监控员
	m_Privilege.InsertString(8, "098");//预留
	privilegeContent.push_back("中心管理员");
	privilegeContent.push_back("中心监控员");
	privilegeContent.push_back("普通监控员（由090,092权限分配）");
	privilegeContent.push_back("分中心管理员");
	privilegeContent.push_back("分中心监控员");
	privilegeContent.push_back("普通监控员（由094权限分配）");
	privilegeContent.push_back("基层站管理员");
	privilegeContent.push_back("普通监控员");
	privilegeContent.push_back("预留");
	*/
	m_Privilege.InsertString(0, "10");//系统管理员
	m_Privilege.InsertString(1, "20");//子系统用户
	m_Privilege.InsertString(2, "30");//高级用户
	m_Privilege.InsertString(3, "40");//普通用户
	m_Privilege.InsertString(4, "50");//浏览用户


	privilegeContent.push_back("系统管理员");
	privilegeContent.push_back("子系统用户");
	privilegeContent.push_back("高级用户");
	privilegeContent.push_back("普通用户");
	privilegeContent.push_back("浏览用户");

	m_Privilege.SetCurSel(1);
	GetDlgItem(IDC_EDIT_PRIVILEGE)->SetWindowTextA(privilegeContent[0]);

	return 0;
}

void CCaptureImage::OnBnClickedButtonDocapture()
{
	HWND   hnd = ::FindWindow(NULL, _T("UAS"));
	CUASDlg*  pWnd = (CUASDlg*)CWnd::FromHandle(hnd);
	//Create XML message
	CString CameraAddress;
	CString UserCode;
	CString CaptureType;
	GetDlgItem(IDC_EDIT_CAMADD)->GetWindowText(CameraAddress);
	string temp = CameraAddress;
	char * address = new char[CameraAddress.GetLength()+1];
	memset(address, 0, CameraAddress.GetLength() + 1);
	strcpy(address, temp.c_str());
	GetDlgItem(IDC_COMBO_PRIVILEGE)->GetWindowText(UserCode);
	GetDlgItem(IDC_COMBO_CAPTYPE)->GetWindowText(CaptureType);

	CString XmlCapImgSet;
	XmlCapImgSet = "<?xml version=\"1.0\"?>\r\n";
	XmlCapImgSet += "<Action>\r\n";
	XmlCapImgSet += "<Control>\r\n";
	XmlCapImgSet += "<CmdType>CaptureImage</CmdType>\r\n";
	//XmlCapImgSet += "<CameraAddress>" + CameraAddress + "</CameraAddress>\r\n";
	XmlCapImgSet += "<Privilege>" + UserCode + "</Privilege>\r\n";
	XmlCapImgSet += "<CaptureType>" + CaptureType + "</CaptureType>\r\n";
	XmlCapImgSet += "</Control>\r\n";
	XmlCapImgSet += "</Action>\r\n";
	char *destXMLCapImg = (LPSTR)(LPCTSTR)XmlCapImgSet;
	CSipMsgProcess *SipAlarm = new CSipMsgProcess;
	char *SipXmlCapImg = new char[MAXBUFSIZE];
	memset(SipXmlCapImg, 0, MAXBUFSIZE);
	SipAlarm->SipCaptureImageMsg(&SipXmlCapImg, m_InfoServer, m_InfoClient,address,destXMLCapImg);
	//send message to client
	if (m_InfoClient.Port == "" || m_InfoClient.IP == "")
	{
		delete SipXmlCapImg;
		MessageBox("没有注册的客户端用户", "UAS 提示", MB_OK | MB_ICONINFORMATION);
		return;
	}
	UA_Msg uas_sendtemp;
	strcpy(uas_sendtemp.data, SipXmlCapImg);
	EnterCriticalSection(&g_uas);
	uas_sendqueue.push(uas_sendtemp);
	LeaveCriticalSection(&g_uas);
	delete SipXmlCapImg;
	ShowTestData = "DO  ----------->\r\n";
	ShowTestTitle = "CaptureImage DO Test";
	SCallId.nStatus = CaptureImage;
}


void CCaptureImage::OnCbnSelchangeComboPrivilege()
{
	int index = m_Privilege.GetCurSel();
	GetDlgItem(IDC_EDIT_PRIVILEGE)->SetWindowTextA(privilegeContent[index]);
}


void CCaptureImage::OnCbnSelchangeComboCaptype()
{
	int index = m_CapTureType.GetCurSel();
	GetDlgItem(IDC_EDIT_CAPTYPE)->SetWindowTextA(capTureTypeContent[index]);
}
