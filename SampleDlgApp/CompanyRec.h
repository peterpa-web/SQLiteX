#pragma once
#include "SQLiteRecordset.h"

class CCompanyRec :
    public CSQLiteRecordset
{
public:
	CCompanyRec(CSQLiteDatabase* pDatabase = nullptr);
	long	m_CompID = 0;
	CString	m_CompName;

public:
	virtual CString GetDefaultSQL();    // Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);  // RFX support
};

