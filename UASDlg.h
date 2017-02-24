// UASDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "DXP.h"
#include "ShowLog.h"
#include "SipMsgProcess.h"
#include "MySocket.h"
#include "NetSet.h"
#include "Invite.h"
#include "PTZ.h"
#include "VideoQuery.h"
#include "VideoPlay.h"
#include "Alarm.h"
#include "SetTestMember.h"
#include "CatalogQuery.h"
#include "DeviceInfQuery.h"
#include "FlowQuery.h"
#include "CoderSet.h"
#include "PSTVSetTime.h"
#include "CaptureImage.h"
//�Զ�����Ϣ
#define WM_RECVDATA    (WM_USER+100)
#define  WM_SENDDATA   (WM_USER+101)

// CUASDlg �Ի���
class CUASDlg : public CDialog
{
// ����
public:
	CUASDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_UAS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;
	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:	
	void InitProgram();
	void InitNetSet();
	void InitInvite();
	void InitPTZ();
	void InitHistoryVideoQuery();
	void InitHistoryVideoPlay();
	void InitCoderSet();
	void InitAlarm();
	void InitCatalogQuery();
	void InitDeviceInfQuery();
	void InitFlowQuery();
	void InitEnableWindow();
	int InitSocket(int port);
	int InitSocketEqual(int port);
	BOOL EnableWindow(UINT uID, BOOL bEnable);
	int GetLocalHostName(CString &sHostName);
	int GetLocalIp(const CString &sHostName, CString &sIpAddress);
	void ShowSendData(CString StrSendData);
	void ShowRecvData(CString strRecvData);	
	LRESULT RecvData(WPARAM wParm, LPARAM lParam);	
	LRESULT SendMsgData(WPARAM wParm, LPARAM lParam);
	void SendData(char *data);
	vector <ProductMember> ProductTestMember;
	
	// �����б���ʾ
	CComboBox m_TestMember;
	// ��ǩҳ
	CTabCtrl m_Ctab;
	CNetSet m_NetSet;
	CInvite m_Invite;
	CPTZ m_PTZ;
	CVideoQuery m_VideoQuery;
	CVideoPlay m_VideoPlay;
	CAlarm m_Alarm;
	CCatalogQuery m_CatalogQuery;
	CDeviceInfQuery m_DeviceInfQuery;
	CFlowQuery m_FlowQuery;
	CCoderSet m_CoderSet;
	CPSTVSetTime m_PSTVSetTime;
	CCaptureImage m_CaptureImg;
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnOpenSip();
	afx_msg void OnBnClickedBtnSendClear();
	// ��ʾ������Ϣ
	CEdit m_ShowSendMsg;
	// ��ʾ������Ϣ
	CEdit m_ShowRecvMsg;
	afx_msg void OnBnClickedBtnRecvClear();
	afx_msg void OnBnClickedBtnSet();
	afx_msg void OnBnClickedBtnLog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedCancel();	
	int nTcpCesq;
	int nCurCesq;

public:
		vector<InfoVideo> m_VideoInfo;
		///////////TCP communication	
		LRESULT OnReceive(WPARAM wparam=NULL, LPARAM lparam=NULL);
		LRESULT OnServerClose(WPARAM wparam=NULL, LPARAM lparam=NULL);
		LRESULT OnConnect(WPARAM wparam=NULL, LPARAM lparam=NULL);
		BOOL TCPConnect(CString TCPIP,CString TCPPort);
		int AnalyseMsg(char* msg);
		void SendTCPMsg(const char *Msg);
		CMySocket m_TCPSocket;
		BOOL		m_bConnect;	
		CString rtspUrl;
		CString RTSPIP;
		CString RTSPPort;
		BOOL bRTSPLIVE;	
		BOOL brtspKeeplive;	
		CString inviteAddress;
		CString videoAddress;
		CString encodeAddress;
		CString catalogAddress;
		CString deviceInfAddress;
		CString flowAddress;
		//ʵʱ������ʷ��Ƶ�ط�ѡ�����
		BOOL bSelectKeepLive;
		int nRealtime;
		BOOL bRealTimeKeepLive;
		int nPtz;	
		BOOL bSeesion;
		CString strSession;
		afx_msg void OnCbnSelchangeCombo1();
		afx_msg void OnStnClickedSabout();
		afx_msg void OnBnClickedCheck1();
		BOOL m_bIsShowKeepAliveMsg;
		CString m_CombIP;
		CComboBox m_ComboxLocalIP;
		afx_msg void OnCbnSelchangeComboIp();
		afx_msg void OnBnClickedButtonReboot();
};

