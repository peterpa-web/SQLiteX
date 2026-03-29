
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
	DDX_Control(pDX, IDC_LIST_TYPE, m_listType);
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
	ON_LBN_SELCHANGE(IDC_LIST_TYPE, &CSQLiteClassGenDlg::OnLbnSelchangeListType)
	ON_BN_CLICKED(IDC_CHECK_NN, &CSQLiteClassGenDlg::OnBnClickedCheckNN)
	ON_BN_CLICKED(IDC_CHECK_PK, &CSQLiteClassGenDlg::OnBnClickedCheckPK)
	ON_BN_CLICKED(IDC_CHECK_AN, &CSQLiteClassGenDlg::OnBnClickedCheckAN)
	ON_BN_CLICKED(IDC_CHECK_UN, &CSQLiteClassGenDlg::OnBnClickedCheckUN)
	ON_EN_CHANGE(IDC_EDIT_VAR_NAME, &CSQLiteClassGenDlg::OnEnChangeEditVarName)
	ON_EN_CHANGE(IDC_EDIT_FILE_NAME, &CSQLiteClassGenDlg::OnEnChangeEditFileName)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_TABLES, &CSQLiteClassGenDlg::OnLvnItemchangedListTables)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_FIELDS, &CSQLiteClassGenDlg::OnLvnItemchangedListFields)
	ON_BN_CLICKED(IDC_BUTTON_DERIVE_CLASS, &CSQLiteClassGenDlg::OnBnClickedDeriveClass)
END_MESSAGE_MAP()


// CSQLiteClassGenDlg-Meldungshandler

void CSQLiteClassGenDlg::ResetTableView()
{
	m_pTable = nullptr;
	m_editClassName.SetWindowText(L"");
	m_editFileName.SetWindowText(L"");
	m_listFields.DeleteAllItems();
	ResetFieldView();
}

void CSQLiteClassGenDlg::ResetFieldView()
{
	m_listType.ResetContent();
	m_editVarName.SetWindowText(L"");
	m_pField = nullptr;
	m_nFktType = -1;
	m_nFieldFocus = -1;
	ShowFlags(0, 0);
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
	m_listTables.InsertColumn(1, _T("Klasse"), LVCFMT_LEFT, 195, 1);

	m_listFields.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_listFields.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 170);
	m_listFields.InsertColumn(1, _T("SQL"), LVCFMT_LEFT, 120, 1);
	m_listFields.InsertColumn(2, _T("CPP"), LVCFMT_LEFT, 120, 2);

	ShowFlags(0, 0);
	m_buttonCreateFiles.EnableWindow(FALSE);

	m_editDbPath.SetWindowText(m_strDbPath);
	if (m_schema.ReadAll(m_strDbPath))
	{
		ShowTables();
	}
	m_editTargetPath.SetWindowText(m_strTargetPath);
	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void CSQLiteClassGenDlg::ShowTables()
{
	m_listTables.DeleteAllItems();
	ResetTableView();
	CSQLiteTable* pT = m_schema.GetFirstTable();
	int i = 0;
	while (pT != nullptr)
	{
		int n = m_listTables.InsertItem(i, pT->m_TblName);
		ShowClassName(pT, i);
		m_listTables.SetItemData(i++, (DWORD_PTR)pT);
		pT = m_schema.GetNextTable();
	}
	m_buttonCreateFiles.EnableWindow(!m_strTargetPath.IsEmpty());
	m_listType.EnableWindow(!m_strTargetPath.IsEmpty());
	m_editVarName.EnableWindow(!m_strTargetPath.IsEmpty());
}

void CSQLiteClassGenDlg::ShowClassName(CSQLiteTable* pT, int nItem)
{
	CWnd* pButton = GetDlgItem(IDC_BUTTON_DERIVE_CLASS);
	pButton->EnableWindow(FALSE);
	CString strClassName = pT->m_ClassName;
	if (!m_strTargetPath.IsEmpty() && !strClassName.IsEmpty())
	{
		if (!pT->GetOldDefs(m_strTargetPath))
		{
			// reload table, ShowFields
		}
		strClassName = pT->m_ClassName;		// may be updated
		if (pT->IsModified())
			strClassName = L"* " + strClassName;
		else
		{
			if (pT->m_OldClass.HasDerived())
				strClassName += L" (2)";
			else
				pButton->EnableWindow();
		}
	}
	m_editClassName.EnableWindow(!strClassName.IsEmpty());
	m_editFileName.EnableWindow(!strClassName.IsEmpty());
	m_listTables.SetItemText(nItem, 1, strClassName);
}

void CSQLiteClassGenDlg::ShowFields()
{
	m_pTable->FillList(m_listFields);
	ResetFieldView();
}

void CSQLiteClassGenDlg::ShowFieldModif()
{
	CString s1 = m_pField->m_SqlTypeRaw + ' ' + m_pField->GetFlagsShort();
	if (m_pField->m_dwModified != 0)
		s1 = L"* " + s1;
	m_listFields.SetItemText(m_nFieldFocus, 1, s1);
	ShowClassName(m_pTable, m_nTableFocus);
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
			s.Replace(L"\n", L" \\\n\t");
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

void CSQLiteClassGenDlg::WriteHeaderFile2(CSQLiteTable* pTable)
{
	CString strPath = m_strTargetPath + L"\\" + pTable->m_FileName + L"2.h";
	CStdioFile file(strPath, CFile::modeWrite | CFile::modeCreate);
	file.WriteString(L"#pragma once\n");
	file.WriteString(L"#include \"" + pTable->m_FileName + L".h\"\n\n");
	file.WriteString(L"class " + pTable->m_ClassName + L"2 :\n"
		L"\tprotected " + pTable->m_ClassName + L"\n"
		L"{\n");
	file.WriteString(L"public:\n"
		L"\t" + pTable->m_ClassName + L"2(CSQLiteDatabase* pDatabase = nullptr);\n");
	file.WriteString(L"};\n");
}

void CSQLiteClassGenDlg::WriteClassFile2(CSQLiteTable* pTable)
{
	CString strPath = m_strTargetPath + L"\\" + pTable->m_FileName + L"2.cpp";
	CStdioFile file(strPath, CFile::modeWrite | CFile::modeCreate);
	file.WriteString(L"#include \"pch.h\"\n"
		L"#include \"" + pTable->m_FileName + L"2.h\"\n\n");
	file.WriteString(pTable->m_ClassName + L"2::" + pTable->m_ClassName + L"2(CSQLiteDatabase* pdb)\n"
		L"\t: " + pTable->m_ClassName + L"(pdb)\n{\n");
	file.WriteString(L"}\n\n");
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

	m_strDbPath = dlg.GetPathName();
	m_editDbPath.SetWindowText(m_strDbPath);
	if (m_schema.ReadAll(m_strDbPath))
	{
		ShowTables();
	}
}


void CSQLiteClassGenDlg::OnClickedButtonSelTargetPath()
{
	CFolderPickerDialog dlg(m_strTargetPath);
	if (dlg.DoModal() == IDOK)
	{
		m_strTargetPath = dlg.GetPathName();
		m_editTargetPath.SetWindowText(m_strTargetPath);
		if (m_schema.ReadAll(m_strDbPath))
		{
			ShowTables();
			m_listType.EnableWindow();
			m_editVarName.EnableWindow();
		}
	}
//	m_buttonCreateFiles.EnableWindow(m_schema.GetFirstTable() != nullptr);
}


void CSQLiteClassGenDlg::OnClickedButtonCreateFiles()
{
	CSQLiteTable* pT = m_schema.GetFirstTable();
	int n = 0;
	while (pT != nullptr)
	{
		if (!pT->m_ClassName.IsEmpty() && pT->IsModified())
		{
			WriteHeaderFile(pT);
			WriteClassFile(pT);
			++n;
		}
		pT = m_schema.GetNextTable();
	}
	CString s;
	s.Format(L"Fertig: %d Klassen erstellt.", n);
	MessageBox(s);
	if (m_schema.ReadAll(m_strDbPath))
	{
		ShowTables();
		m_listType.EnableWindow();
		m_editVarName.EnableWindow();
	}
}


void CSQLiteClassGenDlg::OnClickedButtonRemoveClass()
{
	if (m_pTable == nullptr)
		return;
	CWnd* pButton = GetDlgItem(IDC_BUTTON_REMOVE_CLASS);
	if (m_pTable->m_ClassName.IsEmpty())
	{
		m_pTable->SetDefClassName();
		pButton->SetWindowText(L"Entfernen");
	}
	else
	{
		m_pTable->m_ClassName.Empty();
		m_pTable->m_FileName.Empty();
		m_pTable->ResetFields();
		m_pTable->m_OldClass.Reset();
		pButton->SetWindowText(L"Aktivieren");
	}
	m_editClassName.SetWindowText(m_pTable->m_ClassName);
	m_editFileName.SetWindowText(m_pTable->m_FileName);
	m_pTable->m_dwModified = 0;	// was always set before
	ShowClassName(m_pTable, m_nTableFocus);
	ShowFields();
}


void CSQLiteClassGenDlg::OnEnChangeEditClassName()
{
	if (m_pTable == nullptr)
		return;

	TRACE0("OnEnChangeEditClassName\n");
	m_editClassName.GetWindowText(m_pTable->m_ClassName);
	m_pTable->m_FileName = m_pTable->m_ClassName.Mid(1);
	m_editFileName.SetWindowText(m_pTable->m_FileName);
	if (m_pTable->m_ClassName != m_pTable->m_OldClass.GetOldClassName())
		m_pTable->m_dwModified |= m_pTable->MOD_CLASS;
	else
		m_pTable->m_dwModified &= ~m_pTable->MOD_CLASS;
//	ShowClassName(m_pTable, m_nTableFocus);
}


void CSQLiteClassGenDlg::OnEnChangeEditFileName()
{
	if (m_pTable == nullptr)
		return;
	TRACE0("OnEnChangeEditFileName\n");
	m_editFileName.GetWindowText(m_pTable->m_FileName);
	if (m_pTable->m_FileName != m_pTable->m_OldClass.GetOldFileName())
	{
		m_pTable->m_dwModified |= m_pTable->MOD_FILE;
		m_pTable->m_OldClass.Reset();
	}
	ShowClassName(m_pTable, m_nTableFocus);
}


void CSQLiteClassGenDlg::OnLbnSelchangeListType()
{
	int n = m_listType.GetCurSel();
	int nType = m_listType.GetItemData(n);
	m_nFktType = m_pField->m_nFktType = nType;
	if (nType != m_pField->m_nFktTypeOld)
		m_pField->m_dwModified |= m_pField->MOD_TYPE;
	else
		m_pField->m_dwModified &= ~m_pField->MOD_TYPE;

//	DWORD dwSupported = CSQLiteTypes::GetFlags(m_nFktType);
//	if (m_pTable->m_bView)
//		dwSupported &= FX_NN;
//	ShowFlags(m_pField->m_dwFlags, dwSupported);
	ShowFlags(m_pField->m_dwFlags, 0);
	m_listFields.SetItemText(m_nFieldFocus, 2, m_pField->GetDescr());
	ShowFieldModif();
}


void CSQLiteClassGenDlg::OnBnClickedCheckNN()
{
	m_pField->m_dwFlags ^= FX_NN;
	m_checkNN.SetCheck((m_pField->m_dwFlags & FX_NN) != 0 ? BST_CHECKED : BST_UNCHECKED);
	ShowFieldModif();
}


void CSQLiteClassGenDlg::OnBnClickedCheckPK()
{
	m_pField->m_dwFlags ^= FX_PK;
	m_checkPK.SetCheck((m_pField->m_dwFlags & FX_PK) != 0 ? BST_CHECKED : BST_UNCHECKED);
	ShowFieldModif();
}


void CSQLiteClassGenDlg::OnBnClickedCheckAN()
{
	m_pField->m_dwFlags ^= FX_AN;
	m_checkAN.SetCheck((m_pField->m_dwFlags & FX_AN) != 0 ? BST_CHECKED : BST_UNCHECKED);
	ShowFieldModif();
}


void CSQLiteClassGenDlg::OnBnClickedCheckUN()
{
	m_pField->m_dwFlags ^= FX_UN;
	m_checkUN.SetCheck((m_pField->m_dwFlags & FX_UN) != 0 ? BST_CHECKED : BST_UNCHECKED);
	ShowFieldModif();
}


void CSQLiteClassGenDlg::OnEnChangeEditVarName()
{
	if (m_pField == nullptr)
		return;
	CString str;
	m_editVarName.GetWindowText(str);
	if (!str.IsEmpty())
		m_pField->m_strVarName = str;
	if (str != m_pField->m_strVarNameOld)
		m_pField->m_dwModified |= m_pField->MOD_VAR;
	else
		m_pField->m_dwModified &= ~m_pField->MOD_VAR;
	ShowFieldModif();
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
		CWnd* pButton = GetDlgItem(IDC_BUTTON_REMOVE_CLASS);
		if (m_pTable->m_ClassName.IsEmpty())
		{
			pButton->SetWindowText(L"Aktivieren");
		}
		else
		{
			pButton->SetWindowText(L"Entfernen");
		}
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
		CSQLiteTypes::FillList(m_listType, m_pField->m_nSqlType, m_nFktType);
		m_editVarName.SetWindowText(m_pField->m_strVarName);
	//	DWORD dwSupported = CSQLiteTypes::GetFlags(m_nFktType);
	//	if (m_pTable->m_bView)
	//		dwSupported &= FX_NN;
	//	ShowFlags(m_pField->m_dwFlags, dwSupported);
		ShowFlags(m_pField->m_dwFlags, 0);
	}
	*pResult = 0;
}

void CSQLiteClassGenDlg::OnBnClickedDeriveClass()
{
	if (m_pTable == nullptr)
		return;
	WriteHeaderFile2(m_pTable);
	WriteClassFile2(m_pTable);
	m_pTable->SetDerived();
	ShowClassName(m_pTable, m_nTableFocus);
}
