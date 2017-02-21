// Player.cpp : implementation file
//

#include "stdafx.h"
#include "UAS.h"
#include "Player.h"
extern char strPlayUrl[250];


// CPlayer dialog

IMPLEMENT_DYNAMIC(CPlayer, CDialog)

CPlayer::CPlayer(CWnd* pParent /*=NULL*/)
	: CDialog(CPlayer::IDD, pParent)
{

}

CPlayer::~CPlayer()
{
}

void CPlayer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VLCPLAY, m_VLC);
	DDX_Control(pDX, IDC_STR_URL, m_txt);
	DDX_Control(pDX, IDC_EDT_URL, m_Url);
	DDX_Control(pDX, IDC_BTN_PLAY, m_Play);
}


BEGIN_MESSAGE_MAP(CPlayer, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_PLAY, &CPlayer::OnBnClickedBtnPlay)
END_MESSAGE_MAP()


// CPlayer message handlers

void CPlayer::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	/*CDialog::OnOK();
	DestroyWindow();*/
}

void CPlayer::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialog::OnCancel();
	DestroyWindow();
}

BOOL CPlayer::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CRect rcWindow,rcClient;
	rcWindow.left=350;
	rcWindow.top=50;
	rcWindow.right=996;
	rcWindow.bottom=620;
	MoveWindow(&rcWindow);
	GetClientRect(&rcClient);
	rcClient.bottom-=70;
	m_VLC.MoveWindow(&rcClient);
	/////////////
	CRect rect;
	rect.left=rcClient.left+10;
	rect.top=rcClient.bottom+20;
	rect.right=rcClient.left+60;
	rect.bottom=rcClient.bottom+45;
	m_txt.MoveWindow(&rect);
	/////
	rect.left=rect.right;
	rect.top=rcClient.bottom+20;
	rect.right=rcClient.right-80;
	rect.bottom=rcClient.bottom+45;
	m_Url.MoveWindow(&rect);
	rect.left=rect.right+10;	
	rect.top=rcClient.bottom+20;
	rect.right=rcClient.right-10;
	rect.bottom=rcClient.bottom+45;
	m_Play.MoveWindow(&rect);
	HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);
	GetDlgItem(IDC_EDT_URL)->SetWindowText(strPlayUrl);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPlayer::OnDestroy()
{
	CDialog::OnDestroy();
	delete this;
	// TODO: Add your message handler code here
}

void CPlayer::OnBnClickedBtnPlay()
{
	// TODO: Add your control notification handler code here
	CString urlpath="";
	GetDlgItem(IDC_EDT_URL)->GetWindowText(urlpath);
	if (urlpath=="")
	{
		AfxMessageBox("input url can't be null",MB_OK|MB_ICONERROR);
		return;
	}
	VARIANT a ;
	m_VLC.stop() ;
	m_VLC.playlistClear() ;
	VariantInit(&a) ;
	m_VLC.addTarget(urlpath, a, 0x002+0x004,-666);
	m_VLC.play();
}
