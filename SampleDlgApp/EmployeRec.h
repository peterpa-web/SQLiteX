#pragma once
#include "SQLiteRecordset.h"

class CEmployeRec :
    public CSQLiteRecordset
{
public:
	CEmployeRec(CSQLiteDatabase* pDatabase = nullptr);

// Field/Param Data
	long m_EmployeID = 0;
	CStringW m_FirstName;
	COleDateTime m_Birthday;
	long m_CompID = 0;
	CEuro m_Salary;

public:
	virtual CString GetDefaultSQL();    // Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);  // RFX support
};

