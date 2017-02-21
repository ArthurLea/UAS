#pragma once


// CVideoPlay dialog

class CVideoPlay : public CDialog
{
	DECLARE_DYNAMIC(CVideoPlay)

public:
	CVideoPlay(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoPlay();

// Dialog Data
	enum { IDD = IDD_DLG_VIDEOPLAY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedBtnPlay();
	afx_msg void OnBnClickedBtnStop();
};
