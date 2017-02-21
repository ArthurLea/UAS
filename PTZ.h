#pragma once
#include "afxwin.h"


// CPTZ dialog

class CPTZ : public CDialog
{
	DECLARE_DYNAMIC(CPTZ)

public:
	CPTZ(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPTZ();

// Dialog Data
	enum { IDD = IDD_DLG_PTZ };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnTest();
	afx_msg void OnBnClickedBtnPre();
	CComboBox m_selAddress;
	afx_msg void OnCbnSelchangeSeladress();
	afx_msg void OnBnClickedPtzup();
	afx_msg void OnBnClickedPtzleft();
	afx_msg void OnBnClickedPtzright();
	afx_msg void OnBnClickedPztdown();
	afx_msg void OnBnClickedPtzup2();
	afx_msg void OnBnClickedPtzup3();
	afx_msg void OnBnClickedPtzup4();
	afx_msg void OnBnClickedPtzup5();
	afx_msg void OnBnClickedPtzup6();
	afx_msg void OnBnClickedPtzup7();
};
