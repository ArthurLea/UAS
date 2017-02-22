#pragma once
#include "afxwin.h"


// CFlowQuery 对话框

class CFlowQuery : public CDialogEx
{
	DECLARE_DYNAMIC(CFlowQuery)

public:
	CFlowQuery(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFlowQuery();

// 对话框数据
	enum { IDD = IDD_DLG_FLOWQUERY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedFlowquery();
	CComboBox m_selAddress;
	afx_msg void OnCbnSelchangeSeladress();
	CEdit m_CatQueAddress;
	CEdit m_FlowQueryInfo;
};
