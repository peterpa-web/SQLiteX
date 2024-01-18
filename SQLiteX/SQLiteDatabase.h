#pragma once
#include "SQLite3.h"

// global helpers:
CStringW FromUtf8(LPCSTR pszUtf8);
CStringA ToUtf8(LPCWSTR pszUtf16);

class CSQLiteDatabase
{
public:
	CSQLiteDatabase() {}
	~CSQLiteDatabase();
	virtual bool Open(LPCWSTR lpszFilePath);
	virtual void Close();
	bool IsOpen() { return m_pdb3 != nullptr; }
	sqlite3* GetHandle() { return m_pdb3; }
	void ExecuteSQL(const CStringA& utf8Sql);
	void ExecuteSQL(const CStringW& strSql) { ExecuteSQL(ToUtf8(strSql)); }
	long GetLastRowId();
	bool BeginTrans();
	void CommitTrans();
	void Rollback();
	CStringW GetLastError();
	CString GetImportPath() const { return m_strImportPath; }
	void SetImportPath(const CString& strPath) { m_strImportPath = strPath; }	// see default at Open
	CString GetExportPath() const { return m_strExportPath; }
	void SetExportPath(const CString& strPath) { m_strExportPath = strPath; }	// see default at Open

private:
	sqlite3* m_pdb3 = nullptr;
	CString m_strFilePath;
	CString m_strImportPath;
	CString m_strExportPath;
};


class CSQLiteException : public CException
{
public:
	DECLARE_DYNAMIC(CSQLiteException)
	CSQLiteException(LPCTSTR lpszErrorText, BOOL bAutoDelete = TRUE);
	CSQLiteException(LPCSTR lpszErrorText, BOOL bAutoDelete = TRUE);
//	virtual ~CSQLiteException() {}
	virtual BOOL GetErrorMessage(
		LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext) const;
	void AddContext(const CString& strContext) { m_strErrorText = strContext + _T(": ") + m_strErrorText; }

protected:
	CString m_strErrorText;
};