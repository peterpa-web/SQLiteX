#pragma once
#include "afxdialogex.h"


// CDlgCompany-Dialog

class CDlgCompany : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCompany)

public:
	CDlgCompany(CWnd* pParent = nullptr);   // Standardkonstruktor
	virtual ~CDlgCompany();

// Dialogfelddaten
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COMP };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

	DECLARE_MESSAGE_MAP()
public:
//	CEdit m_editCompName;
	CString m_strCompName;
};
