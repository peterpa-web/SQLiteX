
// SQLiteClassGenDlg.h: Headerdatei
//

#pragma once
#include "SQLiteSchema.h"


// CSQLiteClassGenDlg-Dialogfeld
class CSQLiteClassGenDlg : public CDialogEx
{
// Konstruktion
public:
	CSQLiteClassGenDlg(CWnd* pParent = nullptr);	// Standardkonstruktor

// Dialogfelddaten
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SQLITECLASSGEN_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung


// Implementierung
protected:
	HICON m_hIcon;
	CSQLiteSchema m_schema;
	CSQLiteTable* m_pTable = nullptr;
	CSQLiteField* m_pField = nullptr;
	int m_nFktType = -1;
	CString m_strTargetPath;
	int m_nTableFocus = -1;
	int m_nFieldFocus = -1;

	void ResetTableData();
	void ResetFieldData();
	void ShowTables();
	void ShowFields();
	void ShowFlags(DWORD dwFlags, DWORD dwSupportedFlags);
	void WriteHeaderFile(CSQLiteTable* pTable);
	void WriteClassFile(CSQLiteTable* pTable);

	// Generierte Funktionen für die Meldungstabellen
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editDbPath;
	CEdit m_editTargetPath;
	CEdit m_editClassName;
	CEdit m_editFileName;
	CEdit m_editVarName;
	CListCtrl m_listTables;
	CListCtrl m_listFields;
	CComboBox m_comboFieldType;
	CButton m_checkNN;
	CButton m_checkPK;
	CButton m_checkAN;
	CButton m_checkUN;
	CButton m_buttonCreateFiles;
	afx_msg void OnClickedButtonOpenDb();
	afx_msg void OnClickedButtonSelTargetPath();
	afx_msg void OnClickedButtonCreateFiles();
	afx_msg void OnClickedButtonRemoveClass();
	afx_msg void OnEnChangeEditClassName();
	afx_msg void OnCbnSelchangeComboFieldType();
	afx_msg void OnBnClickedCheckNN();
	afx_msg void OnBnClickedCheckPK();
	afx_msg void OnBnClickedCheckAN();
	afx_msg void OnBnClickedCheckUN();
	afx_msg void OnEnChangeEditVarName();
	afx_msg void OnEnChangeEditFileName();
	afx_msg void OnLvnItemchangedListTables(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemchangedListFields(NMHDR* pNMHDR, LRESULT* pResult);
};
