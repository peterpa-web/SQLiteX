#pragma once
#include "SQLiteDatabase.h"

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
//	bool IsBOF();								// Begin Of File
	bool IsEOF() const { return m_bEOF; }		// End Of File
	bool IsDeleted() const;
	void Create();
	void ImportTxt();
	void MoveNext();
//	void MovePrev();
//	void MoveFirst();
//	void MoveLast();
	void Edit();
	void AddNew();
	void Update();
	void Delete();
//	int GetRecordCount();
//	void SetFieldDirty(void* pField);
//	void SetAbsolutePosition(int nRecord);

	CString m_strFilter;
	CString m_strSort;

	enum OpenType	// dummy, for compatibility only
	{
		dynaset,        // uses SQLExtendedFetch, keyset driven cursor
		snapshot,       // uses SQLExtendedFetch, static cursor
		forwardOnly,    // uses SQLFetch
		dynamic,        // uses SQLExtendedFetch, dynamic cursor
		readOnly
	};

protected:
	enum class UpdState
	{
		done, addNew, edit
	} m_updState = UpdState::done;

	enum class FX_Task
	{
		none, sqlCreate, sqlInsert, sqlSelect, sqlKey, 
		dataClear, dataRead, dataWrite, dataIdent, dataImport, dataUpdate, dataRowId
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
		void AddSQL(LPCSTR psz);
		void AddSQL(LPCWSTR psz) { AddSQL(ToUtf8(psz)); }
		CStringW NextImportField();

		FX_Task m_task;
		UINT m_nFieldType = noFieldType;
		int m_nField = 0;
		CStringA m_strSQL;
		CStringW m_strImportLine;
		int m_nStartField = 0;	// position in import line
		long m_nRowId = 0;
	};

	virtual CString GetDefaultSQL() = 0;		// Default SQL for Recordset -> table name
	virtual void DoFieldExchange(CFieldExchange* pFX) = 0;
	void RFX_Bool(CFieldExchange* pFX, LPCTSTR szName, BOOL& value, DWORD dwFlags = 0);
	void RFX_Long(CFieldExchange* pFX, LPCTSTR szName, long& value, DWORD dwFlags = 0);
//	void RFX_Int(CFieldExchange* pFX, LPCTSTR szName, int& value, DWORD dwFlags = 0);
	void RFX_Text(CFieldExchange* pFX, LPCTSTR szName, CStringW& value, DWORD dwFlags = 0);
	void RFX_Double(CFieldExchange* pFX, LPCTSTR szName, double& value, DWORD dwFlags = 0);
	void RFX_Date(CFieldExchange* pFX, LPCTSTR szName, CTime& value, DWORD dwFlags = 0);

	CSQLiteDatabase* m_pDB;
	sqlite3_stmt* m_pStmt = nullptr;
	int m_nParams = 0;
	bool m_bEOF = true;

	int m_nFields = 0;			// dummy
	int m_nDefaultType = 0;

private:
	void RFX_Gen(CFieldExchange* pFX, LPCTSTR szName, int nType, DWORD dwFlags);

};

