// DlgCompany.cpp: Implementierungsdatei
//

#include "pch.h"
#include "SampleDlgApp.h"
#include "afxdialogex.h"
#include "DlgCompany.h"


// CDlgCompany-Dialog

IMPLEMENT_DYNAMIC(CDlgCompany, CDialogEx)

CDlgCompany::CDlgCompany(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COMP, pParent)
	, m_strCompName(_T(""))
{

}

CDlgCompany::~CDlgCompany()
{
}

void CDlgCompany::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Control(pDX, IDC_EDIT_COMP_NAME, m_editCompName);
	DDX_Text(pDX, IDC_EDIT_COMP_NAME, m_strCompName);
}


BEGIN_MESSAGE_MAP(CDlgCompany, CDialogEx)
END_MESSAGE_MAP()


// CDlgCompany-Meldungshandler
