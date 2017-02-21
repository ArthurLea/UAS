// ShowLog.cpp : implementation file
//

#include "stdafx.h"
#include "UAS.h"
#include "ShowLog.h"
#include "UASDlg.h"

//成员变量
extern CString ShowTestTitle;
extern CString ShowTestData;
// CShowLog dialog

IMPLEMENT_DYNAMIC(CShowLog, CDialog)

CShowLog::CShowLog(CWnd* pParent /*=NULL*/)
	: CDialog(CShowLog::IDD, pParent)
{

}

CShowLog::~CShowLog()
{
}

void CShowLog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_REFRESH, m_focus);
}


BEGIN_MESSAGE_MAP(CShowLog, CDialog)
	ON_BN_CLICKED(IDC_BTN_REFRESH, &CShowLog::OnBnClickedBtnRefresh)
END_MESSAGE_MAP()


// CShowLog message handlers

void CShowLog::OnBnClickedBtnRefresh()
{
	// TODO: Add your control notification handler code here	
	/*HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);	*/	
	GetDlgItem(IDC_EDT_LOG)->SetWindowText(ShowTestData);
	SetWindowText(ShowTestTitle);
	m_focus.SetFocus();
}

BOOL CShowLog::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO:  Add extra initialization here
	/*HWND   hnd=::FindWindow(NULL, _T("UAS"));	
	CUASDlg*  pWnd= (CUASDlg*)CWnd::FromHandle(hnd);	*/	
	GetDlgItem(IDC_EDT_LOG)->SetWindowText(ShowTestData);
	SetWindowText(ShowTestTitle);
	m_focus.SetFocus();
	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
