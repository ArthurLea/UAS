// UASDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "UAS.h"
#include "UASDlg.h"
#include <fstream>
using namespace std;
static int m_SendPort;
static char m_SendIP[16] = { 0 };
static char ProcessIP[50] = { 0 };
queue<UA_Msg> uas_recvqueue;
queue<UA_Msg> uas_sendqueue;

UA_Msg uas_curqueue;
UA_Msg uas_curSendMsg;
//HANDLE hMutex_uas; 
CRITICAL_SECTION g_uas;
HANDLE h_UAS_Recv;
HANDLE h_UAS_Dispatch;
HANDLE h_UAS_Send;

InfoNotify NotifyInfo;
ofstream uas_msg_log;
BOOL bNetSet;
StatusCallID SCallId;
StatusCallID SEventAlarmNoticeDistributeID;//报警事件的通知和分发
StatusCallID SAlarmCallID;//预警需要使用的Call_ID等字段信息，用于数据缓存
StatusCallID sInviteCallID;
CallID InviteKeepAliveID;
char *ByeVia;
char *ByeFrom;
char *ByeTo;
char *CancelFrom;
char *CancelTo;
BOOL bACK;
BOOL bBYE;
BOOL bCANCEL;
BOOL bShowRealTime;
BOOL bNodeParent;
char strBye[MAXBUFSIZE];
char strCancel[MAXBUFSIZE];

hisQuery tHisVidQuery;
ptzQuery tPTZQuery;
//判断是否是心跳信息
char *contact;
BOOL bSipRegister;
BOOL bNodeType;
int nOverTime;
int nCurrentTime;
int nTimeCount;
time_t oldTime, currentTime;
BOOL bOverTime;
BOOL bFlag;
BOOL bVerify;
BOOL bUDPSipConnect;
BOOL bRevMsg;
char strPlayUrl[250];
char InviteCallID[100];
char ptztag[30];
//成员变量
SOCKET m_socket;
SOCKET m_socket_Equal;
InfoServer m_InfoServer;
InfoServer m_InfoServerEqual;
InfoClient m_InfoClient;
CString ShowTestTitle;
CString ShowTestData;
struct Authenticate g_authInfo;
struct Authenticate guac_authInfo;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

														// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// CUASDlg 对话框

CUASDlg::CUASDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUASDlg::IDD, pParent)
	, m_bIsShowKeepAliveMsg(FALSE)
	, m_CombIP(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void CUASDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_TestMember);
	DDX_Control(pDX, IDC_TAB, m_Ctab);
	DDX_Control(pDX, IDC_EDT_SENDMSG, m_ShowSendMsg);
	DDX_Control(pDX, IDC_EDT_RECV_MSG, m_ShowRecvMsg);
	DDX_Check(pDX, IDC_CHECK1, m_bIsShowKeepAliveMsg);
	DDX_CBString(pDX, IDC_COMBO_IP, m_CombIP);
	DDX_Control(pDX, IDC_COMBO_IP, m_ComboxLocalIP);
}

//添加消息响应函数，包括进程间通信的消息
BEGIN_MESSAGE_MAP(CUASDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	//自定义消息
	ON_MESSAGE(WM_RECVDATA, RecvData)
	ON_MESSAGE(WM_SENDDATA, SendMsgData)
	ON_MESSAGE(WM_RECEIVE, OnReceive)
	ON_MESSAGE(WM_SOCKETCLOSE, OnServerClose)
	ON_MESSAGE(WM_CONNCET, OnConnect)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CUASDlg::OnTcnSelchangeTab)
	ON_BN_CLICKED(IDC_BTN_OPEN_SIP, &CUASDlg::OnBnClickedBtnOpenSip)
	ON_BN_CLICKED(IDC_BTN_SEND_CLEAR, &CUASDlg::OnBnClickedBtnSendClear)
	ON_BN_CLICKED(IDC_BTN_RECV_CLEAR, &CUASDlg::OnBnClickedBtnRecvClear)
	ON_BN_CLICKED(IDC_BTN_SET, &CUASDlg::OnBnClickedBtnSet)
	ON_BN_CLICKED(IDC_BTN_LOG, &CUASDlg::OnBnClickedBtnLog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCANCEL, &CUASDlg::OnBnClickedCancel)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CUASDlg::OnCbnSelchangeCombo1)
	ON_STN_CLICKED(IDC_SABOUT, &CUASDlg::OnStnClickedSabout)
	ON_BN_CLICKED(IDC_CHECK1, &CUASDlg::OnBnClickedCheck1)
	ON_CBN_SELCHANGE(IDC_COMBO_IP, &CUASDlg::OnCbnSelchangeComboIp)
	ON_BN_CLICKED(IDC_BUTTON_REBOOT, &CUASDlg::OnBnClickedButtonReboot)
END_MESSAGE_MAP()

// CUASDlg 消息处理程序

BOOL CUASDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	CFont * f;
	f = new CFont;
	f->CreateFont(30,            // nHeight 
		10,           // nWidth 
		0,           // nEscapement 
		0,           // nOrientation 
		FW_BOLD,     // nWeight 
		FALSE,        // bItalic 
		FALSE,       // bUnderline 
		0,           // cStrikeOut 
		ANSI_CHARSET,              // nCharSet 
		OUT_DEFAULT_PRECIS,        // nOutPrecision 
		CLIP_DEFAULT_PRECIS,       // nClipPrecision 
		DEFAULT_QUALITY,           // nQuality 
		DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily 
		_T("Arial"));              // lpszFac
	GetDlgItem(IDC_STATICMT)->SetFont(f);
	CFont * f2;
	f2 = new CFont;
	f2->CreateFont(14,            // nHeight 
		6,           // nWidth 
		0,           // nEscapement 
		0,           // nOrientation 
		FW_BOLD,     // nWeight 
		FALSE,        // bItalic 
		TRUE,       // bUnderline 
		0,           // cStrikeOut 
		ANSI_CHARSET,              // nCharSet 
		OUT_DEFAULT_PRECIS,        // nOutPrecision 
		CLIP_DEFAULT_PRECIS,       // nClipPrecision 
		DEFAULT_QUALITY,           // nQuality 
		DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily 
		_T("宋体"));              // lpszFac
	GetDlgItem(IDC_SABOUT)->SetFont(f2);
	// TODO: 在此添加额外的初始化代码
	GetDlgItem(IDC_USERNAMES)->SetWindowText(_T("client_user"));
	GetDlgItem(IDC_PASSWORDS)->SetWindowText("123456");

	InitProgram();
	InitNetSet();
	InitInvite();
	InitPTZ();
	//InitHistoryVideoQuery();
	InitHistoryVideoPlay();
	InitAlarm();
	//InitCatalogQuery();
	//InitDeviceInfQuery();
	InitFlowQuery();
	InitCoderSet();
	InitEnableWindow();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CUASDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CUASDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CUASDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CUASDlg::InitProgram()
{
	//初始化厂商列表组合框
	FILE *TestMemberFile = NULL;
	TestMemberFile = fopen("测试厂商列表.txt", "r");
	char *temp = new char[40];
	char *temp1 = new char[40];
	int i = 0;
	ProductMember test;
	if (TestMemberFile != NULL)
	{
		while (1)
		{
			if (feof(TestMemberFile))
				break;
			fscanf(TestMemberFile, "%s\n", temp);
			fscanf(TestMemberFile, "%s\n", temp1);
			strcpy(test.IP, temp1);
			ProductTestMember.push_back(test);
			m_TestMember.InsertString(i, temp);
			i++;
		}
	}
	else
	{
		TestMemberFile = fopen("测试厂商列表.txt", "w");
		m_TestMember.AddString("海康威视");
		fprintf(TestMemberFile, "海康威视");
		fprintf(TestMemberFile, "\n192.168.1.100");
		strcpy(test.IP, "192.168.1.100");
		ProductTestMember.push_back(test);
		m_TestMember.AddString("H3C");
		fprintf(TestMemberFile, "\nH3C");
		fprintf(TestMemberFile, "\n192.168.1.111");
		strcpy(test.IP, "192.168.1.111");
		ProductTestMember.push_back(test);
		m_TestMember.AddString("武汉微创光电");
		fprintf(TestMemberFile, "\n武汉微创光电");
		fprintf(TestMemberFile, "\n192.168.1.50");
		strcpy(test.IP, "192.168.1.50");
		ProductTestMember.push_back(test);
		m_TestMember.AddString("航天科工");
		fprintf(TestMemberFile, "\n航天科工");
		fprintf(TestMemberFile, "\n192.168.1.43");
		strcpy(test.IP, "192.168.1.43");
		ProductTestMember.push_back(test);
		m_TestMember.AddString("江苏亚奥");
		fprintf(TestMemberFile, "\n江苏亚奥");
		fprintf(TestMemberFile, "\n192.168.1.30");
		strcpy(test.IP, "192.168.1.30");
		ProductTestMember.push_back(test);
		m_TestMember.AddString("杭州中威");
		fprintf(TestMemberFile, "\n杭州中威");
		fprintf(TestMemberFile, "\n192.168.1.20");
		strcpy(test.IP, "192.168.1.20");
		ProductTestMember.push_back(test);
		m_TestMember.AddString("北京蛙视");
		fprintf(TestMemberFile, "\n北京蛙视");
		fprintf(TestMemberFile, "\n192.168.1.73");
		strcpy(test.IP, "192.168.1.73");
		ProductTestMember.push_back(test);
	}
	m_TestMember.SetCurSel(0);
	strcpy(ProcessIP, ProductTestMember[0].IP);
	if (TestMemberFile)
	{
		fclose(TestMemberFile);
	}
	delete[]temp;
	delete[]temp1;
	//初始化标签页
	m_Ctab.InsertItem(0, _T("网络设置"));
	m_Ctab.InsertItem(1, _T("实时流"));
	m_Ctab.InsertItem(2, _T("设备控制"));
	m_Ctab.InsertItem(3, _T("视频查询"));
	m_Ctab.InsertItem(4, _T("视频回放"));
	m_Ctab.InsertItem(5, _T("报警测试"));
	m_Ctab.InsertItem(6, _T("目录查询及设备状态查询"));
	m_Ctab.InsertItem(7, _T("设备信息查询"));
	m_Ctab.InsertItem(8, _T("流量查询"));
	m_Ctab.InsertItem(9, _T("编码器设置"));
	m_Ctab.InsertItem(10, _T("时间被动设置"));
	m_Ctab.InsertItem(11, _T("监控抓图"));
	/****************************************/
	/*add some function for GB28181**********/
	/****************************************/
	m_Ctab.InsertItem(12,_T("报警事件通知与分析"));



	//为标签页添加初始化对话框
	m_NetSet.Create(IDD_DLG_NETSET, GetDlgItem(IDC_TAB));
	m_Invite.Create(IDD_DLG_INVITE, GetDlgItem(IDC_TAB));
	m_PTZ.Create(IDD_DLG_PTZ, GetDlgItem(IDC_TAB));
	m_VideoQuery.Create(IDD_DLG_VIDEOQUERY, GetDlgItem(IDC_TAB));
	m_VideoPlay.Create(IDD_DLG_VIDEOPLAY, GetDlgItem(IDC_TAB));
	m_Alarm.Create(IDD_DLG_ALARM, GetDlgItem(IDC_TAB));
	m_CatalogQuery.Create(IDD_DLG_CATALOGUREQUERY, GetDlgItem(IDC_TAB));
	m_DeviceInfQuery.Create(IDD_DLG_DEVICEINFQUERY, GetDlgItem(IDC_TAB));
	m_FlowQuery.Create(IDD_DLG_FLOWQUERY, GetDlgItem(IDC_TAB));
	m_CoderSet.Create(IDD_DLG_CODERSET, GetDlgItem(IDC_TAB)); 
	m_PSTVSetTime.Create(IDD_DIG_PSTVTTIME, GetDlgItem(IDC_TAB));
	m_CaptureImg.Create(IDD_DLG_CAPIMG, GetDlgItem(IDC_TAB));

	/****************************************/
	/*add some function for GB28181**********/
	/****************************************/
	m_CeventNotice.Create(IDD_DIG_ALARMEVENT_NOTICE_DISTRIBUTE,GetDlgItem(IDC_TAB));



	//获得IDC_TAB客户区大小
	CRect rect;
	m_Ctab.GetClientRect(&rect);
	//调整子对话框在父窗口中的位置，可以改动数值，使子窗体的大小合适
	rect.top += 22;
	rect.bottom -= 3;
	rect.left += 2;
	rect.right -= 4;
	//设置子对话框尺寸并移动到指定位置
	m_NetSet.MoveWindow(&rect);
	m_Invite.MoveWindow(&rect);
	m_PTZ.MoveWindow(&rect);
	m_VideoQuery.MoveWindow(&rect);
	m_VideoPlay.MoveWindow(&rect);
	m_Alarm.MoveWindow(&rect);
	m_CatalogQuery.MoveWindow(&rect);
	m_DeviceInfQuery.MoveWindow(&rect);
	m_FlowQuery.MoveWindow(&rect);
	m_CoderSet.MoveWindow(&rect);
	m_PSTVSetTime.MoveWindow(&rect);
	m_CaptureImg.MoveWindow(&rect);

	/****************************************/
	/*add some function for GB28181**********/
	/****************************************/
	m_CeventNotice.MoveWindow(&rect);

	//分别设置隐藏和显示
	m_NetSet.ShowWindow(true);
	//设置默认的选项卡
	m_Ctab.SetCurSel(0);
	bSipRegister = FALSE;
	bNodeType = FALSE;
	bOverTime = FALSE;
	bVerify = FALSE;
	SCallId.nStatus = 1000;
	bACK = FALSE;
	bBYE = FALSE;
	bCANCEL = FALSE;
	bRevMsg = TRUE;
	bUDPSipConnect = FALSE;
	nTimeCount = 0;
	bNodeParent = FALSE;
	strcpy(strPlayUrl, "");
	ByeVia = new char[100];
	ByeFrom = new char[100];
	ByeTo = new char[100];
	rtspUrl = "";
	RTSPIP = "";
	RTSPPort = "";
	m_TCPSocket.Initialize(this);
	bRTSPLIVE = FALSE;
	brtspKeeplive = FALSE;
	bSelectKeepLive = TRUE;
	strcpy(InviteCallID, "");
	strcpy(ptztag, "");
	inviteAddress = "";
	nRealtime = 1;
	bRealTimeKeepLive = FALSE;
	videoAddress = "";
	encodeAddress = "";
	catalogAddress = "";
	deviceInfAddress = "";
	flowAddress = "";
	nPtz = 1;
	bSeesion = FALSE;
	strSession = "";

}

void CUASDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	int CurSel = m_Ctab.GetCurSel();
	switch (CurSel)
	{
	case 0:
		m_NetSet.ShowWindow(true);
		m_Invite.ShowWindow(false);
		m_PTZ.ShowWindow(false);
		m_VideoQuery.ShowWindow(false);
		m_VideoPlay.ShowWindow(false);
		m_Alarm.ShowWindow(false);
		m_CatalogQuery.ShowWindow(false);
		m_DeviceInfQuery.ShowWindow(false);
		m_FlowQuery.ShowWindow(false);
		m_CoderSet.ShowWindow(false);	
		m_PSTVSetTime.ShowWindow(false);
		m_CaptureImg.ShowWindow(false);
		m_CeventNotice.ShowWindow(false);
		break;
	case 1:
		m_NetSet.ShowWindow(false);
		m_Invite.ShowWindow(true);
		m_PTZ.ShowWindow(false);
		m_VideoQuery.ShowWindow(false);
		m_VideoPlay.ShowWindow(false);
		m_Alarm.ShowWindow(false);
		m_CatalogQuery.ShowWindow(false);
		m_DeviceInfQuery.ShowWindow(false);
		m_FlowQuery.ShowWindow(false);
		m_CoderSet.ShowWindow(false);
		m_PSTVSetTime.ShowWindow(false);
		m_CaptureImg.ShowWindow(false);
		m_CeventNotice.ShowWindow(false);
		break;
	case 2:
		m_NetSet.ShowWindow(false);
		m_Invite.ShowWindow(false);
		m_PTZ.ShowWindow(true);
		m_VideoQuery.ShowWindow(false);
		m_VideoPlay.ShowWindow(false);
		m_Alarm.ShowWindow(false);
		m_CatalogQuery.ShowWindow(false);
		m_DeviceInfQuery.ShowWindow(false);
		m_FlowQuery.ShowWindow(false);
		m_CoderSet.ShowWindow(false);
		m_PSTVSetTime.ShowWindow(false);
		m_CaptureImg.ShowWindow(false);
		m_CeventNotice.ShowWindow(false);
		break;
	case 3:
		m_NetSet.ShowWindow(false);
		m_Invite.ShowWindow(false);
		m_PTZ.ShowWindow(false);
		m_VideoQuery.ShowWindow(true);
		m_VideoPlay.ShowWindow(false);
		m_Alarm.ShowWindow(false);
		m_CatalogQuery.ShowWindow(false);
		m_DeviceInfQuery.ShowWindow(false);
		m_FlowQuery.ShowWindow(false);
		m_CoderSet.ShowWindow(false);
		m_PSTVSetTime.ShowWindow(false);
		m_CaptureImg.ShowWindow(false);
		m_CeventNotice.ShowWindow(false);
		break;
	case 4:
		m_NetSet.ShowWindow(false);
		m_Invite.ShowWindow(false);
		m_PTZ.ShowWindow(false);
		m_VideoQuery.ShowWindow(false);
		m_VideoPlay.ShowWindow(true);
		m_Alarm.ShowWindow(false);
		m_CatalogQuery.ShowWindow(false);
		m_DeviceInfQuery.ShowWindow(false);
		m_FlowQuery.ShowWindow(false);
		m_CoderSet.ShowWindow(false);
		m_PSTVSetTime.ShowWindow(false);
		m_CaptureImg.ShowWindow(false);
		m_CeventNotice.ShowWindow(false);
		break;
	case 5:
		m_NetSet.ShowWindow(false);
		m_Invite.ShowWindow(false);
		m_PTZ.ShowWindow(false);
		m_VideoQuery.ShowWindow(false);
		m_VideoPlay.ShowWindow(false);
		m_Alarm.ShowWindow(true);
		m_CatalogQuery.ShowWindow(false);
		m_DeviceInfQuery.ShowWindow(false);
		m_FlowQuery.ShowWindow(false);
		m_CoderSet.ShowWindow(false);
		m_PSTVSetTime.ShowWindow(false);
		m_CaptureImg.ShowWindow(false);
		m_CeventNotice.ShowWindow(false);
		break;
	case 6:
		m_NetSet.ShowWindow(false);
		m_Invite.ShowWindow(false);
		m_PTZ.ShowWindow(false);
		m_VideoQuery.ShowWindow(false);
		m_VideoPlay.ShowWindow(false);
		m_Alarm.ShowWindow(false);
		m_CatalogQuery.ShowWindow(true);
		m_DeviceInfQuery.ShowWindow(false);
		m_FlowQuery.ShowWindow(false);
		m_CoderSet.ShowWindow(false);
		m_PSTVSetTime.ShowWindow(false);
		m_CaptureImg.ShowWindow(false);
		m_CeventNotice.ShowWindow(false);
		break;
	case 7:
		m_NetSet.ShowWindow(false);
		m_Invite.ShowWindow(false);
		m_PTZ.ShowWindow(false);
		m_VideoQuery.ShowWindow(false);
		m_VideoPlay.ShowWindow(false);
		m_Alarm.ShowWindow(false);
		m_CatalogQuery.ShowWindow(false);
		m_DeviceInfQuery.ShowWindow(true);
		m_FlowQuery.ShowWindow(false);
		m_CoderSet.ShowWindow(false);
		m_PSTVSetTime.ShowWindow(false);
		m_CaptureImg.ShowWindow(false);
		m_CeventNotice.ShowWindow(false);
		break;
	case 8:
		m_NetSet.ShowWindow(false);
		m_Invite.ShowWindow(false);
		m_PTZ.ShowWindow(false);
		m_VideoQuery.ShowWindow(false);
		m_VideoPlay.ShowWindow(false);
		m_Alarm.ShowWindow(false);
		m_CatalogQuery.ShowWindow(false);
		m_DeviceInfQuery.ShowWindow(false);
		m_FlowQuery.ShowWindow(true);
		m_CoderSet.ShowWindow(false);
		m_PSTVSetTime.ShowWindow(false);
		m_CaptureImg.ShowWindow(false);
		m_CeventNotice.ShowWindow(false);
		break;
	case 9:
		m_NetSet.ShowWindow(false);
		m_Invite.ShowWindow(false);
		m_PTZ.ShowWindow(false);
		m_VideoQuery.ShowWindow(false);
		m_VideoPlay.ShowWindow(false);
		m_Alarm.ShowWindow(false);
		m_CatalogQuery.ShowWindow(false);
		m_DeviceInfQuery.ShowWindow(false);
		m_FlowQuery.ShowWindow(false);
		m_CoderSet.ShowWindow(true);
		m_PSTVSetTime.ShowWindow(false);
		m_CaptureImg.ShowWindow(false);
		m_CeventNotice.ShowWindow(false);
		break;
	case 10:
		m_NetSet.ShowWindow(false);
		m_Invite.ShowWindow(false);
		m_PTZ.ShowWindow(false);
		m_VideoQuery.ShowWindow(false);
		m_VideoPlay.ShowWindow(false);
		m_Alarm.ShowWindow(false);
		m_CatalogQuery.ShowWindow(false);
		m_DeviceInfQuery.ShowWindow(false);
		m_FlowQuery.ShowWindow(false);
		m_CoderSet.ShowWindow(false);
		m_PSTVSetTime.ShowWindow(true);
		m_CaptureImg.ShowWindow(false);
		m_CeventNotice.ShowWindow(false);
		break; 
	case 11:
		m_NetSet.ShowWindow(false);
		m_Invite.ShowWindow(false);
		m_PTZ.ShowWindow(false);
		m_VideoQuery.ShowWindow(false);
		m_VideoPlay.ShowWindow(false);
		m_Alarm.ShowWindow(false);
		m_CatalogQuery.ShowWindow(false);
		m_DeviceInfQuery.ShowWindow(false);
		m_FlowQuery.ShowWindow(false);
		m_CoderSet.ShowWindow(false);
		m_PSTVSetTime.ShowWindow(false);
		m_CaptureImg.ShowWindow(true);
		m_CeventNotice.ShowWindow(false);
		break; 
	case 12:
		m_NetSet.ShowWindow(false);
		m_Invite.ShowWindow(false);
		m_PTZ.ShowWindow(false);
		m_VideoQuery.ShowWindow(false);
		m_VideoPlay.ShowWindow(false);
		m_Alarm.ShowWindow(false);
		m_CatalogQuery.ShowWindow(false);
		m_DeviceInfQuery.ShowWindow(false);
		m_FlowQuery.ShowWindow(false);
		m_CoderSet.ShowWindow(false);
		m_PSTVSetTime.ShowWindow(false);
		m_CaptureImg.ShowWindow(false);
		m_CeventNotice.ShowWindow(true);
		break; 
	default:
		break;
	}
	*pResult = 0;
}

//封装成员函数--是否启用或禁用控件--默认为真状态启用按钮
BOOL CUASDlg::EnableWindow(UINT uID, BOOL bEnable = TRUE)
{
	return ::EnableWindow(GetDlgItem(uID)->GetSafeHwnd(), bEnable);
}

//封装成员函数--获取本地计算机名称
int CUASDlg::GetLocalHostName(CString &sHostName)
{
	char szHostName[40];
	int nRetCode;
	nRetCode = gethostname(szHostName, sizeof(szHostName));
	if (nRetCode != 0)
	{
		MessageBox("本地计算机名称获取失败", "UAS 出错", MB_OK | MB_ICONERROR);
		return GetLastError();
	}
	sHostName = szHostName;
	return 0;
}

//封装成员函数--获取本地计算机IP地址
int CUASDlg::GetLocalIp(const CString &sHostName, CString &sIpAddress)
{
	//将CString类型转换成char*
	int strLength = sHostName.GetLength() + 1;
	char *cHostName = new char[strLength];
	strncpy(cHostName, sHostName, strLength);
	struct hostent FAR * lpHostEnt = gethostbyname(cHostName);
	if (lpHostEnt == NULL)
	{
		//产生错误		
		MessageBox("本地计算机IP地址获取失败", "UAS 出错", MB_OK | MB_ICONERROR);
		return GetLastError();
	}
	//获取IP地址
	LPSTR lpAddr = lpHostEnt->h_addr_list[0];

	if (lpAddr)
	{
		struct in_addr inAddr;
		memmove(&inAddr, lpAddr, 4);
		//格式转换为标准格式
		sIpAddress = inet_ntoa(inAddr);
		m_ComboxLocalIP.InsertString(0, sIpAddress);
		if (sIpAddress.IsEmpty())
			MessageBox("本地计算机IP地址获取失败", "UAS 出错", MB_OK | MB_ICONERROR);
	}
	lpAddr = lpHostEnt->h_addr_list[1];

	if (lpAddr)
	{
		struct in_addr inAddr;
		memmove(&inAddr, lpAddr, 4);
		//格式转换为标准格式
		sIpAddress = inet_ntoa(inAddr);
		m_ComboxLocalIP.InsertString(1, sIpAddress);
		if (sIpAddress.IsEmpty())
			MessageBox("本地计算机IP地址获取失败", "UAS 出错", MB_OK | MB_ICONERROR);
	}
	m_ComboxLocalIP.SetCurSel(0);

	return 0;
}

//初始化网络配置信息
void CUASDlg::InitNetSet()
{
	//设置初始化本地IP地址和端口本地网络配置信息	
	CString LocalHostName;
	GetLocalHostName(LocalHostName);
	GetLocalIp(LocalHostName, m_InfoServer.IP);
	/*m_InfoServer.Port="5800";
	m_InfoServer.UserAddress="456";
	m_InfoServer.UserName="456";*/
	FILE *NetFile = NULL;
	NetFile = fopen("UASNetLog.txt", "r");
	char *temp = new char[40];
	if (NetFile != NULL)
	{
		fscanf(NetFile, "%s\n", temp);
		m_InfoServer.Port.Format("%s", temp);

		fscanf(NetFile, "%s\n", temp);
		m_InfoServer.UserName.Format("%s", temp);

		fscanf(NetFile, "%s\n", temp);
		m_InfoServer.UserAddress.Format("%s", temp);
	}
	else
	{
		NetFile = fopen("UASNetLog.txt", "w");
		m_InfoServer.Port = "5800";
		m_InfoServer.UserName = "456";
		m_InfoServer.UserAddress = "456";
		fprintf(NetFile, "5800");
		fprintf(NetFile, "\n456");
		fprintf(NetFile, "\n456");
	}
	if (NetFile)
	{
		fclose(NetFile);
	}
	delete[]temp;

	GetDlgItem(IDC_STR_LOCALIP)->SetWindowText(m_InfoServer.IP);
	GetDlgItem(IDC_STR_LOCAL_PORT)->SetWindowText(m_InfoServer.Port);
	GetDlgItem(IDC_STR_LOCAL_ADD)->SetWindowText(m_InfoServer.UserAddress);
	GetDlgItem(IDC_STR_LOCAL_NAME)->SetWindowText(m_InfoServer.UserName);
	//网络配置属性页初始化	
	m_NetSet.GetDlgItem(IDC_IP)->SetWindowText(m_InfoServer.IP);
	m_NetSet.GetDlgItem(IDC_EDT_PORT)->SetWindowText(m_InfoServer.Port);
	m_NetSet.GetDlgItem(IDC_EDT_ADDRESS)->SetWindowText(m_InfoServer.UserAddress);
	m_NetSet.GetDlgItem(IDC_EDT_NAME)->SetWindowText(m_InfoServer.UserName);
	bNetSet = TRUE;
}

//初始化实时流测试项目配置信息
void CUASDlg::InitInvite()
{
	CString ipStr = m_InfoServer.IP;
	ipStr += " UDP 1236";
	//实时流属性页初始化
	m_Invite.GetDlgItem(IDC_EDT_USERCODE)->SetWindowText("20");//用户权限码
	m_Invite.GetDlgItem(IDC_EDT_FORMAT)->SetWindowText("4CIF");
	m_Invite.GetDlgItem(IDC_EDT_VIDEO)->SetWindowText("H.264");
	m_Invite.GetDlgItem(IDC_EDT_AUDIO)->SetWindowText("G.722");
	m_Invite.GetDlgItem(IDC_EDT_MAXBIT)->SetWindowText("800");
	m_Invite.GetDlgItem(IDC_EDT_TRANSMODE)->SetWindowText("null");
	m_Invite.GetDlgItem(IDC_EDT_PROTOCOL)->SetWindowText("地址编码");
	m_Invite.GetDlgItem(IDC_EDT_MULTICAST)->SetWindowText("0");
	m_Invite.GetDlgItem(IDC_EDT_SOCKET)->SetWindowText(ipStr);
}

//初始化云台控制属性页
void CUASDlg::InitPTZ()
{
	m_PTZ.GetDlgItem(IDC_EDIT_SN)->SetWindowText("11");
	m_PTZ.GetDlgItem(IDC_EDT_ADD)->SetWindowText("011051430001");
	m_PTZ.GetDlgItem(IDC_EDT_PTZ)->SetWindowText("A50F4D1000001021");
	m_PTZ.GetDlgItem(IDC_EDT_CONTROLPRIORITY)->SetWindowText("5");
	m_PTZ.GetDlgItem(IDC_EDT_PTL)->SetWindowText("null");
	m_PTZ.GetDlgItem(IDC_EDT_BEGINNUM)->SetWindowText("1");
	m_PTZ.GetDlgItem(IDC_EDT_ENDNUM)->SetWindowText("10");
}

//初始化视频查询属性页
//void CUASDlg::InitHistoryVideoQuery()
//{
//	m_VideoQuery.GetDlgItem(IDC_EDT_PRIVILEGE)->SetWindowText("43");
//	m_VideoQuery.GetDlgItem(IDC_EDT_FILETYPE)->SetWindowText("time");
//	m_VideoQuery.GetDlgItem(IDC_EDT_BEGINTIME)->SetWindowText("2016-05-20T13:50:50Z");
//	m_VideoQuery.GetDlgItem(IDC_EDT_ENDTIME)->SetWindowText("2017-06-05T14:50:50Z");
//	m_VideoQuery.GetDlgItem(IDC_EDT_MAXFILENUM)->SetWindowText("0");
//	m_VideoQuery.GetDlgItem(IDC_EDT_MAXFILENUM2)->SetWindowText("设备地址");
//	m_VideoQuery.GetDlgItem(IDC_EDT_ADDRESS)->SetWindowText("设备地址");
//}

//初始化视频回放属性页
void CUASDlg::InitHistoryVideoPlay()
{
	m_VideoPlay.GetDlgItem(IDC_EDT_DESTINATION)->SetWindowText("192.168.17.99");
	m_VideoPlay.GetDlgItem(IDC_EDT_CLIENTPORT)->SetWindowText("4588-4589");
	m_VideoPlay.GetDlgItem(IDC_EDT_RANGE)->SetWindowText("npt=0.000-");
	m_VideoPlay.GetDlgItem(IDC_EDT_SCALE)->SetWindowText("1.0");
}

//初始化编码器设置属性页
void CUASDlg::InitCoderSet()
{
	m_CoderSet.GetDlgItem(IDC_EDT_USERCODE)->SetWindowText("030255123490");
	m_CoderSet.GetDlgItem(IDC_EDT_FORMAT)->SetWindowText("4CIF");
	m_CoderSet.GetDlgItem(IDC_EDT_FRAME)->SetWindowText("25");
	m_CoderSet.GetDlgItem(IDC_EDT_BITRATE)->SetWindowText("300");
	m_CoderSet.GetDlgItem(IDC_EDT_PRIORITY)->SetWindowText("0");
	m_CoderSet.GetDlgItem(IDC_EDT_GOP)->SetWindowText("8");
	m_CoderSet.GetDlgItem(IDC_EDIT_IMAGEQUALITY)->SetWindowText("高");
	m_CoderSet.GetDlgItem(IDC_EDT_ADDRESS)->SetWindowText("011061430001");
}

//初始化报警测试属性页
void CUASDlg::InitAlarm()
{
	m_Alarm.GetDlgItem(IDC_EDT_ADDRESS)->SetWindowText("011061430001");
	m_Alarm.GetDlgItem(IDC_EDT_USERCODE)->SetWindowText("1");
	m_Alarm.GetDlgItem(IDC_EDT_LEVEL)->SetWindowText("4");
	m_Alarm.m_AlarmTypeSel.SetCurSel(0);
	m_Alarm.GetDlgItem(IDC_EDT_ALARM_TYPE)->SetWindowText("1");
	m_Alarm.GetDlgItem(IDC_EDT_IP)->SetWindowText("2010-11-11T00:00:00");
	m_Alarm.GetDlgItem(IDC_EDT_PORT)->SetWindowText("2099-12-11T00:00:00");

}

//void CUASDlg::InitCatalogQuery()
//{
//	m_CatalogQuery.GetDlgItem(IDC_EDT_PRIVILEGE)->SetWindowText("20");
//	m_CatalogQuery.GetDlgItem(IDC_EDT_ADDRESS)->SetWindowText("011061430001");//UAC 地址编码 252000001199000001
//}

//void CUASDlg::InitDeviceInfQuery()
//{
//	m_DeviceInfQuery.GetDlgItem(IDC_EDT_PRIVILEGE)->SetWindowText("20");
//	m_DeviceInfQuery.GetDlgItem(IDC_SELADRESS)->SetWindowText("Catalog");
//	m_DeviceInfQuery.m_selAddress.SetCurSel(0);
//	m_DeviceInfQuery.GetDlgItem(IDC_EDT_ADDRESS)->SetWindowText("011061430001");
//}

void CUASDlg::InitFlowQuery()
{
	m_FlowQuery.GetDlgItem(IDC_EDT_PRIVILEGE)->SetWindowText("20");
	m_FlowQuery.GetDlgItem(IDC_EDT_ADDRESS)->SetWindowText("设备地址");
}

void CUASDlg::InitEnableWindow()
{
	//初始化禁用日志流程和测试报告按钮
	EnableWindow(IDC_BTN_LOG, FALSE);
	EnableWindow(IDC_BTN_RESULT, FALSE);
	m_NetSet.GetDlgItem(IDC_IP)->EnableWindow(FALSE);
	m_NetSet.GetDlgItem(IDC_EDT_PORT)->EnableWindow(FALSE);
	m_NetSet.GetDlgItem(IDC_EDT_ADDRESS)->EnableWindow(FALSE);
	m_NetSet.GetDlgItem(IDC_EDT_NAME)->EnableWindow(FALSE);
	m_Invite.GetDlgItem(IDC_BTN_TEST)->EnableWindow(FALSE);
	m_Invite.GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(FALSE); 
	m_Invite.GetDlgItem(IDC_BTN_PLAY)->EnableWindow(FALSE);
	m_Invite.GetDlgItem(IDC_BTN_BYE)->EnableWindow(FALSE);
	m_PTZ.GetDlgItem(IDC_BTN_TEST)->EnableWindow(FALSE);
	m_PTZ.GetDlgItem(IDC_BTN_PRE)->EnableWindow(FALSE);
	m_VideoQuery.GetDlgItem(IDC_BTN_QUERY)->EnableWindow(TRUE); //YWD
	m_VideoQuery.GetDlgItem(IDC_BTN_GETURL)->EnableWindow(FALSE);
	m_VideoPlay.GetDlgItem(IDC_BTN_START)->EnableWindow(FALSE);
	m_VideoPlay.GetDlgItem(IDC_BTN_PLAY)->EnableWindow(FALSE);
	m_VideoPlay.GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	m_Alarm.GetDlgItem(IDC_BTN_ALARM_SET)->EnableWindow(FALSE);
	m_Alarm.GetDlgItem(IDC_BTN_ALARM_CANCEL)->EnableWindow(FALSE);
	m_CatalogQuery.GetDlgItem(IDC_QUERY)->EnableWindow(FALSE);
	m_CatalogQuery.GetDlgItem(IDC_DEVICEINFQUERY2)->EnableWindow(FALSE);
	m_DeviceInfQuery.GetDlgItem(IDC_DEVICEINFQUERY)->EnableWindow(FALSE); 
	m_FlowQuery.GetDlgItem(IDC_FLOWQUERY)->EnableWindow(FALSE);
	m_CoderSet.GetDlgItem(IDC_BTN_SET)->EnableWindow(FALSE);
	m_PSTVSetTime.GetDlgItem(IDC_BUTTON_NTPTIME)->EnableWindow(FALSE);
	m_PSTVSetTime.GetDlgItem(IDC_BUTTON_CurTIME)->EnableWindow(FALSE);
	m_PSTVSetTime.GetDlgItem(IDC_BUTTON_SETTIME)->EnableWindow(FALSE);
	m_CaptureImg.GetDlgItem(IDC_BUTTON_DOCAPTURE)->EnableWindow(FALSE);
}

int CUASDlg::InitSocket(int port)
{
	m_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == m_socket)
	{
		MessageBox("load socket is error", "UAS  Error", MB_OK | MB_ICONERROR);
		return -1;
	}
	SOCKADDR_IN  addrSocket;
	addrSocket.sin_family = AF_INET;
	addrSocket.sin_port = htons(port);
	addrSocket.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	int revel;
	revel = bind(m_socket, (SOCKADDR*)&addrSocket, sizeof(SOCKADDR));
	if (SOCKET_ERROR == revel)
	{
		closesocket(m_socket);
		MessageBox("端口被占用", "UAS  警告", MB_OK | MB_ICONINFORMATION);
		return -1;
	}
	return 0;
}

int CUASDlg::InitSocketEqual(int port)
{
	m_socket_Equal = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == m_socket_Equal)
	{
		MessageBox("load socket is error", "UAS  Error", MB_OK | MB_ICONERROR);
		return -1;
	}
	SOCKADDR_IN  addrSocket;
	addrSocket.sin_family = AF_INET;
	addrSocket.sin_port = htons(port);
	addrSocket.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	int revel;
	revel = bind(m_socket_Equal, (SOCKADDR*)&addrSocket, sizeof(SOCKADDR));
	if (SOCKET_ERROR == revel)
	{
		closesocket(m_socket_Equal);
		MessageBox("端口被占用", "UAS  警告", MB_OK | MB_ICONINFORMATION);
		return -1;
	}
	return 0;
}

void CUASDlg::ShowSendData(CString StrSendData)
{
	CTime   theTime = CTime::GetCurrentTime();
	CString str = theTime.Format(_T("%H:%M:%S"));
	CString strtemp;
	GetDlgItemText(IDC_EDT_SENDMSG, strtemp);
	strtemp += "\t系统时间：" + str + "\r\n";
	strtemp += StrSendData;
	strtemp += "\r\n";
	uas_msg_log << StrSendData << endl << endl << endl;
	SetDlgItemText(IDC_EDT_SENDMSG, strtemp);
	GetDlgItem(IDC_EDT_SENDMSG)->SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
}

void CUASDlg::ShowRecvData(CString strRecvData)
{
	string st = strRecvData.GetBuffer(strRecvData.GetLength());
	int index = st.find("KeepAlive");
	BOOL b = IsDlgButtonChecked(IDC_CHECK1);
	if (index == string::npos || b)//index == string::npos 表示不是保活信息
	{
		//ShowSendData(data);
		CTime   theTime = CTime::GetCurrentTime();
		CString str = theTime.Format(_T("%H:%M:%S"));
		CString strtemp;
		GetDlgItemText(IDC_EDT_RECV_MSG, strtemp);
		strtemp += "\t系统时间：" + str + "\r\n";
		strtemp += strRecvData;
		strtemp += "\r\n";
		uas_msg_log << strRecvData << endl << endl << endl;
		SetDlgItemText(IDC_EDT_RECV_MSG, strtemp);
		GetDlgItem(IDC_EDT_RECV_MSG)->SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
	}

}

DWORD WINAPI RecvMsg(LPVOID lpParameter)
{
	SOCKET sock = ((RECVPARAM*)lpParameter)->sock;
	HWND   hwnd = ((RECVPARAM*)lpParameter)->hwnd;
	SOCKADDR_IN addrFrom;
	int len = sizeof(SOCKADDR);
	char RecvBuf[MAXBUFSIZE];
	memset(RecvBuf, 0, MAXBUFSIZE);
	int retvel;
	while (TRUE)
	{
		retvel = recvfrom(sock, RecvBuf, MAXBUFSIZE, 0, (SOCKADDR*)&addrFrom, &len);
		if (SOCKET_ERROR == retvel)
			break;
		char m_SendIPt[16] = { 0 };
		sprintf(m_SendIPt, inet_ntoa(addrFrom.sin_addr));
		if (strcmp(m_SendIPt, ProcessIP))
		{
			memset(RecvBuf, 0, MAXBUFSIZE);
			continue;
		}
		if (strlen(RecvBuf)<30)
		{
			memset(RecvBuf, 0, MAXBUFSIZE);
			continue;
		}
		sprintf(m_SendIP, inet_ntoa(addrFrom.sin_addr));
		m_SendPort = ntohs(addrFrom.sin_port);
		UA_Msg uas_recvtemp;
		strcpy(uas_recvtemp.data, RecvBuf);

		//WaitForSingleObject(hMutex_uas,INFINITE);
		EnterCriticalSection(&g_uas);
		uas_recvqueue.push(uas_recvtemp);
		//ReleaseMutex(hMutex_uas);
		LeaveCriticalSection(&g_uas);
		//::PostMessage(hwnd,WM_RECVDATA,0,(LPARAM)RecvBuf);

		memset(RecvBuf, 0, MAXBUFSIZE);
	}
	return 0;
}

DWORD WINAPI DispatchRecvMsg(LPVOID lpParameter)
{
	while (TRUE)
	{
		//while (uas_recvqueue.empty()==TRUE){}	
		if (uas_recvqueue.empty() == TRUE)
		{
			Sleep(10);
		}
		else
		{
			//WaitForSingleObject(hMutex_uas,INFINITE);
			EnterCriticalSection(&g_uas);
			uas_curqueue = uas_recvqueue.front();
			uas_recvqueue.pop();
			//ReleaseMutex(hMutex_uas);
			LeaveCriticalSection(&g_uas);
			Sleep(100);
			char RecvBuf[MAXBUFSIZE];
			memset(RecvBuf, 0, MAXBUFSIZE);
			strcpy(RecvBuf, uas_curqueue.data);
			HWND   hMainWnd = ::FindWindow(NULL, _T("UAS"));
			::PostMessage(hMainWnd, WM_RECVDATA, 0, (LPARAM)RecvBuf);
		}
	}
	return 0;
}

DWORD WINAPI SendMsg(LPVOID)
{
	while (TRUE)
	{
		//while (uas_sendqueue.empty()==TRUE){	}		
		if (uas_sendqueue.empty() == TRUE)
		{
			Sleep(10);
		}
		else
		{
			//WaitForSingleObject(hMutex_uas,INFINITE);
			EnterCriticalSection(&g_uas);
			uas_curSendMsg = uas_sendqueue.front();
			uas_sendqueue.pop();
			//ReleaseMutex(hMutex_uas);
			LeaveCriticalSection(&g_uas);
			Sleep(100);
			char SendBuf[MAXBUFSIZE];
			memset(SendBuf, 0, MAXBUFSIZE);
			strcpy(SendBuf, uas_curSendMsg.data);
			HWND   hnd = ::FindWindow(NULL, _T("UAS"));
			::PostMessage(hnd, WM_SENDDATA, 0, (LPARAM)SendBuf);
		}
	}
	return 0;
}

LRESULT CUASDlg::SendMsgData(WPARAM wParm, LPARAM lParam)
{
	//WaitForSingleObject(hMutex_uas,INFINITE);
	EnterCriticalSection(&g_uas);
	SendData((char *)lParam);
	//ReleaseMutex(hMutex_uas);
	LeaveCriticalSection(&g_uas);
	return NULL;
}

LRESULT CUASDlg::RecvData(WPARAM wParm, LPARAM lParam)
{
	USES_CONVERSION;
	CString str = "";
	str = A2T((char*)lParam);
	CSipMsgProcess *sip;
	sip = new CSipMsgProcess;
	int len = str.GetLength();
	//WaitForSingleObject(hMutex_uas,INFINITE);	
	EnterCriticalSection(&g_uas);
	if (bRevMsg)
	{
		int nflag = sip->SipParser((char*)lParam, len);
		//ReleaseMutex(hMutex_uas);	
		LeaveCriticalSection(&g_uas);
		switch (nflag)
		{
		case 0:
		{
			ShowRecvData(str);
			str.ReleaseBuffer();
		}
		break;
		case 1:
		{
			ShowRecvData(str);
			str.ReleaseBuffer();
			MessageBox("报告解析出错", "UAS 错误", MB_OK | MB_ICONERROR);
			bRevMsg = FALSE;
		}
		break;
		default:
			break;
		}
	}
	return NULL;
}

void CUASDlg::SendData(char* data)
{
	if (bUDPSipConnect)
	{
		EnterCriticalSection(&g_uas);
		m_InfoClient.IP = m_SendIP;
		char port[10];
		itoa(m_SendPort, port, 10);
		m_InfoClient.Port = port;
		LeaveCriticalSection(&g_uas);
		GetDlgItem(IDC_STR_REMOTE_IP)->SetWindowText(m_InfoClient.IP);
		GetDlgItem(IDC_STR_REMOTE_PORT)->SetWindowText(m_InfoClient.Port);
		bUDPSipConnect = FALSE;
	}
	int nClientPort = atoi(m_InfoClient.Port);
	DWORD DWIP;
	DWIP = ntohl(inet_addr(m_InfoClient.IP));
	SOCKADDR_IN  addrto;
	addrto.sin_family = AF_INET;
	addrto.sin_port = htons(nClientPort);
	addrto.sin_addr.S_un.S_addr = htonl(DWIP);
	if (data != NULL)
	{
		sendto(m_socket, data, strlen(data) + 1, 0, (SOCKADDR*)&addrto, sizeof(SOCKADDR));
		string st = data;
		int index = st.find("KeepAlive");
		if (index == string::npos || m_bIsShowKeepAliveMsg)
		{
			ShowSendData(data);
		}
	}
}

void CUASDlg::OnBnClickedBtnOpenSip()
{
	if (bNetSet)
	{
		int n = atoi(m_InfoServer.Port);
		int nflag = -1;
		nflag = InitSocket(n);
		if (nflag != 0)
		{
			return;
		}
		CString cstrUser, cstrPassword;
		GetDlgItemText(IDC_USERNAMES, cstrUser);
		GetDlgItemText(IDC_PASSWORDS, cstrPassword);

		GetDlgItem(IDC_USERNAMES)->EnableWindow(FALSE);
		GetDlgItem(IDC_PASSWORDS)->EnableWindow(FALSE);
		g_authInfo.username = cstrUser;
		g_authInfo.password = cstrPassword;
		g_authInfo.uri = "sip:" + m_InfoServer.IP + ":" + m_InfoServer.Port;
		uas_msg_log.open("uas_msg.log");
		CSipMsgProcess *Sip = new CSipMsgProcess;
		contact = new char[100];
		Sip->CopyContact(&contact, m_InfoServer);
		RECVPARAM *pRecvParam = new RECVPARAM;
		pRecvParam->sock = m_socket;
		pRecvParam->hwnd = m_hWnd;
		InitializeCriticalSection(&g_uas);
		// 创建了三个线程 接收 转发 发送
		h_UAS_Recv = CreateThread(NULL, 0, RecvMsg, (LPVOID)pRecvParam, 0, NULL);
		h_UAS_Dispatch = CreateThread(NULL, 0, DispatchRecvMsg, NULL, 0, NULL);
		h_UAS_Send = CreateThread(NULL, 0, SendMsg, NULL, 0, NULL);
		//hMutex_uas=CreateMutex(NULL,FALSE,NULL);		
		//Sleep(500);
		//CloseHandle(hThread);
		ShowSendData("\t----UDP communication is listening----\r\n");
		//开启SIP按钮不可用
		EnableWindow(IDC_BTN_OPEN_SIP, FALSE);
		m_NetSet.m_kAlterBtn.EnableWindow(FALSE);
		bFlag = FALSE;
		SetTimer(4, 4000, NULL);
	}
	else
	{
		MessageBox("请确定网络配置", "UAS 提示", MB_OK | MB_ICONINFORMATION);
	}
}

void CUASDlg::OnBnClickedBtnSendClear()
{
	// TODO: Add your control notification handler code here
	m_ShowSendMsg.SetWindowText("");
}

void CUASDlg::OnBnClickedBtnRecvClear()
{
	// TODO: Add your control notification handler code here
	m_ShowRecvMsg.SetWindowText("");
}

void CUASDlg::OnBnClickedBtnSet()
{
	// TODO: Add your control notification handler code here
	CSetTestMember dlg;
	dlg.DoModal();
}

void CUASDlg::OnBnClickedBtnLog()
{
	// TODO: Add your control notification handler code here
	CShowLog dlg;
	dlg.DoModal();
}

void CUASDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (5 == nIDEvent)
	{
		nTcpCesq++;
		nCurCesq = nTcpCesq;
		CString cesq;
		cesq.Format("%d", nTcpCesq);
		CString options;
		options = "OPTIONS " + rtspUrl + " RTSP/1.0\r\n";
		options += "CSeq: " + cesq + "\r\n\r\n";
		//options+="Session:6310936469860791894\r\n\r\n";
		char *dest = (LPSTR)(LPCTSTR)options;
		SendTCPMsg(dest);
	}
	else
	{
		if (0 /*bACK*/)
		{
			char *data = new char[MAXBUFSIZE];
			memset(data, 0, MAXBUFSIZE);
			CSipMsgProcess *sip;
			sip = new CSipMsgProcess;
			string xml = "<?xml version=\"1.0\"?>\r\n";
			xml += "<Action>\r\n";
			xml += "<Notify>\r\n";
			xml += "<CmdType>RealTimeKeepLive</CmdType>\r\n";
			xml += "</Notify>\r\n";
			xml += "</Action>\r\n";
			char *strxml = new char[XMLSIZE];
			strcpy(strxml, xml.c_str());
			sip->DORealTimeKeepLiveMsg(&data, m_InfoServer, m_InfoClient, inviteAddress, strxml);
			UA_Msg uas_sendtemp;
			strcpy(uas_sendtemp.data, data);
			//WaitForSingleObject(hMutex_uas,INFINITE);
			EnterCriticalSection(&g_uas);
			uas_sendqueue.push(uas_sendtemp);
			//ReleaseMutex(hMutex_uas);	
			LeaveCriticalSection(&g_uas);
			//SendData(data);	
			/*if ( !bShowRealTime)
			{
			ShowSendData(data);
			}*/
			delete strxml;
			strxml = NULL;
			delete data;
			data = NULL;
		}
		//if (bOverTime)
		//{		
		//nTimeCount++;		
		//	if ( nTimeCount==15 )
		//	{
		//		if (bFlag)
		//		{
		//			bFlag=FALSE;			
		//		}
		//		else
		//		{
		//			bOverTime=FALSE;
		//			bSipRegister=FALSE;
		//			bACK=FALSE;
		//			bUDPSipConnect=FALSE;
		//			m_InfoClient.IP="";
		//			m_InfoClient.Port="";
		//			m_InfoClient.UserName="";
		//			m_InfoClient.UserAddress="";
		//			//更新远程配置信息显示
		//			GetDlgItem(IDC_STR_REMOTE_IP)->SetWindowText("未指定");
		//			GetDlgItem(IDC_STR_REMOTE_PORT)->SetWindowText("未指定");
		//			GetDlgItem(IDC_STR_REMOTE_ADD)->SetWindowText("未指定");
		//			GetDlgItem(IDC_STR_REMOTE_NAME)->SetWindowText("未指定");
		//			m_Invite.GetDlgItem(IDC_BTN_TEST)->EnableWindow(FALSE);
		//			m_PTZ.GetDlgItem(IDC_BTN_TEST)->EnableWindow(FALSE);
		//			m_PTZ.GetDlgItem(IDC_BTN_PRE)->EnableWindow(FALSE);	
		//			m_VideoQuery.GetDlgItem(IDC_BTN_QUERY)->EnableWindow(FALSE);
		//			//m_CoderSet.GetDlgItem(IDC_BTN_SET)->EnableWindow(FALSE);
		//			m_Alarm.GetDlgItem(IDC_BTN_ALARM_SET)->EnableWindow(FALSE);
		//			m_Alarm.GetDlgItem(IDC_BTN_TIMESET)->EnableWindow(FALSE);	
		//			MessageBox("客户端已下线","USA 提示",MB_OK|MB_ICONINFORMATION);				
		//		}
		//		nTimeCount=0;		
		//	}	

		//}

	}
	CDialog::OnTimer(nIDEvent);
}

void CUASDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	if (IDYES == MessageBox("确认退出？", NULL, MB_YESNO | MB_ICONINFORMATION))
	{
		if (INVALID_SOCKET != m_socket)
		{
			closesocket(m_socket);
		}
		CloseHandle(h_UAS_Recv);
		CloseHandle(h_UAS_Dispatch);
		CloseHandle(h_UAS_Send);
		uas_msg_log.close();
		KillTimer(4);
		OnCancel();
	}
}

int CUASDlg::AnalyseMsg(char* msg)
{
	string strTemp(msg);
	string temp;
	string dest;
	string::size_type CmdTypeStart;
	if ((CmdTypeStart = strTemp.find("CSeq: 2", 0)) != string::npos &&
		(CmdTypeStart = strTemp.find("Public:", 0)) != string::npos)
	{
		if (!brtspKeeplive)
		{
			dest = "DESCRIBE ";
			dest += rtspUrl;
			dest += " RTSP/1.0\r\n";
			dest += "User-Agent: VLC Media player\r\n";
			dest += "Accept: application/sdp\r\n";
			nTcpCesq++;
			nCurCesq = nTcpCesq;
			CString cesq;
			cesq.Format("%d", nTcpCesq);
			dest += "CSeq: " + cesq + "\r\n\r\n";
			SendTCPMsg(dest.c_str());
			//ShowSendData(dest.c_str());
			brtspKeeplive = TRUE;
		}
		else
		{
			return 0;
		}
	}
	else if ((CmdTypeStart = strTemp.find("CSeq: 3", 0)) != string::npos)
	{
		CString strdestIP;
		CString strdestPort;
		m_VideoPlay.GetDlgItem(IDC_EDT_DESTINATION)->GetWindowText(strdestIP);
		m_VideoPlay.GetDlgItem(IDC_EDT_CLIENTPORT)->GetWindowText(strdestPort);
		dest = "SETUP ";
		dest += rtspUrl;
		dest += " RTSP/1.0\r\n";
		nTcpCesq++;
		nCurCesq = nTcpCesq;
		CString cesq;
		cesq.Format("%d", nTcpCesq);
		dest += "CSeq: " + cesq + "\r\n";
		dest += "Transport: RTP/AVP/UDP;unicast;destination=" + strdestIP + ";client_port=" + strdestPort + ";";
		dest += "ssrc=0\r\n\r\n";
		SendTCPMsg(dest.c_str());
		//ShowSendData(dest.c_str());
	}
	else if ((CmdTypeStart = strTemp.find("CSeq: 4", 0)) != string::npos)
	{
		dest = "PLAY ";
		dest += rtspUrl;
		dest += " RTSP/1.0\r\n";
		nTcpCesq++;
		nCurCesq = nTcpCesq;
		CString cesq;
		cesq.Format("%d", nTcpCesq);
		dest += "CSeq: " + cesq + "\r\n";
		dest += "User-Agent: VLC Media player\r\n";
		string temp1;
		string::size_type CmdTypeEnd;
		if ((CmdTypeStart = strTemp.find("Session", CmdTypeStart + 1)) != string::npos)
			if ((CmdTypeEnd = strTemp.find("\r\n", CmdTypeStart + 1)) != string::npos)
			{
				temp1 = strTemp.substr(CmdTypeStart + 8, CmdTypeEnd - CmdTypeStart - 5);
				bSeesion = TRUE;
				char* str = new char[100];
				strcpy(str, temp1.c_str());
				int nlength = strlen(str);
				char *dst = new char[100];
				int j = 0;
				for (int i = 0; i<nlength; i++)
				{
					if ((str[i] != ':') && (str[i] != '_') && (str[i] != '\t') &&
						(str[i] != '\r') && (str[i] != '\n'))
					{
						dst[j] = str[i];
						j++;
					}
					if ((str[i] == '\0') || (str[i] == '\r') || (str[i] == '\n'))
					{
						dst[j] = '\0';
						break;
					}

				}
				strSession.Format("%s", dst);
				delete str;
			}
		if (bSeesion)
		{
			dest += "Session:" + strSession + "\r\n";
		}
		//dest+="\r\n";
		CString str;
		m_VideoPlay.GetDlgItem(IDC_EDT_SCALE)->GetWindowText(str);
		dest += "Range: npt=0.000-\r\n";
		dest += "Scale:" + str + "\r\n";
		SendTCPMsg(dest.c_str());
		SetTimer(5, 5000, NULL);
		bRTSPLIVE = TRUE;
	}
	return 0;
}

LRESULT CUASDlg::OnReceive(WPARAM wparam, LPARAM lparam)
{
	char buf[MAXBUFSIZE];
	memset(buf, 0, MAXBUFSIZE);
	m_TCPSocket.Receive(buf, MAXBUFSIZE);
	int flag = 1;
	flag = AnalyseMsg(buf);
	switch (flag)
	{
	case 0:
	{
		ShowRecvData(buf);
	}
	break;
	case 1:
	{
		AfxMessageBox("信息解析出错", MB_OK | MB_ICONERROR);
		ShowRecvData(buf);
	}
	break;
	default:
		break;
	}
	return NULL;
}

BOOL CUASDlg::TCPConnect(CString TCPIP, CString TCPPort)
{
	////////////////////////TCP communication
	m_TCPSocket.Close();
	if (!m_TCPSocket.Create())
	{
		MessageBox("TCP Socket Create is error", "UAS  Error", MB_OK | MB_ICONERROR);
		m_TCPSocket.Close();
		return FALSE;
	}
	int nTCPPort = atoi(TCPPort);
	if (!m_TCPSocket.Connect(TCPIP, nTCPPort))
	{
		MessageBox("TCP connect is error", "UAS  Error", MB_OK | MB_ICONERROR);
		m_TCPSocket.Close();
		return FALSE;
	}
	ShowSendData("****** TCP connect is successful ******\r\n");
	return TRUE;
}

LRESULT CUASDlg::OnConnect(WPARAM wparam, LPARAM lparam)
{
	m_bConnect = TRUE;
	return NULL;
}

LRESULT CUASDlg::OnServerClose(WPARAM wparam, LPARAM lparam)
{
	if (!m_bConnect) return NULL;
	m_TCPSocket.Close();
	m_bConnect = FALSE;
	return NULL;
}

void CUASDlg::SendTCPMsg(const char *Msg)
{
	m_TCPSocket.SendMsg(Msg);
	ShowSendData(Msg);
}

void CUASDlg::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
	int nCurSel = m_TestMember.GetCurSel();
	strcpy(ProcessIP, ProductTestMember[nCurSel].IP);
}

void CUASDlg::OnStnClickedSabout()
{
	// TODO: 在此添加控件通知处理程序代码
	CAboutDlg dlg;
	dlg.DoModal();
}

void CUASDlg::OnBnClickedCheck1()
{
	m_bIsShowKeepAliveMsg = IsDlgButtonChecked(IDC_CHECK1);
}

void CUASDlg::OnCbnSelchangeComboIp()
{
	// TODO: 在此添加控件通知处理程序代码
	CString selIpStr;
	m_ComboxLocalIP.GetWindowTextA(selIpStr);
	GetDlgItem(IDC_STR_LOCALIP)->SetWindowTextA(selIpStr);
	m_InfoServer.IP = selIpStr;
}

void CUASDlg::OnBnClickedButtonReboot()
{
	char strPath[100];
	GetModuleFileName(NULL, strPath, 100);

	//创建守护进程，在启用新进程成功后发送WM_QUIT消息结束原来的进程；
	STARTUPINFO startInfo;
	PROCESS_INFORMATION processInfo;
	ZeroMemory(&startInfo, sizeof(STARTUPINFO));
	startInfo.cb = sizeof(STARTUPINFO);
	if (CreateProcess(NULL, (LPTSTR)(LPCTSTR)strPath, NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &processInfo))
	{
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
		PostQuitMessage(WM_CLOSE);
	}
}
