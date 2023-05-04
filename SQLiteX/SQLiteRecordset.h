#pragma once
#include "Euro.h"
#include "DateLong.h"
#include "Blob.h"
#include "SQLiteDatabase.h"

enum class TxtFmt
{
	standard,			// import only: auto detect format
	utf8Native,			// unformatted table data
	utf8International,	// international time and number format
	isoGerman,			// german time and number format with ISO encoding
	utf8MarkGerman		// german time and number format with UTF8 BOM encoding
};

enum FX_Flags	// for create table
{
	FX_NN = 1,	// not null
	FX_PK = 2,	// primary key
	FX_AN = 4,	// autonumber
	FX_UN = 8	// unique
};

class CSQLiteRecordset
{
private:
	CSQLiteRecordset() {}	// don't use
public:
	CSQLiteRecordset(CSQLiteDatabase* pdb);
	virtual ~CSQLiteRecordset();
	virtual bool Open(LPCWSTR lpszSQL = nullptr);
	bool OpenRow(long nRowId = 0);
	virtual void Close();
	bool Requery();
	bool IsOpen() const;
	bool IsEOF() const { return m_bEOF; }		// End Of File
	bool IsDeleted() const;
	void Create();
	void Clear();	// reset the column variables
	void Import(TxtFmt fmt = TxtFmt::standard, LPCTSTR pszExt = _T("txt"), char cSep = ';');
	void Export(TxtFmt fmt, LPCTSTR pszExt = _T("txt"), char cSep = ';');		// expects Open()
	void MoveNext();
	void Edit(long nRowId = 0);
	void AddNew();
	void Update();
	void Delete(long nRowId = 0);
	void DeleteAll();
	void Drop();
//	int GetRecordCount();
//	void SetFieldDirty(void* pField);
//	void SetAbsolutePosition(int nRecord);

	CString m_strFilter;
	CString m_strSort;
	CString m_strConstraints;

	enum OpenType
	{
//		dynaset,        // uses SQLExtendedFetch, keyset driven cursor
		snapshot,       // uses SQLExtendedFetch, static cursor
//		forwardOnly,    // uses SQLFetch
//		dynamic,        // uses SQLExtendedFetch, dynamic cursor
		view			// view is readOnly
	};

protected:
	enum class UpdState
	{
		done, addNew, edit
	} m_updState = UpdState::done;

	enum class FX_Task
	{
		colTypesForCreate,			// list of col names with type
		colNames,					// list of col names
		colVarsForInsert,			// list of all col vars for binding
		colBind,					// bind all colls for update
		colNameVarsForUpdate,		// list of col name=? pairs excl. pk
		valClearAll,				// clear all values
		valParseImport,				// parse import strings
		valReadAll,					// read all colls to values
		valToExport,				// list value strings for export
		pkName,						// pk if found
		pkAfterInsert				// assign last rowid
	};

	class CFieldExchange
	{
	public:
		enum FieldType
		{
			noFieldType = -1,
			outputColumn = 0,
			param = 1,			// SQL_PARAM_INPUT,
			inputParam = param,
			outputParam = 2,	// SQL_PARAM_OUTPUT,
			inoutParam = 3		// SQL_PARAM_INPUT_OUTPUT,
		};

		CFieldExchange(FX_Task t) : m_task(t) {}
		void SetFieldType(UINT nFieldType) { m_nFieldType = nFieldType; }
		void AddSQL(LPCSTR psz, char cSep = 0);
		void AddSQL(LPCWSTR psz) { AddSQL(ToUtf8(psz)); }
		CStringA NextImportField();

		FX_Task m_task;
		UINT m_nFieldType = noFieldType;
		int m_nField = 0;
		char m_cSQLSep = ',';
		CStringA m_utf8SQL;
		CStringA m_strImportLine;
		int m_nStartField = 0;	// position in import line
//		long m_nRowId = 0;
		TxtFmt m_fmt = TxtFmt::standard;
		bool m_bSkipPk = false;
	};

	virtual CString GetDefaultSQL() = 0;		// Default SQL for Recordset -> table name
	virtual void DoFieldExchange(CFieldExchange* pFX) = 0;
	void RFX_Bool(CFieldExchange* pFX, LPCTSTR szName, BOOL& value, DWORD dwFlags = 0);
	void RFX_Long(CFieldExchange* pFX, LPCTSTR szName, long& value, DWORD dwFlags = 0);
//	void RFX_Int(CFieldExchange* pFX, LPCTSTR szName, int& value, DWORD dwFlags = 0);
//	void RFX_Int64(CFieldExchange* pFX, LPCTSTR szName, __int64& value, DWORD dwFlags = 0);
	void RFX_Text(CFieldExchange* pFX, LPCTSTR szName, CStringW& value, DWORD dwFlags = 0);
	void RFX_Double(CFieldExchange* pFX, LPCTSTR szName, double& value, DWORD dwFlags = 0);
//	void RFX_Date(CFieldExchange* pFX, LPCTSTR szName, CTime& value, DWORD dwFlags = 0);
	void RFX_Date(CFieldExchange* pFX, LPCTSTR szName, CDateLong& value, DWORD dwFlags = 0);
	void RFX_DateTime(CFieldExchange* pFX, LPCTSTR szName, COleDateTime& value, DWORD dwFlags = 0);
	void RFX_Euro(CFieldExchange* pFX, LPCTSTR szName, CEuro& value, DWORD dwFlags = 0);
	void RFX_Blob(CFieldExchange* pFX, LPCTSTR szName, CBlob& value, DWORD dwFlags = 0);

	CSQLiteDatabase* m_pDB = nullptr;
	sqlite3_stmt* m_pStmtSel = nullptr;
	sqlite3_stmt* m_pStmtUpd = nullptr;
	int m_nParams = 0;
	bool m_bEOF = true;
	long m_nRowId = 0;			// see Edit() & OpenRow()

	int m_nFields = 0;			// dummy
	int m_nDefaultType = snapshot;

	void CloseUpd();

private:
	void RFX_Gen(CFieldExchange* pFX, LPCTSTR szName, int nType, DWORD dwFlags);
	BOOL ReadStringA(CStdioFile& f, CStringA& rString);
};

