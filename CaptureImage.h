#pragma once
#include "afxwin.h"
#include <vector>
using namespace std;

// CCaptureImage 对话框

class CCaptureImage : public CDialogEx
{
	DECLARE_DYNAMIC(CCaptureImage)

public:
	CCaptureImage(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCaptureImage();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_CAPIMG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	vector<CString>privilegeContent;
	CComboBox m_Privilege;
	vector<CString>capTureTypeContent;
	CComboBox m_CapTureType;
	afx_msg void OnBnClickedButtonDocapture();
	afx_msg void OnCbnSelchangeComboPrivilege();
	afx_msg void OnCbnSelchangeComboCaptype();
};
