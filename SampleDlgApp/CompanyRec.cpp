#include "pch.h"
#include "CompanyRec.h"

CCompanyRec::CCompanyRec(CSQLiteDatabase* pdb)
	: CSQLiteRecordset(pdb)
{
	m_strConstraints = L"";
}

CString CCompanyRec::GetDefaultSQL()
{
	return L"Company";
}

void CCompanyRec::DoFieldExchange(CFieldExchange* pFX)
{
	RFX_Long(pFX, L"CompID", m_CompID, FX_PK);
	RFX_Text(pFX, L"CompName", m_CompName, FX_NN);
	// pFX->SetFieldType(CFieldExchange::param);
	// RFX_Text(pFX, L"AAA", m_AAAParam, FX_NN);
}
