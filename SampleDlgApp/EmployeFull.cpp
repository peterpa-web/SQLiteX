#include "pch.h"
#include "EmployeFull.h"

CEmployeFull::CEmployeFull(CSQLiteDatabase* pdb)
	: CSQLiteRecordset(pdb)
{
	m_nDefaultType = view;
	m_strConstraints = L"";
}

CString CEmployeFull::GetDefaultSQL()
{
	return L"EmployeFull";
}

void CEmployeFull::DoFieldExchange(CFieldExchange* pFX)
{
    RFX_Long(pFX, L"EmployeID", m_EmployeID, FX_PK);
	RFX_Text(pFX, L"FirstName", m_FirstName);
    RFX_DateTime(pFX, L"Birthday", m_Birthday);
    RFX_Euro(pFX, L"Salary", m_Salary);
    RFX_Text(pFX, L"CompName", m_CompName);
	// pFX->SetFieldType(CFieldExchange::param);
	// RFX_Text(pFX, L"AAA", m_AAAParam, FX_NN);
}

void CEmployeFull::Create()
{
    CStringA utf8Sql = ToUtf8(L"CREATE VIEW EmployeFull AS SELECT"
        L" EmployeID, FirstName, Birthday, Salary, CompName"
        L" FROM Employe a"
        L" LEFT JOIN Company b ON a.CompId = b.CompId"
        L" ORDER BY FirstName;");

	try	{
		m_pDB->ExecuteSQL(utf8Sql);
		CreateIndex();
	}
	catch (CSQLiteException* pe)
	{
		pe->AddContext(GetDefaultSQL());
		throw pe;
	}
}
