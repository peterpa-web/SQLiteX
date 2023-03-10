#pragma once
#include "Euro.h"
#include "DateLong.h"
#include "SQLiteDatabase.h"

enum class TxtFmt
{
	standard,			// import only: auto detect format
	utf8Native,			// unformatted table data
	utf8International,	// international time and number format
	isoGerman,			// german time and number format with ISO encoding
	utf8MarkGerman		// german time and number format with UTF8 BOM encoding
};

class CSQLiteRecordset
{
public:
	CSQLiteRecordset(CSQLiteDatabase* pdb);
	~CSQLiteRecordset();
	virtual bool Open(LPCWSTR lpszSQL = nullptr);
	bool OpenRow(long nRow);
	virtual void Close();
	bool Requery();
	bool IsOpen() const;
	bool IsEOF() const { return m_bEOF; }		// End Of File
	bool IsDeleted() const;
	void Create();
	void Import(TxtFmt fmt = TxtFmt::standard, LPCTSTR pszExt = _T("txt"), char cSep = ';');
	void Export(TxtFmt fmt, LPCTSTR pszExt = _T("txt"), char cSep = ';');		// expects Open()
	void MoveNext();
	void Edit();
	void AddNew();
	void Update();
	void Delete();
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
		readOnly	// is a view, no table
	};

protected:
	enum class RecState
	{
		done, open, addNew, edit
	} m_recState = RecState::done;

	enum class FX_Task
	{
		colNamesTypeForCreate,		// list of col names with type
		colNamesForInsert,			// list of col names except pk
		colNamesForSelect,			// list of col names
		colVarsForImport,			// list of all col vars for binding
		colParseBindForImport,		// parse and bind to all colls
		colAllForExport,			// export line from all colls
		colNameValForUpdate,		// list of col name = value pairs except pk
		valClearAll,				// clear all values
		valReadAll,					// read all colls to values
		valStringForInsert,			// list values except pk
		pkName,						// pk or _rowid_
		pkString,					// get key value
		pkAfterInsert				// last rowid
	};

	enum FX_Flags	// for create table
	{
		FX_NN = 1,	// not null
		FX_PK = 2,	// primary key
		FX_AN = 4,	// autonumber
		FX_UN = 8	// unique
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
		long m_nRowId = 0;
		TxtFmt m_fmt = TxtFmt::standard;
	};

	virtual CString GetDefaultSQL() = 0;		// Default SQL for Recordset -> table name
	virtual void DoFieldExchange(CFieldExchange* pFX) = 0;
	void RFX_Bool(CFieldExchange* pFX, LPCTSTR szName, BOOL& value, DWORD dwFlags = 0);
	void RFX_Long(CFieldExchange* pFX, LPCTSTR szName, long& value, DWORD dwFlags = 0);
//	void RFX_Int(CFieldExchange* pFX, LPCTSTR szName, int& value, DWORD dwFlags = 0);
	void RFX_Text(CFieldExchange* pFX, LPCTSTR szName, CStringW& value, DWORD dwFlags = 0);
	void RFX_Double(CFieldExchange* pFX, LPCTSTR szName, double& value, DWORD dwFlags = 0);
//	void RFX_Date(CFieldExchange* pFX, LPCTSTR szName, CTime& value, DWORD dwFlags = 0);
	void RFX_Date(CFieldExchange* pFX, LPCTSTR szName, CDateLong& value, DWORD dwFlags = 0);
//	void RFX_Time(CFieldExchange* pFX, LPCTSTR szName, CTime& value, DWORD dwFlags = 0);
//	void RFX_DateTime(CFieldExchange* pFX, LPCTSTR szName, CTime& value, DWORD dwFlags = 0);
	void RFX_Euro(CFieldExchange* pFX, LPCTSTR szName, CEuro& value, DWORD dwFlags = 0);

	CSQLiteDatabase* m_pDB;
	sqlite3_stmt* m_pStmt = nullptr;
	int m_nParams = 0;
	bool m_bEOF = true;

	int m_nFields = 0;			// dummy
	int m_nDefaultType = 0;

private:
	void RFX_Gen(CFieldExchange* pFX, LPCTSTR szName, int nType, DWORD dwFlags);
	BOOL ReadStringA(CStdioFile& f, CStringA& rString);
};

