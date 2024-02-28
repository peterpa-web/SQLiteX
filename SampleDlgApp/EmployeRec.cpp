#include "pch.h"
#include "EmployeRec.h"

CEmployeRec::CEmployeRec(CSQLiteDatabase* pdb)
	: CSQLiteRecordset(pdb)
{
	m_strConstraints = L"FOREIGN KEY(CompID) REFERENCES Company(CompID)";
}

CString CEmployeRec::GetDefaultSQL()
{
	return L"Employe";
}

void CEmployeRec::DoFieldExchange(CFieldExchange* pFX)
{
	RFX_Long(pFX, L"EmployeID", m_EmployeID, FX_PK);
    RFX_Text(pFX, L"FirstName", m_FirstName, FX_NN);
    RFX_DateTime(pFX, L"Birthday", m_Birthday);
	RFX_Long(pFX, L"CompID", m_CompID);
    RFX_Euro(pFX, L"Salary", m_Salary);
	// pFX->SetFieldType(CFieldExchange::param);
	// RFX_Text(pFX, L"AAA", m_AAAParam, FX_NN);
}
