#pragma once
#include "afxdialogex.h"


// CDlgExport-Dialog

class CDlgExport : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgExport)

public:
	CDlgExport(CWnd* pParent = nullptr);   // Standardkonstruktor
	virtual ~CDlgExport();

// Dialogfelddaten
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXPORT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bCompany;
	BOOL m_bEmploye;
	CString m_strFmt;
	CString m_strExt;
	CString m_strSep;
	CComboBox m_comboFmt;
	virtual BOOL OnInitDialog();
};
