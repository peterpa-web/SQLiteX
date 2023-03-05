
// SampleDlg.h: Headerdatei
//

#pragma once
#include "SQLiteDatabase.h"


// CSampleDlg-Dialogfeld
class CSampleDlg : public CDialogEx
{
// Konstruktion
public:
	CSampleDlg(CWnd* pParent = nullptr);	// Standardkonstruktor

// Dialogfelddaten
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SAMPLEDLGAPP };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung


// Implementierung
protected:
	HICON m_hIcon;
	CSQLiteDatabase m_db;

	// Generierte Funktionen für die Meldungstabellen
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:
	void FillListComp();
	void FillListEmpl();
	void FillListEmplFull();

public:
	CListCtrl m_listComp;
	CListCtrl m_listEmpl;
	CListCtrl m_listEmplFull;
	CString m_strDataDir;
	CString m_strExpDir;
	CString m_strDbPath;

	afx_msg void OnBnClickedCreate();
	afx_msg void OnBnClickedImport();
	afx_msg void OnBnClickedExport();
	afx_msg void OnBnClickedAddComp();
	afx_msg void OnBnClickedEditComp();
	afx_msg void OnBnClickedDelComp();
	afx_msg void OnBnClickedAddEmpl();
	afx_msg void OnBnClickedEditEmpl();
	afx_msg void OnBnClickedDelEmpl();
};
