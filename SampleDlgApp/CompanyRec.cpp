#include "pch.h"
#include "CompanyRec.h"

CCompanyRec::CCompanyRec(CSQLiteDatabase* pdb)
	: CSQLiteRecordset(pdb)
{
}


CString CCompanyRec::GetDefaultSQL()
{
	return _T("[Company]");
}

void CCompanyRec::DoFieldExchange(CFieldExchange* pFX)
{
	RFX_Long(pFX, _T("[CompID]"), m_CompID, FX_PK);
	RFX_Text(pFX, _T("[CompName]"), m_CompName);
}
