#pragma once
#include "afxwin.h"


// CFlowQuery �Ի���

class CFlowQuery : public CDialogEx
{
	DECLARE_DYNAMIC(CFlowQuery)

public:
	CFlowQuery(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CFlowQuery();

// �Ի�������
	enum { IDD = IDD_DLG_FLOWQUERY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedFlowquery();
	CComboBox m_selAddress;
	afx_msg void OnCbnSelchangeSeladress();
	CEdit m_CatQueAddress;
	CEdit m_FlowQueryInfo;
};
