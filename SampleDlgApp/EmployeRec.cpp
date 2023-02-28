#include "pch.h"
#include "EmployeRec.h"

CEmployeRec::CEmployeRec(CSQLiteDatabase* pdb)
	: CSQLiteRecordset(pdb)
{
	m_strConstraints = _T("FOREIGN KEY (CompID) REFERENCES Company(CompID)");
}


CString CEmployeRec::GetDefaultSQL()
{
	return _T("[Employe]");
}

void CEmployeRec::DoFieldExchange(CFieldExchange* pFX)
{
	RFX_Long(pFX, _T("[EmployeID]"), m_EmployeID, FX_PK);
	RFX_Text(pFX, _T("[FirstName]"), m_FirstName);
	RFX_Date(pFX, _T("[Birthday]"), m_Birthday);
	RFX_Long(pFX, _T("[CompID]"), m_CompID, FX_NN);
	RFX_Euro(pFX, _T("[Salary]"), m_Salary);
}
