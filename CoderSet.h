#pragma once


// CCoderSet dialog

class CCoderSet : public CDialog
{
	DECLARE_DYNAMIC(CCoderSet)

public:
	CCoderSet(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCoderSet();

// Dialog Data
	enum { IDD = IDD_DLG_CODERSET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnSet();
};
