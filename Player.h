#pragma once
#include "vlcplay.h"
#include "afxwin.h"
#include "UAS.h"
#include "UASDlg.h"

// CPlayer dialog

class CPlayer : public CDialog
{
	DECLARE_DYNAMIC(CPlayer)

public:
	CPlayer(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPlayer();

// Dialog Data
	enum { IDD = IDD_DLG_PLAYER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CVlcplay m_VLC;
protected:
	virtual void OnOK();
	virtual void OnCancel();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	CStatic m_txt;
	CEdit m_Url;
	CButton m_Play;
	afx_msg void OnBnClickedBtnPlay();
};
