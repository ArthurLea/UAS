#pragma once


// CDlgEqualLevel 对话框

class CDlgEqualLevel : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgEqualLevel)

public:
	CDlgEqualLevel(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgEqualLevel();

// 对话框数据
	enum { IDD = IDD_EQUAL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRegsitere();
	afx_msg void OnBnClickedNotifye();
};
