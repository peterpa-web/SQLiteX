// DlgExport.cpp: Implementierungsdatei
//

#include "pch.h"
#include "SampleDlgApp.h"
#include "afxdialogex.h"
#include "DlgExport.h"


// CDlgExport-Dialog

IMPLEMENT_DYNAMIC(CDlgExport, CDialogEx)

CDlgExport::CDlgExport(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EXPORT, pParent)
	, m_bCompany(TRUE)
	, m_bEmploye(TRUE)
	, m_strFmt(_T("utf8International"))
	, m_strExt(_T("txt"))
	, m_strSep(_T(";"))
{

}

CDlgExport::~CDlgExport()
{
}

void CDlgExport::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_COMP, m_bCompany);
	DDX_Check(pDX, IDC_CHECK_EMPL, m_bEmploye);
	DDX_CBString(pDX, IDC_COMBO_FMT, m_strFmt);
	DDX_Text(pDX, IDC_EDIT_EXT, m_strExt);
	DDX_Text(pDX, IDC_EDIT_SEP, m_strSep);
	DDX_Control(pDX, IDC_COMBO_FMT, m_comboFmt);
}


BEGIN_MESSAGE_MAP(CDlgExport, CDialogEx)
END_MESSAGE_MAP()


// CDlgExport-Meldungshandler


BOOL CDlgExport::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  hier zusätzliche Initialisierung hinzufügen.
	m_comboFmt.AddString(_T("utf8Native"));
	m_comboFmt.AddString(_T("utf8International"));
	m_comboFmt.AddString(_T("isoGerman"));
	m_comboFmt.AddString(_T("utf8MarkGerman"));

	return TRUE;  // return TRUE unless you set the focus to a control
}
