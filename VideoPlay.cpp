// VideoPlay.cpp : implementation file
//

#include "stdafx.h"
#include "UAS.h"
#include "VideoPlay.h"
#include "UASDlg.h"

extern SHELLEXECUTEINFO vlc;
// CVideoPlay dialog

IMPLEMENT_DYNAMIC(CVideoPlay, CDialog)

CVideoPlay::CVideoPlay(CWnd* pParent /*=NULL*/)
	: CDialog(CVideoPlay::IDD, pParent)
{

}

CVideoPlay::~CVideoPlay()
{
}

void CVideoPlay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CVideoPlay, CDialog)
	ON_BN_CLICKED(IDC_BTN_START, &CVideoPlay::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_PLAY, &CVideoPlay::OnBnClickedBtnPlay)
	ON_BN_CLICKED(IDC_BTN_STOP, &CVideoPlay::OnBnClickedBtnStop)
END_MESSAGE_MAP()


// CVideoPlay message handlers

void CVideoPlay::OnBnClickedBtnStart()
{
	// TODO: Add your control notification handler code here
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	CString strURL="";
	GetDlgItem(IDC_EDT_URL)->GetWindowText(strURL);
	pWnd->rtspUrl=strURL;
	GetDlgItem(IDC_EDT_IP)->GetWindowText(pWnd->RTSPIP);
	GetDlgItem(IDC_EDT_PORT)->GetWindowText(pWnd->RTSPPort);
	if ( !pWnd->TCPConnect(pWnd->RTSPIP,pWnd->RTSPPort) )
	{
		MessageBox("TCP连接失败","UAS 提示",MB_OK|MB_ICONINFORMATION);		
		return;
	}
	pWnd->nTcpCesq=2;
	pWnd->nCurCesq=pWnd->nTcpCesq;
	CString cesq;
	cesq.Format("%d",pWnd->nTcpCesq);
	CString options;
	options="OPTIONS "+pWnd->rtspUrl+" RTSP/1.0\r\n";
	options+="CSeq: "+cesq+"\r\n";
	options+="User-Agent: VLC Media player\r\n\r\n";
	//options+="Session: 6310936469860791894\r\n\r\n";
	char *dest = (LPSTR)(LPCTSTR)options;
	pWnd->SendTCPMsg(dest);
	//pWnd->ShowSendData(dest);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_PLAY)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_PLAY)->ShowWindow(TRUE);
	pWnd->brtspKeeplive=FALSE;
	pWnd->bSelectKeepLive=FALSE;
	pWnd->m_Invite.GetDlgItem(IDC_BTN_TEST)->EnableWindow(FALSE);
	pWnd->m_Invite.GetDlgItem(IDC_BTN_BYE)->EnableWindow(FALSE);
	//water：增加打开vlc的代码
	string ipaddr;
	int i, j;
	ipaddr = pWnd->rtspUrl;
	i = ipaddr.find('/', 0);
	j = ipaddr.find(':', i + 2);
	if (j == string::npos)
	{
		j = ipaddr.find('/', i + 2);
	}
	ipaddr = ipaddr.substr(i + 2, j - i - 2);
	string m_vlc_sdp_str;
	m_vlc_sdp_str += "m=video ";
	m_vlc_sdp_str += "4588 ";
	m_vlc_sdp_str += "RTP/AVP 96\r";
	m_vlc_sdp_str += "a=rtpmap:96 H264/90000\r";
	m_vlc_sdp_str += "a=framerate:25\r";
	m_vlc_sdp_str += "c=IN IP4 ";
	m_vlc_sdp_str += ipaddr;
	m_vlc_sdp_str += "\r";
	FILE *sdp;
	sdp = fopen(".//VLC//start.sdp", "wt+");
	fprintf(sdp, "%s", m_vlc_sdp_str.c_str());//这里不能调用messagebox，就先不判断了
	fclose(sdp);
	ZeroMemory(&vlc, sizeof(vlc));
	vlc.cbSize = sizeof(vlc);
	vlc.fMask = SEE_MASK_NOCLOSEPROCESS;
	vlc.hwnd = NULL;
	vlc.lpVerb = NULL;
	vlc.lpFile = "vlc.exe";
	vlc.lpParameters = "start.sdp";
	vlc.lpDirectory = ".\\vlc\\";
	vlc.nShow = SW_SHOW;
	vlc.hInstApp = NULL;
	//ShellExecute(NULL, "open", "keppAliveSendTest.exe", NULL, ".\\vlc\\", SW_SHOWNORMAL);
	ShellExecuteEx(&vlc);
	//water:end
}

/*播放过程中更改播放速率*/
void CVideoPlay::OnBnClickedBtnPlay()
{
	// TODO: Add your control notification handler code here
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	CString Play;
	CString strRange;
	CString strScale;
	GetDlgItem(IDC_EDT_RANGE)->GetWindowText(strRange);
	GetDlgItem(IDC_EDT_SCALE)->GetWindowText(strScale);
	Play="PLAY "+pWnd->rtspUrl+" RTSP/1.0\r\n";
	pWnd->nTcpCesq++;
	pWnd->nCurCesq=pWnd->nTcpCesq;
	CString cesq;
	cesq.Format("%d",pWnd->nTcpCesq);
	Play+="CSeq: "+cesq+"\r\n";
	if (pWnd->bSeesion)
	{
		Play+="Session:"+pWnd->strSession+"\r\n";			
	}	
	//Play+="Range: "+strRange+"\r\n";
	Play+="Scale: "+strScale+"\r\n";
	//Play += "\r\n\r\n";
	Play+="\r\n";
	char *dest = (LPSTR)(LPCTSTR)Play;
	pWnd->SendTCPMsg(dest);
	//pWnd->ShowSendData(dest);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
	//water：增加打开vlc的代码
	//string ipaddr;
	//int i, j;
	//ipaddr = pWnd->rtspUrl;
	//i = ipaddr.find('/', 0);
	//j = ipaddr.find(':', i + 2);
	//if (j == string::npos)
	//{
	//	j = ipaddr.find('/', i + 2);

	//}
	//ipaddr = ipaddr.substr(i + 2, j - i - 2);
	//string m_vlc_sdp_str;
	//m_vlc_sdp_str += "m=video ";
	//m_vlc_sdp_str += "4588 ";
	//m_vlc_sdp_str += "RTP/AVP 96\r";
	//m_vlc_sdp_str += "a=rtpmap:96 H264/90000\r";
	//m_vlc_sdp_str += "a=framerate:25\r";
	//m_vlc_sdp_str += "c=IN IP4 ";
	//m_vlc_sdp_str += ipaddr;
	//m_vlc_sdp_str += "\r";
	//FILE *sdp;
	//sdp = fopen(".//VLC//start.sdp", "wt+");
	//fprintf(sdp, "%s", m_vlc_sdp_str.c_str());//这里不能调用messagebox，就先不判断了
	//fclose(sdp);
	//system(".\\VLC\\vlc.exe .\\VLC\\start.sdp");
	//water:end
}

void CVideoPlay::OnBnClickedBtnStop()
{
	// TODO: Add your control notification handler code here
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);	
	CString stop;	
	pWnd->nTcpCesq++;
	pWnd->nCurCesq=pWnd->nTcpCesq;
	CString cesq;
	cesq.Format("%d",pWnd->nTcpCesq);
	CString options;
	stop="TEARDOWN "+pWnd->rtspUrl+" RTSP/1.0\r\n";
	stop+="CSeq: "+cesq+"\r\n";
	if (pWnd->bSeesion)
	{
		stop+="Session:"+pWnd->strSession+"\r\n\r\n";			
	}			
	char *dest = (LPSTR)(LPCTSTR)stop;
	if (!pWnd->KillTimer(5))
	{
		AfxMessageBox("心跳计时器已关闭",MB_OK);
	}
	
	pWnd->SendTCPMsg(dest);
	
	//pWnd->ShowSendData(dest);
	GetDlgItem(IDC_BTN_PLAY)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_START)->EnableWindow(TRUE);
	pWnd->bRTSPLIVE=FALSE;
	pWnd->bSelectKeepLive=TRUE;
	pWnd->m_Invite.GetDlgItem(IDC_BTN_TEST)->EnableWindow(TRUE);
}
