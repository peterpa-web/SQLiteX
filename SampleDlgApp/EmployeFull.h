#pragma once
#include "SQLiteRecordset.h"

class CEmployeFull :
    public CSQLiteRecordset
{
public:
	CEmployeFull(CSQLiteDatabase* pDatabase = nullptr);

// Field/Param Data
	long	m_EmployeID = 0;
	CString	m_FirstName;
	CTime	m_Birthday;
	CEuro	m_Salary;
	CString	m_CompName;

public:
	virtual CString GetDefaultSQL();		// Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);  // RFX support
	void Create();
};

