#include "pch.h"
#include "EmployeFull.h"

CEmployeFull::CEmployeFull(CSQLiteDatabase* pdb)
	: CSQLiteRecordset(pdb)
{
	m_nDefaultType = view;
}

CString CEmployeFull::GetDefaultSQL()
{
    return _T("[EmployeFull]");
}

void CEmployeFull::DoFieldExchange(CFieldExchange* pFX)
{
	RFX_Long(pFX, _T("[EmployeID]"), m_EmployeID, FX_PK);
	RFX_Text(pFX, _T("[FirstName]"), m_FirstName);
//	RFX_Date(pFX, _T("[Birthday]"), m_Birthday);
	RFX_DateTime(pFX, _T("[Birthday]"), m_Birthday);
	RFX_Euro(pFX, _T("[Salary]"), m_Salary);
	RFX_Text(pFX, _T("[CompName]"), m_CompName);
}

void CEmployeFull::Create()
{
	CStringA utf8Sql = "CREATE VIEW ";
	CStringA utf8Table = ToUtf8(GetDefaultSQL());
	utf8Sql += utf8Table;
	utf8Sql += _T(" AS SELECT"
		" EmployeID, FirstName, Birthday, Salary, CompName"
		" FROM Employe a"
		" LEFT JOIN Company b ON a.CompId = b.CompId"
		" ORDER BY FirstName");

	try
	{
		m_pDB->ExecuteSQL(utf8Sql);
		CreateIndex();
	}
	catch (CSQLiteException* pe)
	{
		pe->AddContext(GetDefaultSQL());
		throw pe;
	}
}
