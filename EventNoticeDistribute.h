#pragma once


// CEventNoticeDistribute 对话框

class CEventNoticeDistribute : public CDialogEx
{
	DECLARE_DYNAMIC(CEventNoticeDistribute)

public:
	CEventNoticeDistribute(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CEventNoticeDistribute();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIG_ALARMEVENT_NOTICE_DISTRIBUTE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonNoticeDistribute();
	virtual BOOL OnInitDialog();
	CComboBox m_selAddress;
	afx_msg void OnCbnSelchangeSeladress();
};
