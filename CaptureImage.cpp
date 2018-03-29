// CaptureImage.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "UAS.h"
#include "CaptureImage.h"
#include "afxdialogex.h"
#include "UASDlg.h"

//��Ա����
extern InfoServer m_InfoServer;
extern InfoClient m_InfoClient;
extern StatusCallID SCallId;
extern CString ShowTestTitle;
extern CString ShowTestData;
extern CRITICAL_SECTION g_uas;
extern queue<UA_Msg> uas_sendqueue;

// CCaptureImage �Ի���

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


// CCaptureImage ��Ϣ�������
BOOL CCaptureImage::OnInitDialog()
{
	CDialog::OnInitDialog(); 

	GetDlgItem(IDC_EDIT_CAMADD)->SetWindowTextA("200400101160000328");

	m_CapTureType.InsertString(0, "0");//ץȡʵʱ����I֡
	m_CapTureType.InsertString(1, "1");//IPCֱ��ץͼ
	capTureTypeContent.push_back("ץȡʵʱ����I֡");
	capTureTypeContent.push_back("IPCֱ��ץͼ");
	m_CapTureType.SetCurSel(0);
	GetDlgItem(IDC_EDIT_CAPTYPE)->SetWindowTextA(capTureTypeContent[0]);
	/*�㶫�汾*/
	/*
	m_Privilege.InsertString(0, "090");//���Ĺ���Ա
	m_Privilege.InsertString(1, "091");//���ļ��Ա
	m_Privilege.InsertString(2, "092");//��ͨ���Ա����090,092Ȩ�޷��䣩
	m_Privilege.InsertString(3, "093");//�����Ĺ���Ա
	m_Privilege.InsertString(4, "094");//�����ļ��Ա
	m_Privilege.InsertString(5, "095");//��ͨ���Ա����094Ȩ�޷��䣩
	m_Privilege.InsertString(6, "096");//����վ����Ա
	m_Privilege.InsertString(7, "097");//��ͨ���Ա
	m_Privilege.InsertString(8, "098");//Ԥ��
	privilegeContent.push_back("���Ĺ���Ա");
	privilegeContent.push_back("���ļ��Ա");
	privilegeContent.push_back("��ͨ���Ա����090,092Ȩ�޷��䣩");
	privilegeContent.push_back("�����Ĺ���Ա");
	privilegeContent.push_back("�����ļ��Ա");
	privilegeContent.push_back("��ͨ���Ա����094Ȩ�޷��䣩");
	privilegeContent.push_back("����վ����Ա");
	privilegeContent.push_back("��ͨ���Ա");
	privilegeContent.push_back("Ԥ��");
	*/
	m_Privilege.InsertString(0, "10");//ϵͳ����Ա
	m_Privilege.InsertString(1, "20");//��ϵͳ�û�
	m_Privilege.InsertString(2, "30");//�߼��û�
	m_Privilege.InsertString(3, "40");//��ͨ�û�
	m_Privilege.InsertString(4, "50");//����û�


	privilegeContent.push_back("ϵͳ����Ա");
	privilegeContent.push_back("��ϵͳ�û�");
	privilegeContent.push_back("�߼��û�");
	privilegeContent.push_back("��ͨ�û�");
	privilegeContent.push_back("����û�");

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
		MessageBox("û��ע��Ŀͻ����û�", "UAS ��ʾ", MB_OK | MB_ICONINFORMATION);
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
