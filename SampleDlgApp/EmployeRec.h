#pragma once
#include "SQLiteRecordset.h"

class CEmployeRec :
    public CSQLiteRecordset
{
public:
	CEmployeRec(CSQLiteDatabase* pDatabase = nullptr);
	long	m_EmployeID = 0;
	CString	m_FirstName;
//	CDateLong	m_Birthday;
	COleDateTime	m_Birthday;
	long	m_CompID = 0;
	CEuro	m_Salary;

public:
	virtual CString GetDefaultSQL();    // Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);  // RFX support
};

