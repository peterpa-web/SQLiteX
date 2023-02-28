
// SampleDlg.cpp: Implementierungsdatei
//

#include "pch.h"
#include "framework.h"
#include "SampleDlgApp.h"
#include "SampleDlg.h"
#include "afxdialogex.h"
#include "CompanyRec.h"
#include "EmployeRec.h"
#include "EmployeFull.h"
#include "DlgCompany.h"
#include "DlgEmploye.h"

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


// CSampleDlg-Dialogfeld



CSampleDlg::CSampleDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SAMPLEDLGAPP, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_strDataDir = _T("..\\SampleDlgApp\\data");	// containing test.db3 and import files
}

void CSampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_COMP, m_listComp);
	DDX_Control(pDX, IDC_LIST_EMPL, m_listEmpl);
	DDX_Control(pDX, IDC_LIST_EMPL_FULL, m_listEmplFull);
}

BEGIN_MESSAGE_MAP(CSampleDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CREATE, &CSampleDlg::OnBnClickedCreate)
	ON_BN_CLICKED(IDC_IMPORT, &CSampleDlg::OnBnClickedImport)
	ON_BN_CLICKED(IDC_ADD_COMP, &CSampleDlg::OnBnClickedAddComp)
	ON_BN_CLICKED(IDC_EDIT_COMP, &CSampleDlg::OnBnClickedEditComp)
	ON_BN_CLICKED(IDC_DEL_COMP, &CSampleDlg::OnBnClickedDelComp)
	ON_BN_CLICKED(IDC_ADD_EMPL, &CSampleDlg::OnBnClickedAddEmpl)
	ON_BN_CLICKED(IDC_EDIT_EMPL, &CSampleDlg::OnBnClickedEditEmpl)
	ON_BN_CLICKED(IDC_DEL_EMPL, &CSampleDlg::OnBnClickedDelEmpl)
END_MESSAGE_MAP()


// CSampleDlg-Meldungshandler

BOOL CSampleDlg::OnInitDialog()
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
	SetIcon(m_hIcon, TRUE);			// Großes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden

	// TODO: Hier zusätzliche Initialisierung einfügen

    m_listComp.SetExtendedStyle( LVS_EX_FULLROWSELECT );
    m_listComp.InsertColumn(0, _T("Id"), LVCFMT_RIGHT, 30);
    m_listComp.InsertColumn(1, _T("Name"), LVCFMT_LEFT, 185, 1 );
    
    m_listEmpl.SetExtendedStyle( LVS_EX_FULLROWSELECT );
	m_listEmpl.InsertColumn(0, _T("Id"), LVCFMT_RIGHT, 30);
	m_listEmpl.InsertColumn(1, _T("First Name"), LVCFMT_LEFT, 100, 1 );
	m_listEmpl.InsertColumn(2, _T("Birthday"), LVCFMT_LEFT, 80, 2 );
	m_listEmpl.InsertColumn(3, _T("C-Id"), LVCFMT_RIGHT, 40, 3 );
	m_listEmpl.InsertColumn(4, _T("Salary"), LVCFMT_RIGHT, 60, 4 );
    
	m_listEmplFull.SetExtendedStyle( LVS_EX_FULLROWSELECT );
	m_listEmplFull.InsertColumn(0, _T("Id"), LVCFMT_RIGHT, 30);
	m_listEmplFull.InsertColumn(1, _T("First Name"), LVCFMT_LEFT, 100, 1);
	m_listEmplFull.InsertColumn(2, _T("Birthday"), LVCFMT_LEFT, 80, 2);
	m_listEmplFull.InsertColumn(3, _T("Company"), LVCFMT_LEFT, 185, 3);
	m_listEmplFull.InsertColumn(4, _T("Salary"), LVCFMT_RIGHT, 60, 4);

	m_strDbPath = m_strDataDir + _T("\\test.db3");
	CFileStatus fs;
	if (CFile::GetStatus(m_strDbPath, fs) && (fs.m_attribute & CFile::directory) == 0)
	{
		m_db.Open(m_strDbPath);
		FillListComp();
		FillListEmpl();
		FillListEmplFull();
	}

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void CSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSampleDlg::OnPaint()
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

// Das System ruft diese Funktion auf, um den Cursor abzufragen, der angezeigt wird, während der Benutzer
//  das minimierte Fenster mit der Maus zieht.
HCURSOR CSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSampleDlg::FillListComp()
{
	m_listComp.DeleteAllItems();
	CCompanyRec cr(&m_db);
	cr.Open();
	int nItem = 0;
	while (!cr.IsEOF())
	{
		CString strId;
		strId.Format(_T("%d"), cr.m_CompID);
		m_listComp.InsertItem(nItem, strId, 0);
		m_listComp.SetItemData(nItem, cr.m_CompID);
		m_listComp.SetItemText(nItem, 1, cr.m_CompName);
		cr.MoveNext();
		nItem++;
	}
}

void CSampleDlg::FillListEmpl()
{
	m_listEmpl.DeleteAllItems();
	CEmployeRec er(&m_db);
	er.Open();
	int nItem = 0;
	while (!er.IsEOF())
	{
		CString strId;
		strId.Format(_T("%d"), er.m_EmployeID);
		m_listEmpl.InsertItem(nItem, strId, 0);
		m_listEmpl.SetItemData(nItem, er.m_EmployeID);
		m_listEmpl.SetItemText(nItem, 1, er.m_FirstName);
		m_listEmpl.SetItemText(nItem, 2, er.m_Birthday.Format("%d.%m.%Y"));
		strId.Format(_T("%d"), er.m_CompID);
		m_listEmpl.SetItemText(nItem, 3, strId);
		m_listEmpl.SetItemText(nItem, 4, er.m_Salary.ToString());
		er.MoveNext();
		nItem++;
	}
}

void CSampleDlg::FillListEmplFull()
{
	m_listEmplFull.DeleteAllItems();
	CEmployeFull ef(&m_db);
	ef.Open();
	int nItem = 0;
	while (!ef.IsEOF())
	{
		CString strId;
		strId.Format(_T("%d"), ef.m_EmployeID);
		m_listEmplFull.InsertItem(nItem, strId, 0);
		m_listEmplFull.SetItemData(nItem, ef.m_EmployeID);
		m_listEmplFull.SetItemText(nItem, 1, ef.m_FirstName);
		m_listEmplFull.SetItemText(nItem, 2, ef.m_Birthday.Format("%d.%m.%Y"));
		m_listEmplFull.SetItemText(nItem, 3, ef.m_CompName);
		m_listEmplFull.SetItemText(nItem, 4, ef.m_Salary.ToString());
		ef.MoveNext();
		nItem++;
	}
}



void CSampleDlg::OnBnClickedCreate()
{
	m_db.Close();
	DeleteFile(m_strDbPath);
	m_db.Open(m_strDbPath);
	m_db.ExecuteSQL("PRAGMA foreign_keys = ON;");

	CCompanyRec c(&m_db);
	c.Create();
	FillListComp();

	CEmployeRec e(&m_db);
	e.Create();
	FillListEmpl();

	CEmployeFull f(&m_db);
	f.Create();
	FillListEmplFull();
}


void CSampleDlg::OnBnClickedImport()
{
	m_db.BeginTrans();
	try
	{
		CCompanyRec c(&m_db);
		c.ImportTxt();
		FillListComp();

		CEmployeRec e(&m_db);
		e.ImportTxt();
		FillListEmpl();
		FillListEmplFull();
	}
	catch (CSQLiteException* pe)
	{
		m_db.Rollback();
		throw pe;
	}
	m_db.CommitTrans();
}


void CSampleDlg::OnBnClickedAddComp()
{
	CDlgCompany dlg(this);
	int nRc = dlg.DoModal();
	if (nRc != IDOK)
		return;

	CCompanyRec cr(&m_db);
	cr.AddNew();
	cr.m_CompName = dlg.m_strCompName;
	cr.Update();

	FillListComp();
}


void CSampleDlg::OnBnClickedEditComp()
{
	int nSel = m_listComp.GetSelectionMark();
	if (nSel < 0)
		return;

	long nId = m_listComp.GetItemData(nSel);
	CCompanyRec cr(&m_db);
	cr.OpenRow(nId);

	CDlgCompany dlg(this);
	dlg.m_strCompName = cr.m_CompName;
	int nRc = dlg.DoModal();
	if (nRc != IDOK)
		return;

	cr.Edit();
	cr.m_CompName = dlg.m_strCompName;
	cr.Update();

	FillListComp();
	FillListEmplFull();
}


void CSampleDlg::OnBnClickedDelComp()
{
	int nSel = m_listComp.GetSelectionMark();
	if (nSel < 0)
		return;

	long nId = m_listComp.GetItemData(nSel);
	CCompanyRec cr(&m_db);
	cr.m_CompID = nId;
	cr.Delete();

	FillListComp();
	FillListEmplFull();
}


void CSampleDlg::OnBnClickedAddEmpl()
{
	CDlgEmploye dlg(this);
	int nRc = dlg.DoModal();
	if (nRc != IDOK)
		return;

	CEmployeRec er(&m_db);
	er.AddNew();
	er.m_FirstName = dlg.m_strFirstName;
	CStringA strData(dlg.m_strBirthday);
	int p = 0;
	CStringA s = strData.Tokenize(".", p);
	int d = atoi(s);
	s = strData.Tokenize(".", p);
	int m = atoi(s);
	s = strData.Tokenize(" ", p);
	int y = atoi(s);
	er.m_Birthday = CTime(y, m, d, 0, 0, 0);
	er.m_CompID = dlg.m_nCompId;
	er.m_Salary = dlg.m_dSalary;
	er.Update();

	FillListEmpl();
	FillListEmplFull();
}


void CSampleDlg::OnBnClickedEditEmpl()
{
	int nSel = m_listEmpl.GetSelectionMark();
	if (nSel < 0)
		return;

	long nId = m_listEmpl.GetItemData(nSel);
	CEmployeRec er(&m_db);
	er.OpenRow(nId);

	CDlgEmploye dlg(this);
	dlg.m_strFirstName = er.m_FirstName;
	dlg.m_strBirthday = er.m_Birthday.Format("%d.%m.%Y");
	dlg.m_nCompId = er.m_CompID;
	dlg.m_dSalary = er.m_Salary.ToDouble();
	int nRc = dlg.DoModal();
	if (nRc != IDOK)
		return;

	er.Edit();
	er.m_FirstName = dlg.m_strFirstName;
	CStringA strData(dlg.m_strBirthday);
	int p = 0;
	CStringA s = strData.Tokenize(".", p);
	int d = atoi(s);
	s = strData.Tokenize(".", p);
	int m = atoi(s);
	s = strData.Tokenize(" ", p);
	int y = atoi(s);
	er.m_Birthday = CTime(y, m, d, 0, 0, 0);
	er.m_CompID = dlg.m_nCompId;
	er.m_Salary = dlg.m_dSalary;
	er.Update();

	FillListEmpl();
	FillListEmplFull();
}


void CSampleDlg::OnBnClickedDelEmpl()
{
	int nSel = m_listEmpl.GetSelectionMark();
	if (nSel < 0)
		return;

	long nId = m_listEmpl.GetItemData(nSel);
	CEmployeRec er(&m_db);
	er.m_EmployeID = nId;
	er.Delete();

	FillListEmpl();
	FillListEmplFull();
}
