#pragma once
#include "afxdialogex.h"


// CDlgEmploye-Dialog

class CDlgEmploye : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgEmploye)

public:
	CDlgEmploye(CWnd* pParent = nullptr);   // Standardkonstruktor
	virtual ~CDlgEmploye();

// Dialogfelddaten
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EMPL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

	DECLARE_MESSAGE_MAP()
public:
//	CEdit m_editFirstName;
//	CEdit m_editBirthday;
//	CEdit m_editSalary;
	CString m_strFirstName;
	CString m_strBirthday;
	int m_nCompId;
	double m_dSalary;
};
