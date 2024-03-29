
// SQLiteClassGenDlg.cpp: Implementierungsdatei
//

#include "pch.h"
#include "framework.h"
#include "SQLiteClassGen.h"
#include "SQLiteClassGenDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg-Dialogfeld für Anwendungsbefehl "Info"

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialogfelddaten
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

// Implementierung
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSQLiteClassGenDlg-Dialogfeld



CSQLiteClassGenDlg::CSQLiteClassGenDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SQLITECLASSGEN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSQLiteClassGenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_DB_PATH, m_editDbPath);
	DDX_Control(pDX, IDC_EDIT_TARGET_PATH, m_editTargetPath);
	DDX_Control(pDX, IDC_EDIT_CLASS_NAME, m_editClassName);
	DDX_Control(pDX, IDC_EDIT_FILE_NAME, m_editFileName);
	DDX_Control(pDX, IDC_COMBO_FIELD_TYPE, m_comboFieldType);
	DDX_Control(pDX, IDC_LIST_TABLES, m_listTables);
	DDX_Control(pDX, IDC_LIST_FIELDS, m_listFields);
	DDX_Control(pDX, IDC_CHECK_NN, m_checkNN);
	DDX_Control(pDX, IDC_CHECK_PK, m_checkPK);
	DDX_Control(pDX, IDC_CHECK_AN, m_checkAN);
	DDX_Control(pDX, IDC_CHECK_UN, m_checkUN);
	DDX_Control(pDX, IDC_BUTTON_CREATE_FILES, m_buttonCreateFiles);
	DDX_Control(pDX, IDC_EDIT_VAR_NAME, m_editVarName);
}

BEGIN_MESSAGE_MAP(CSQLiteClassGenDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN_DB, &CSQLiteClassGenDlg::OnClickedButtonOpenDb)
	ON_BN_CLICKED(IDC_BUTTON_SEL_TARGET_PATH, &CSQLiteClassGenDlg::OnClickedButtonSelTargetPath)
	ON_BN_CLICKED(IDC_BUTTON_CREATE_FILES, &CSQLiteClassGenDlg::OnClickedButtonCreateFiles)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_CLASS, &CSQLiteClassGenDlg::OnClickedButtonRemoveClass)
	ON_EN_CHANGE(IDC_EDIT_CLASS_NAME, &CSQLiteClassGenDlg::OnEnChangeEditClassName)
	ON_CBN_SELCHANGE(IDC_COMBO_FIELD_TYPE, &CSQLiteClassGenDlg::OnCbnSelchangeComboFieldType)
	ON_BN_CLICKED(IDC_CHECK_NN, &CSQLiteClassGenDlg::OnBnClickedCheckNN)
	ON_BN_CLICKED(IDC_CHECK_PK, &CSQLiteClassGenDlg::OnBnClickedCheckPK)
	ON_BN_CLICKED(IDC_CHECK_AN, &CSQLiteClassGenDlg::OnBnClickedCheckAN)
	ON_BN_CLICKED(IDC_CHECK_UN, &CSQLiteClassGenDlg::OnBnClickedCheckUN)
	ON_EN_CHANGE(IDC_EDIT_VAR_NAME, &CSQLiteClassGenDlg::OnEnChangeEditVarName)
	ON_EN_CHANGE(IDC_EDIT_FILE_NAME, &CSQLiteClassGenDlg::OnEnChangeEditFileName)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_TABLES, &CSQLiteClassGenDlg::OnLvnItemchangedListTables)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_FIELDS, &CSQLiteClassGenDlg::OnLvnItemchangedListFields)
END_MESSAGE_MAP()


// CSQLiteClassGenDlg-Meldungshandler

void CSQLiteClassGenDlg::ResetTableData()
{
	m_pTable = nullptr;
	m_editClassName.SetWindowText(L"");
	m_editFileName.SetWindowText(L"");
	m_listFields.DeleteAllItems();
	ResetFieldData();
}

void CSQLiteClassGenDlg::ResetFieldData()
{
	m_comboFieldType.ResetContent();
	m_editVarName.SetWindowText(L"");
	m_pField = nullptr;
	m_nFktType = -1;
	m_nFieldFocus = -1;
	ShowFlags(0, 0);
}

void CSQLiteClassGenDlg::ShowTables()
{
	m_listTables.DeleteAllItems();
	ResetTableData();
	CSQLiteTable* pT = m_schema.GetFirstTable();
	int i = 0;
	while (pT != nullptr)
	{
		int n = m_listTables.InsertItem(i, pT->m_TblName);
		m_listTables.SetItemText(i, 1, pT->m_ClassName);
		m_listTables.SetItemData(i++, (DWORD_PTR)pT);
		pT = m_schema.GetNextTable();
	}
}

void CSQLiteClassGenDlg::ShowFields()
{
	m_pTable->FillList(m_listFields);
	ResetFieldData();
}

void CSQLiteClassGenDlg::ShowFlags(DWORD dwFlags, DWORD dwSupportedFlags)
{
	m_checkNN.EnableWindow((dwSupportedFlags & FX_NN) != 0 ? 1 : 0);
	m_checkPK.EnableWindow((dwSupportedFlags & FX_PK) != 0 ? 1 : 0);
	m_checkAN.EnableWindow((dwSupportedFlags & FX_AN) != 0 ? 1 : 0);
	m_checkUN.EnableWindow((dwSupportedFlags & FX_UN) != 0 ? 1 : 0);

	m_checkNN.SetCheck((dwFlags & FX_NN) != 0 ? BST_CHECKED : BST_UNCHECKED);
	m_checkPK.SetCheck((dwFlags & FX_PK) != 0 ? BST_CHECKED : BST_UNCHECKED);
	m_checkAN.SetCheck((dwFlags & FX_AN) != 0 ? BST_CHECKED : BST_UNCHECKED);
	m_checkUN.SetCheck((dwFlags & FX_UN) != 0 ? BST_CHECKED : BST_UNCHECKED);
}

void CSQLiteClassGenDlg::WriteHeaderFile(CSQLiteTable* pTable)
{
	CString strPath = m_strTargetPath + L"\\" + pTable->m_FileName + L".h";
	CStdioFile file(strPath, CFile::modeWrite | CFile::modeCreate);
	file.WriteString(L"#pragma once\n");
	file.WriteString(L"#include \"SQLiteRecordset.h\"\n\n");
	file.WriteString(L"class " + pTable->m_ClassName + L" :\n"
		L"\tpublic CSQLiteRecordset\n"
		L"{\n");
	file.WriteString(L"public:\n"
		L"\t" + pTable->m_ClassName + L"(CSQLiteDatabase* pDatabase = nullptr);\n"
		L"\n// Field/Param Data\n");
	// write vars
	CStringList listVars;
	pTable->GetDefs(listVars);
	POSITION pos = listVars.GetHeadPosition();
	while (pos != NULL)
	{
		CString s = listVars.GetNext(pos);
		file.WriteString(L"\t" + s + L"\n");
	}
	file.WriteString(L"\npublic:\n"
		L"\tvirtual CString GetDefaultSQL();    // Default SQL for Recordset\n"
		L"\tvirtual void DoFieldExchange(CFieldExchange* pFX);  // RFX support\n");
	if (pTable->m_idx.GetCount() > 0)
		file.WriteString(L"\tvirtual void CreateIndex() override;\n"
		L"\tvirtual void DropIndex() override;\n");
	if (pTable->m_bView)
		file.WriteString(L"\tvoid Create();\n");
	file.WriteString(L"};\n");
}

void CSQLiteClassGenDlg::WriteClassFile(CSQLiteTable* pTable)
{
	CString strPath = m_strTargetPath + L"\\" + pTable->m_FileName + L".cpp";
	CStdioFile file(strPath, CFile::modeWrite | CFile::modeCreate);
	file.WriteString(L"#include \"pch.h\"\n"
		L"#include \"" + pTable->m_FileName + L".h\"\n\n");
	file.WriteString(pTable->m_ClassName + L"::" + pTable->m_ClassName + L"(CSQLiteDatabase* pdb)\n"
		L"\t: CSQLiteRecordset(pdb)\n{\n");
	if (pTable->m_bView)
		file.WriteString(L"\tm_nDefaultType = view;\n");
	file.WriteString(L"\tm_strConstraints = L\"" + pTable->GetConstraintsQuoted() + L"\";\n");
	file.WriteString(L"}\n\n");
	file.WriteString(L"CString " + pTable->m_ClassName + L"::GetDefaultSQL()\n"
		L"{\n\treturn L\"" + pTable->m_TblName + L"\";\n}\n\n");
	file.WriteString(L"void " + pTable->m_ClassName + L"::DoFieldExchange(CFieldExchange* pFX)\n{\n");
	// write fkts
//		RFX_Long(pFX, _T("[CompID]"), m_CompID, FX_PK);
//		RFX_Text(pFX, _T("[CompName]"), m_CompName, FX_NN);
	CStringList listFkts;
	pTable->GetFunctions(listFkts);
	POSITION pos = listFkts.GetHeadPosition();
	while (pos != NULL)
	{
		CString s = listFkts.GetNext(pos);
		file.WriteString(L"\t" + s + L"\n");
	}

	// write parameters
	file.WriteString(L"\t// pFX->SetFieldType(CFieldExchange::param);\n");
	file.WriteString(L"\t// RFX_Text(pFX, L\"AAA\", m_AAAParam, FX_NN);\n");

	file.WriteString(L"}\n");

	// write indices
	if (pTable->m_idx.GetCount() > 0)
	{
		file.WriteString(L"\nvoid " + pTable->m_ClassName + L"::CreateIndex()\n{\n");
		POSITION pos = pTable->m_idx.GetHeadPosition();
		while (pos != NULL)
		{
			const CSQLiteIndex& si = pTable->m_idx.GetNext(pos);
			CString s = si.m_Sql;
			s.Replace(L"\"", L"\\\"");
			file.WriteString(L"\tm_pDB->ExecuteSQL(L\"" + s + L"\");\n");
		}
		file.WriteString(L"}\n");

		file.WriteString(L"\nvoid " + pTable->m_ClassName + L"::DropIndex()\n{\n");
		pos = pTable->m_idx.GetHeadPosition();
		while (pos != NULL)
		{
			const CSQLiteIndex& si = pTable->m_idx.GetNext(pos);
			file.WriteString(L"\tm_pDB->ExecuteSQL(L\"DROP INDEX IF EXISTS " + si.m_Name + L";\");\n");
		}
		file.WriteString(L"}\n");
	}
	if (pTable->m_bView)
	{
		file.WriteString(L"\nvoid " + pTable->m_ClassName + L"::Create()\n{\n");
		file.WriteString(L"\tCStringA utf8Sql = ToUtf8(L\"" + pTable->m_Sql + L";\");\n");
		file.WriteString(L"\ttry"
			L"\t{\n"
			L"\t\tm_pDB->ExecuteSQL(utf8Sql);\n"
			L"\t\tCreateIndex();\n"
			L"\t}\n"
			L"\tcatch (CSQLiteException* pe)\n"
			L"\t{\n"
			L"\t\tpe->AddContext(GetDefaultSQL());\n"
			L"\t\tthrow pe;\n"
			L"\t}\n"
			L"}\n");
	}
}

BOOL CSQLiteClassGenDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Hinzufügen des Menübefehls "Info..." zum Systemmenü.

	// IDM_ABOUTBOX muss sich im Bereich der Systembefehle befinden.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Symbol für dieses Dialogfeld festlegen.  Wird automatisch erledigt
	//  wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	SetIcon(m_hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden

	// TODO: Hier zusätzliche Initialisierung einfügen

	m_listTables.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_listTables.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 120);
	m_listTables.InsertColumn(1, _T("Klasse"), LVCFMT_LEFT, 125, 1);

	m_listFields.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_listFields.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 120);
	m_listFields.InsertColumn(1, _T("SQL"), LVCFMT_LEFT, 60, 1);
	m_listFields.InsertColumn(2, _T("CPP"), LVCFMT_LEFT, 155, 2);

	ShowFlags(0, 0);
	m_buttonCreateFiles.EnableWindow(FALSE);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void CSQLiteClassGenDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// Wenn Sie dem Dialogfeld eine Schaltfläche "Minimieren" hinzufügen, benötigen Sie
//  den nachstehenden Code, um das Symbol zu zeichnen.  Für MFC-Anwendungen, die das 
//  Dokument/Ansicht-Modell verwenden, wird dies automatisch ausgeführt.

void CSQLiteClassGenDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Gerätekontext zum Zeichnen

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Symbol in Clientrechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Die System ruft diese Funktion auf, um den Cursor abzufragen, der angezeigt wird, während der Benutzer
//  das minimierte Fenster mit der Maus zieht.
HCURSOR CSQLiteClassGenDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSQLiteClassGenDlg::OnClickedButtonOpenDb()
{
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Database Files (.db;.db3)|*.db;*.db3|All Files (*.*)|*.*||"));
	if (dlg.DoModal() != IDOK)
		return;

	m_editDbPath.SetWindowText( dlg.GetPathName());
	m_schema.ReadAll(dlg.GetPathName());
	ShowTables();
	m_buttonCreateFiles.EnableWindow(!m_strTargetPath.IsEmpty());
}


void CSQLiteClassGenDlg::OnClickedButtonSelTargetPath()
{
	CFolderPickerDialog dlg(m_strTargetPath);
	if (dlg.DoModal() == IDOK)
	{
		m_strTargetPath = dlg.GetPathName();
		m_editTargetPath.SetWindowText(m_strTargetPath);
	}
	m_buttonCreateFiles.EnableWindow(m_schema.GetFirstTable() != nullptr);
}


void CSQLiteClassGenDlg::OnClickedButtonCreateFiles()
{
	CSQLiteTable* pT = m_schema.GetFirstTable();
	while (pT != nullptr)
	{
		if (!pT->m_ClassName.IsEmpty())
		{
			WriteHeaderFile(pT);
			WriteClassFile(pT);
		}
		pT = m_schema.GetNextTable();
	}
	MessageBox(L"Dateien fertig erstellt.");
}


void CSQLiteClassGenDlg::OnClickedButtonRemoveClass()
{
	if (m_pTable == nullptr)
		return;
	m_pTable->m_ClassName.Empty();
	m_pTable->m_FileName.Empty();
	m_editClassName.SetWindowText(L"");
	m_editFileName.SetWindowText(L"");
//	int n = m_listTables.GetCurSel();
//	CString str = m_pTable->m_TblName + L" " + m_pTable->m_ClassName;
//	m_listTables.SetDlgItemText(n, str);
}


void CSQLiteClassGenDlg::OnEnChangeEditClassName()
{
	if (m_pTable == nullptr)
		return;

	m_editClassName.GetWindowText(m_pTable->m_ClassName);
	m_pTable->m_FileName = m_pTable->m_ClassName.Mid(1);
	m_editFileName.SetWindowText(m_pTable->m_FileName);
	m_listTables.SetItemText(m_nTableFocus, 1, m_pTable->m_ClassName);
}


void CSQLiteClassGenDlg::OnCbnSelchangeComboFieldType()
{
	int n = m_comboFieldType.GetCurSel();
	m_nFktType = m_pField->m_nFktType = m_comboFieldType.GetItemData(n);
	DWORD dwSupported = CSQLiteTypes::GetFlags(m_nFktType);
	if (m_pTable->m_bView)
		dwSupported &= FX_NN;
	ShowFlags(m_pField->m_dwFlags, dwSupported);
	m_listFields.SetItemText(m_nFieldFocus, 2, m_pField->GetDescr());
}


void CSQLiteClassGenDlg::OnBnClickedCheckNN()
{
	m_pField->m_dwFlags ^= FX_NN;
	m_checkNN.SetCheck((m_pField->m_dwFlags & FX_NN) != 0 ? BST_CHECKED : BST_UNCHECKED);
}


void CSQLiteClassGenDlg::OnBnClickedCheckPK()
{
	m_pField->m_dwFlags ^= FX_PK;
	m_checkNN.SetCheck((m_pField->m_dwFlags & FX_PK) != 0 ? BST_CHECKED : BST_UNCHECKED);
}


void CSQLiteClassGenDlg::OnBnClickedCheckAN()
{
	m_pField->m_dwFlags ^= FX_AN;
	m_checkNN.SetCheck((m_pField->m_dwFlags & FX_AN) != 0 ? BST_CHECKED : BST_UNCHECKED);
}


void CSQLiteClassGenDlg::OnBnClickedCheckUN()
{
	m_pField->m_dwFlags ^= FX_UN;
	m_checkNN.SetCheck((m_pField->m_dwFlags & FX_UN) != 0 ? BST_CHECKED : BST_UNCHECKED);
}


void CSQLiteClassGenDlg::OnEnChangeEditVarName()
{
	if (m_pField == nullptr)
		return;
	CString str;
	m_editVarName.GetWindowText(str);
	if (!str.IsEmpty())
		m_pField->m_strVarName = str;
}


void CSQLiteClassGenDlg::OnEnChangeEditFileName()
{
	if (m_pTable != nullptr)
		m_editFileName.GetWindowText(m_pTable->m_FileName);
}


void CSQLiteClassGenDlg::OnLvnItemchangedListTables(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if ((pNMLV->uOldState & LVIS_FOCUSED) !=
		(pNMLV->uNewState & LVIS_FOCUSED))
	{
		m_nTableFocus = pNMLV->iItem;
		m_pTable = (CSQLiteTable*)m_listTables.GetItemData(pNMLV->iItem);
		m_editClassName.SetWindowText(m_pTable->m_ClassName);
		m_editFileName.SetWindowText(m_pTable->m_FileName);
		ShowFields();
	}
	*pResult = 0;
}


void CSQLiteClassGenDlg::OnLvnItemchangedListFields(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if ((pNMLV->uOldState & LVIS_FOCUSED) !=
		(pNMLV->uNewState & LVIS_FOCUSED))
	{
		m_nFieldFocus = pNMLV->iItem;
		m_pField = (CSQLiteField*)m_listFields.GetItemData(pNMLV->iItem);
		m_nFktType = m_pField->m_nFktType;
		CSQLiteTypes::FillCombo(m_comboFieldType, m_pField->m_nSqlType, m_nFktType);
		m_editVarName.SetWindowText(m_pField->m_strVarName);
		DWORD dwSupported = CSQLiteTypes::GetFlags(m_nFktType);
		if (m_pTable->m_bView)
			dwSupported &= FX_NN;
		ShowFlags(m_pField->m_dwFlags, dwSupported);
	}
	*pResult = 0;
}
