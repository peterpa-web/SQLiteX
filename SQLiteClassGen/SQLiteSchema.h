#pragma once
#include "SQLiteRecordset.h"
#include "SQLiteTypes.h"

class CSQLiteTableInfo;

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

class CSQLiteIndex
{
public:
	CString m_Name;
	CString m_Sql;
};

class CSQLiteTable
{
public:
	CString m_TblName;
	CString m_ClassName;
	CString m_FileName;
	CList<CSQLiteField> m_fields;
	CString m_strConstraintsQuoted;
	CList<CSQLiteIndex> m_idx;
	bool m_bView = false;
	CString m_Sql;

	void ParseFields(const CString& strFields);
	void AddField(const CSQLiteTableInfo& ti);
	void FillList(CListCtrl& list);
	void GetDefs(CStringList& list);
	void GetFunctions(CStringList& list);
	CString GetConstraintsQuoted() { return m_strConstraintsQuoted; }

protected:
	CString StripDeco(CString strName);
	CSQLiteField* FindField(const CString& strName);
	void AddContraints(CString str);
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

class CSQLiteTableInfo :
    protected CSQLiteRecordset
{
public:
	CSQLiteTableInfo(CSQLiteDatabase* pDatabase = nullptr);

	long m_Cid = 0;
	CString	m_Name;
	CString m_Type;
	BOOL m_NotNull = FALSE;
	CString m_Dflt_Value;
	BOOL m_Pk = FALSE;

public:
	virtual CString GetDefaultSQL();    // Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);  // RFX support

	void OpenTable(const CString& strTblName) { Open(L"PRAGMA table_info(" + strTblName + L");"); }
	bool IsEOF() const { return m_bEOF; }		// End Of File
	void MoveNext() { CSQLiteRecordset::MoveNext(); }
};

