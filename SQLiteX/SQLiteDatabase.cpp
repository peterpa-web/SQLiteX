#include "pch.h"
#include "SQLiteDatabase.h"

CStringW FromUtf8(LPCSTR pszUtf8)
{
	if (pszUtf8 == nullptr)
		return CStringW();
	size_t nLen = strlen(pszUtf8);
	if (nLen == 0)
		return CStringW();

	CStringW str;
	LPWSTR pBuf = str.GetBuffer(nLen + 1);
	int nNewLen = MultiByteToWideChar(CP_UTF8, 0, pszUtf8, nLen, pBuf, nLen + 1);
	str.ReleaseBuffer(nNewLen);
	if (nNewLen == 0)
		throw new CSQLiteException(L"CStringUtil::FromUtf8 failed");
	return str;
}

CStringA ToUtf8(LPCWSTR pszUtf16)
{
	if (pszUtf16 == nullptr)
		return CStringA();
	size_t nLen = wcslen(pszUtf16);
	if (nLen == 0)
		return CStringA();

	CStringA str;
	int nLen8 = WideCharToMultiByte(CP_UTF8, 0, pszUtf16, -1, NULL, 0, NULL, NULL);
	LPSTR pBuf = str.GetBuffer(nLen8);
	int nNewLen = WideCharToMultiByte(CP_UTF8, 0, pszUtf16, nLen, pBuf, nLen8, NULL, NULL);
	str.ReleaseBuffer(nNewLen);
	if (nNewLen == 0)
		throw new CSQLiteException(L"CStringUtil::ToUtf8 failed");
	return str;
}

CSQLiteDatabase::~CSQLiteDatabase()
{
	Close();
}

bool CSQLiteDatabase::Open(LPCWSTR lpszFilePath)
{
	m_strFilePath = lpszFilePath;
	int p = m_strFilePath.ReverseFind('\\');
	m_strImportPath = m_strExportPath = m_strFilePath.Left(p);

	CStringA utf8FilePath = ToUtf8(lpszFilePath);
	int iResult = sqlite3_open(utf8FilePath, &m_pdb3);
	if (iResult == 0)
		return true;

	m_pdb3 = nullptr;
	return false;
}

void CSQLiteDatabase::Close()
{
	if (m_pdb3 == nullptr)
		return;

	int nRc = sqlite3_close(m_pdb3);
	if (nRc != SQLITE_OK)
		throw new CSQLiteException(GetLastError());
	//	if (iResult != 0)
//	{
//		TRACE2("sqlite3_close() ret=%d %s\n", iResult, GetLastError());
//		ASSERT(FALSE);	// 5: Assure all recordsets are closed before
//	}
	m_pdb3 = nullptr;
}

void CSQLiteDatabase::ExecuteSQL(const CStringA& utf8Sql)
{
	ASSERT(IsOpen());
	TRACE1("ExecSQL %S\n", utf8Sql);
	int nRc = sqlite3_exec(m_pdb3, utf8Sql, nullptr, nullptr, nullptr);
	if (nRc != SQLITE_OK)
		throw new CSQLiteException(GetLastError());
}

long CSQLiteDatabase::GetLastRowId()
{
	return (long)sqlite3_last_insert_rowid(GetHandle());
}

bool CSQLiteDatabase::BeginTrans()
{
	ExecuteSQL(CStringA("BEGIN;"));
	return true;
}

void CSQLiteDatabase::CommitTrans()
{
	ExecuteSQL(CStringA("COMMIT;"));
}

void CSQLiteDatabase::Rollback()
{
	ExecuteSQL(CStringA("ROLLBACK;"));
}

CStringW CSQLiteDatabase::GetLastError()
{
	CStringW str((LPCWSTR)sqlite3_errmsg16(m_pdb3));
	int nOffs = sqlite3_error_offset(m_pdb3);
	if (nOffs >= 0)
		str.Format(L"%s (offs=%d)", (LPCWSTR)str, nOffs);
	return str;
}


//#################################################

IMPLEMENT_DYNAMIC(CSQLiteException, CException)

CSQLiteException::CSQLiteException(LPCWSTR lpszErrorText, BOOL bAutoDelete) : CException(bAutoDelete)
{
	m_strErrorText = lpszErrorText;
	TRACE1("CSQLiteException: %s\n", m_strErrorText);
}

CSQLiteException::CSQLiteException(LPCSTR lpszErrorText, BOOL bAutoDelete) : CException(bAutoDelete)
{
	m_strErrorText = FromUtf8(lpszErrorText);
	TRACE1("CSQLiteException: %s\n", m_strErrorText);
}

BOOL CSQLiteException::GetErrorMessage(
	LPTSTR lpszError, UINT nMaxError,
	PUINT pnHelpContext) const
{
	ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));
	if (pnHelpContext != NULL)
		*pnHelpContext = 0;

	if (nMaxError == 0 || lpszError == NULL)
		return FALSE;

	LPTSTR dummy = lstrcpyn(lpszError, m_strErrorText, nMaxError);
	return TRUE;
}

