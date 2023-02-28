// DlgEmploye.cpp: Implementierungsdatei
//

#include "pch.h"
#include "SampleDlgApp.h"
#include "afxdialogex.h"
#include "DlgEmploye.h"


// CDlgEmploye-Dialog

IMPLEMENT_DYNAMIC(CDlgEmploye, CDialogEx)

CDlgEmploye::CDlgEmploye(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EMPL, pParent)
	, m_strFirstName(_T(""))
	, m_strBirthday(_T(""))
	, m_nCompId(0)
	, m_dSalary(0)
{

}

CDlgEmploye::~CDlgEmploye()
{
}

void CDlgEmploye::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Control(pDX, IDC_EDIT_FIRST_NAME, m_editFirstName);
	//  DDX_Control(pDX, IDC_EDIT_BIRTHDAY, m_editBirthday);
	//  DDX_Control(pDX, IDC_EDIT_SALARY, m_editSalary);
	DDX_Text(pDX, IDC_EDIT_FIRST_NAME, m_strFirstName);
	DDX_Text(pDX, IDC_EDIT_BIRTHDAY, m_strBirthday);
	DDX_Text(pDX, IDC_EDIT_COMP_ID, m_nCompId);
	DDX_Text(pDX, IDC_EDIT_SALARY, m_dSalary);
}


BEGIN_MESSAGE_MAP(CDlgEmploye, CDialogEx)
END_MESSAGE_MAP()


// CDlgEmploye-Meldungshandler
