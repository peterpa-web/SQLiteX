#pragma once
#include "SQLiteRecordset.h"
#include "SQLiteTypes.h"

class CSQLiteField
{
public:
	CString m_SqlName;
	CString m_SqlTypeRaw;
	int m_nSqlType = 0;
	int m_nFktType = -1;
	DWORD m_dwFlags = 0;
	CString m_strVarName;

	void SetDefaultType();
	void SetFlags(const CString& strFlags);
	CString GetSqlTypeString() { return CSQLiteTypes::GetSqlType(m_nSqlType); }
	CString GetDescr() { return CString(CSQLiteTypes::GetDescr(m_nFktType)); }
	CString GetDef();
	CString GetFunction();
};

class CSQLiteTable
{
public:
	CString m_TblName;
	CString m_ClassName;
	CString m_FileName;
	CList<CSQLiteField> m_fields;

	void ParseFields(const CString& strFields);
	void FillList(CListCtrl& list);
	void GetIncludes(CStringList& list);
	void GetDefs(CStringList& list);
	void GetFunctions(CStringList& list);
};

class CSQLiteSchema
{
public:
	~CSQLiteSchema();
	void ReadAll(const CString& strDbPath);
	CSQLiteTable* GetFirstTable() { 
		m_posT = m_tables.GetHeadPosition(); 
		return GetNextTable(); 
	}
	CSQLiteTable* GetNextTable() { 
		return m_posT == NULL ? NULL : m_tables.GetNext(m_posT); 
	}

protected:
	CTypedPtrList<CPtrList, CSQLiteTable*> m_tables;
	POSITION m_posT = NULL;

	void ResetTables();
};

class CSQLiteSchemaInt :
    public CSQLiteRecordset
{
public:
	CSQLiteSchemaInt(CSQLiteDatabase* pDatabase = nullptr);

	CString	m_Type;
	CString m_Name;
	CString m_TblName;
	long m_RootPage = 0;
	CString m_Sql;

public:
	virtual CString GetDefaultSQL();    // Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);  // RFX support

};

