#include "pch.h"
#include "SQLiteDatabase.h"
#include "SQLiteRecordset.h"

CSQLiteRecordset::CSQLiteRecordset(CSQLiteDatabase* pdb) : m_pDB(pdb)
{
	ASSERT(pdb != nullptr);
	ASSERT(m_pDB->IsOpen());
}

CSQLiteRecordset::~CSQLiteRecordset()
{
	Close();
}

bool CSQLiteRecordset::Open(LPCWSTR lpszSQL)
{
	if (IsOpen())
	{
		ASSERT(FALSE);
		return false;
	}

	if (m_recState != RecState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": Open() recState != done");

	m_recState = RecState::open;

	CStringA utf8SQL;
	if (lpszSQL != nullptr)
		utf8SQL = ToUtf8(lpszSQL);
	else
	{
		utf8SQL = "SELECT ";
		CFieldExchange fx(FX_Task::colNames);
		DoFieldExchange(&fx);
		utf8SQL += fx.m_utf8SQL;
		utf8SQL += " FROM ";
		utf8SQL += ToUtf8(GetDefaultSQL());

		if (!m_strFilter.IsEmpty())
		{
			utf8SQL += " WHERE ";
			utf8SQL += ToUtf8(m_strFilter);
		}
		if (!m_strSort.IsEmpty())
		{
			utf8SQL += " ORDER BY ";
			utf8SQL += ToUtf8(m_strSort);
		}
		utf8SQL += ";";
	}

	int iResult = sqlite3_prepare_v2(m_pDB->GetHandle(), utf8SQL, -1, &m_pStmt, NULL);
	if (iResult != SQLITE_OK)
	{
		CString str;
		str.Format(_T("Open %s: %s"), (LPCWSTR)GetDefaultSQL(), (LPCWSTR)m_pDB->GetLastError());
		Close();
		throw new CSQLiteException(str);
	}
	// todo: bind params
	ASSERT(m_nParams == 0);
	// rc = sqlite3_bind_text(pstmt, 1, argc[2], -1, NULL);

	// read 1st record
	MoveNext();
	return true;
}

bool CSQLiteRecordset::OpenRow(long nRow)
{
	CFieldExchange fx(FX_Task::pkName);
	fx.m_utf8SQL = "_rowid_";
	DoFieldExchange(&fx);

	CString strSQL;
	strSQL.Format(L"SELECT * FROM %s WHERE %s = %d;", (LPCTSTR)GetDefaultSQL(), 
		(LPCWSTR)FromUtf8(fx.m_utf8SQL), nRow);
	return Open(strSQL);
}

void CSQLiteRecordset::Close()
{
	m_bEOF = true;
	m_recState = RecState::done;
	if (m_pStmt == nullptr)
		return;

	int iResult = sqlite3_finalize(m_pStmt);
	if (iResult != 0)
		TRACE1("sqlite3_finalize() ret=%d\n", iResult);
	m_pStmt = nullptr;
}

bool CSQLiteRecordset::Requery()
{
	int iResult = sqlite3_reset(m_pStmt);
	ASSERT(m_recState == RecState::open);
	if (iResult != SQLITE_OK)
	{
		CString str;
		str.Format(_T("%s: %s"), (LPCWSTR)GetDefaultSQL(), (LPCWSTR)m_pDB->GetLastError());
		Close();
		throw new CSQLiteException(str);
	}
	// read 1st record
	MoveNext();
	return true;
}

bool CSQLiteRecordset::IsOpen() const
{
	return m_recState == RecState::open && m_pStmt != nullptr;
}

bool CSQLiteRecordset::IsDeleted() const
{
	return false;	// dummy
}

void CSQLiteRecordset::Create()
{
	if (m_recState != RecState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": Create() recState != done");
	CStringA utf8Sql = "CREATE TABLE ";
	CStringA utf8Table = ToUtf8(GetDefaultSQL());

	CFieldExchange fx(FX_Task::colTypesForCreate);
	DoFieldExchange(&fx);

	utf8Sql += utf8Table;
	utf8Sql += '(' + fx.m_utf8SQL;
	if (!m_strConstraints.IsEmpty())
		utf8Sql += ',' + ToUtf8(m_strConstraints);
	utf8Sql += ");";
	try
	{
		m_pDB->ExecuteSQL(utf8Sql);
	}
	catch (CSQLiteException* pe)
	{
		pe->AddContext(GetDefaultSQL());
		throw pe;
	}
}

unsigned char utf8BOM[3] = { 0xEF, 0xBB, 0xBF };

BOOL CSQLiteRecordset::ReadStringA(CStdioFile& f, CStringA& rString)
{
	rString = "";    // empty string without deallocating
	const int nMaxSize = 128;
	LPSTR lpsz = rString.GetBuffer(nMaxSize);
	LPSTR lpszResult;
	int nLen = 0;
	for (;;)
	{
		lpszResult = fgets(lpsz, nMaxSize + 1, f.m_pStream);
		rString.ReleaseBuffer();

		// handle error/eof case
		if (lpszResult == NULL && !feof(f.m_pStream))
		{
			Afx_clearerr_s(f.m_pStream);
			AfxThrowFileException(CFileException::genericException, _doserrno,
				f.GetFileName());
		}

		// if string is read completely or EOF
		if (lpszResult == NULL ||
			(nLen = AtlStrLen(lpsz)) < nMaxSize ||
			lpsz[nLen - 1] == '\n')
			break;

		nLen = rString.GetLength();
		lpsz = rString.GetBuffer(nMaxSize + nLen) + nLen;
	}

	// remove '\n' from end of string if present
	lpsz = rString.GetBuffer(0);
	nLen = rString.GetLength();
	if (nLen != 0 && lpsz[nLen - 1] == '\n')
		rString.GetBufferSetLength(nLen - 1);

	return nLen != 0;
}

void CSQLiteRecordset::Import(TxtFmt fmt, LPCTSTR pszExt, char cSep)
{
	if (m_recState != RecState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": Import() recState != done");

	CFieldExchange fxN(FX_Task::colNames);
	DoFieldExchange(&fxN);

	CStringA utf8InsertStmt = "INSERT INTO " + ToUtf8(GetDefaultSQL());
	utf8InsertStmt += " (";
	utf8InsertStmt += fxN.m_utf8SQL;
	utf8InsertStmt += ") VALUES (";
	CFieldExchange fxV(FX_Task::colVarsForImport);
	DoFieldExchange(&fxV);
	utf8InsertStmt += fxV.m_utf8SQL;
	utf8InsertStmt += ");";
	TRACE1("Import: %S\n", utf8InsertStmt);
	int iResult = sqlite3_prepare_v2(m_pDB->GetHandle(), utf8InsertStmt, -1, &m_pStmt, NULL);
	if (iResult != SQLITE_OK)
	{
		CString str;
		str.Format(_T("Import prep %s: %s"), (LPCWSTR)GetDefaultSQL(), (LPCWSTR)m_pDB->GetLastError());
		Close();
		throw new CSQLiteException(str);
	}

	CString strFileName = GetDefaultSQL();
	if (strFileName[0] == '[')	// then strip
		strFileName = strFileName.Mid(1, strFileName.GetLength() - 2);
	CString strFilePath = m_pDB->GetImportPath() + '\\' + strFileName + '.' + pszExt;
	CStdioFile fin(strFilePath, CFile::modeRead | CFile::typeText);
	CStringA strLine;
	int nLine = 0;
	try
	{
		while (ReadStringA(fin, strLine))
		{
			if (nLine == 0 && strLine.GetLength() >= 3)
			{
				if (strncmp(strLine, (LPCSTR)utf8BOM, 3) == 0)
				{
					if (fmt == TxtFmt::standard)
						fmt = TxtFmt::utf8MarkGerman;
					strLine = strLine.Mid(3);
				}
			}
			++nLine;
			CFieldExchange fx(FX_Task::colParseBindForImport);
			fx.m_fmt = fmt;
			fx.m_cSQLSep = cSep;
			fx.m_strImportLine = strLine;
			DoFieldExchange(&fx);

			iResult = sqlite3_step(m_pStmt);
			if (iResult != SQLITE_DONE)
			{
				CString str;
				str.Format(_T("Import step %s: %s"), (LPCWSTR)GetDefaultSQL(), (LPCWSTR)m_pDB->GetLastError());
				throw new CSQLiteException(str);
			}
			iResult = sqlite3_reset(m_pStmt);
			if (iResult != SQLITE_OK)
			{
				CString str;
				str.Format(_T("Import reset %s: %s"), (LPCWSTR)GetDefaultSQL(), (LPCWSTR)m_pDB->GetLastError());
				throw new CSQLiteException(str);
			}
//			iResult = sqlite3_clear_bindings(m_pStmt);
//			if (iResult != SQLITE_OK)
//			{
//				CString str;
//				str.Format(_T("Import clear_b %s: %s"), (LPCWSTR)GetDefaultSQL(), (LPCWSTR)m_pDB->GetLastError());
//				Close();
//				throw new CSQLiteException(str);
//			}
		}
	}
	catch (CSQLiteException* pe)
	{
		CString str;
		str.Format(_T(" (%d)"), nLine);
		pe->AddContext(GetDefaultSQL() + str);
		Close();
		throw;
	}
	Close();
}

void CSQLiteRecordset::Export(TxtFmt fmt, LPCTSTR pszExt, char cSep)
{
	ASSERT(fmt != TxtFmt::standard);

	if (m_recState != RecState::open)
		throw new CSQLiteException(GetDefaultSQL() + ": Export() recState != open");

	int nLine = 0;
	try
	{
		CString strFileName = GetDefaultSQL();
		if (strFileName[0] == '[')	// then strip
			strFileName = strFileName.Mid(1, strFileName.GetLength() - 2);
		CString strFilePath = m_pDB->GetExportPath() + '\\' + strFileName + '.' + pszExt;
		CFile fout(strFilePath, CFile::modeWrite | CFile::modeCreate);
		if (fmt == TxtFmt::utf8MarkGerman)
			fout.Write(utf8BOM, 3);

		while (!IsEOF())
		{
			++nLine;
			CFieldExchange fx(FX_Task::valStrings);
			fx.m_cSQLSep = cSep;
			fx.m_fmt = fmt;
			DoFieldExchange(&fx);
			fout.Write(fx.m_utf8SQL, fx.m_utf8SQL.GetLength());
			fout.Write("\r\n", 2);
			MoveNext();
		}
	}
	catch (CSQLiteException* pe)
	{
		CString str;
		str.Format(_T(" (%d)"), nLine);
		pe->AddContext(GetDefaultSQL() + str);
		throw;
	}
}

void CSQLiteRecordset::MoveNext()
{
	int iResult = sqlite3_step(m_pStmt);
	if (iResult != SQLITE_ROW)
	{
		m_bEOF = true;
		if (iResult != SQLITE_DONE)
		{
			CString str;
			str.Format(_T("%s: %s"), (LPCWSTR)GetDefaultSQL(), (LPCWSTR)m_pDB->GetLastError());
			Close();
			throw new CSQLiteException(str);
		}
		CFieldExchange fx(FX_Task::valClearAll);
		DoFieldExchange(&fx);
		return;
	}
	m_bEOF = false;

	CFieldExchange fx(FX_Task::valReadAll);
	DoFieldExchange(&fx);
}

void CSQLiteRecordset::Edit()
{
	if (m_recState == RecState::edit)
		return;
	if (m_nDefaultType == view)
		throw new CSQLiteException(GetDefaultSQL() + ": readOnly");
	if (m_recState != RecState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": Edit() recState != done");
	m_recState = RecState::edit;
}

void CSQLiteRecordset::AddNew()
{
	if (m_recState == RecState::addNew)
		return;
	if (m_nDefaultType == view)
		throw new CSQLiteException(GetDefaultSQL() + ": readOnly");
	if (m_recState != RecState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": AddNew() recState != done");
	m_recState = RecState::addNew;
	CFieldExchange fx(FX_Task::valClearAll);
	DoFieldExchange(&fx);
}

void CSQLiteRecordset::Update()
{
	if (m_recState != RecState::addNew && m_recState != RecState::edit)
		throw new CSQLiteException(GetDefaultSQL() + ": Update() missing AddNew() or Edit()");
	if (m_nDefaultType == view)
		throw new CSQLiteException(GetDefaultSQL() + ": readOnly");
	if (m_recState == RecState::addNew)
	{
		CFieldExchange fx1(FX_Task::colNames);
		DoFieldExchange(&fx1);

		CStringA utf8Insert = "INSERT INTO " + ToUtf8(GetDefaultSQL());
		utf8Insert += " (" + fx1.m_utf8SQL + ") VALUES (";

		CFieldExchange fx2(FX_Task::valStrings);
		DoFieldExchange(&fx2);
		utf8Insert += fx2.m_utf8SQL + ");";
		try
		{
			m_pDB->ExecuteSQL(utf8Insert);
		}
		catch (CSQLiteException* pe)
		{
			pe->AddContext(GetDefaultSQL());
			throw;
		}
		CFieldExchange fx3(FX_Task::pkAfterInsert);
		fx3.m_nRowId = m_pDB->GetLastRowId();
		DoFieldExchange(&fx3);
		m_recState = RecState::done;
		return;
	}
	if (m_recState == RecState::edit)
	{
		CFieldExchange fx1(FX_Task::colNameValForUpdate);
		DoFieldExchange(&fx1);

		CStringA utf8Update = "UPDATE " + ToUtf8(GetDefaultSQL()) + " SET " + fx1.m_utf8SQL;

		CFieldExchange fx2(FX_Task::pkName);
		DoFieldExchange(&fx2);
		utf8Update += " WHERE " + fx2.m_utf8SQL;

		CFieldExchange fx3(FX_Task::pkString);
		DoFieldExchange(&fx3);

		utf8Update += '=' + fx3.m_utf8SQL + ';';
		try
		{
			m_pDB->ExecuteSQL(utf8Update);
		}
		catch (CSQLiteException* pe)
		{
			pe->AddContext(GetDefaultSQL());
			throw;
		}
		m_recState = RecState::done;
	}
}

void CSQLiteRecordset::Delete()
{
	if (m_nDefaultType == view)
		throw new CSQLiteException(GetDefaultSQL() + ": readOnly");
	if (m_recState != RecState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": Delete() recState != done");

	CStringA utf8Delete = "DELETE FROM " + ToUtf8(GetDefaultSQL());

	CFieldExchange fx1(FX_Task::pkName);
	DoFieldExchange(&fx1);
	utf8Delete += " WHERE " + fx1.m_utf8SQL;

	CFieldExchange fx2(FX_Task::pkString);
	DoFieldExchange(&fx2);

	utf8Delete += '=' + fx2.m_utf8SQL + ';';
	try
	{
		m_pDB->ExecuteSQL(utf8Delete);
	}
	catch (CSQLiteException* pe)
	{
		pe->AddContext(GetDefaultSQL());
		throw;
	}
}

void CSQLiteRecordset::DeleteAll()
{
	if (m_nDefaultType == view)
		throw new CSQLiteException(GetDefaultSQL() + ": readOnly");
	if (m_recState != RecState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": Delete() recState != done");

	CStringA utf8Delete = "DELETE FROM " + ToUtf8(GetDefaultSQL()) + ';';
	try
	{
		m_pDB->ExecuteSQL(utf8Delete);
	}
	catch (CSQLiteException* pe)
	{
		pe->AddContext(GetDefaultSQL());
		throw;
	}
}

void CSQLiteRecordset::Drop()
{
	if (m_recState != RecState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": Delete() recState != done");
	CStringA utf8Drop = "DROP TABLE ";
	if (m_nDefaultType == view)
		utf8Drop = "DROP VIEW ";

	utf8Drop += ToUtf8(GetDefaultSQL()) + ';';
	try
	{
		m_pDB->ExecuteSQL(utf8Drop);
	}
	catch (CSQLiteException* pe)
	{
		pe->AddContext(GetDefaultSQL());
		throw;
	}
}

/*
int CSQLiteRecordset::GetRecordCount()
{
	return 0;
}

void CSQLiteRecordset::SetFieldDirty(void* pField)
{
	throw new CSQLiteException(GetDefaultSQL() + ": SetFieldDirty() to be implemented");
}

void CSQLiteRecordset::SetAbsolutePosition(int nRecord)
{
	throw new CSQLiteException(GetDefaultSQL() + ": SetAbsolutePosition() to be implemented");
}
*/

void CSQLiteRecordset::RFX_Gen(CFieldExchange* pFX, LPCTSTR szName, int nType, DWORD dwFlags)
{
	switch (pFX->m_task)
	{
	case FX_Task::colTypesForCreate:
	{
		pFX->AddSQL(szName);
		switch (nType)
		{
		case SQLITE_INTEGER:
			pFX->m_utf8SQL += " INTEGER";
			break;
		case SQLITE_FLOAT:
			pFX->m_utf8SQL += " FLOAT";
			break;
		case SQLITE_TEXT:
			pFX->m_utf8SQL += " TEXT";
			break;
//		case SQLITE_BLOB:		// to be implemented
//			break;
		default:
			break;
		}
		if ((dwFlags & FX_NN) != 0)
			pFX->m_utf8SQL += " NOT NULL";
		if ((dwFlags & FX_PK) != 0 && nType == SQLITE_INTEGER)
			pFX->m_utf8SQL += " PRIMARY KEY";
		if ((dwFlags & FX_AN) != 0)
			pFX->m_utf8SQL += " AUTOINCREMENT";
		if ((dwFlags & FX_UN) != 0)
			pFX->m_utf8SQL += " UNIQUE";
		break;
	}
	case FX_Task::colNames:
	{
		pFX->AddSQL(szName);
		break;
	}
	case FX_Task::pkName:
		if ((dwFlags & FX_PK) != 0 && nType == SQLITE_INTEGER)
			pFX->m_utf8SQL = ToUtf8(szName);
		break;
	case FX_Task::colVarsForImport:
		pFX->AddSQL("?", ',');
		break;
	case FX_Task::pkString:
	case FX_Task::pkAfterInsert:
		break;		// skip such column types
	default:
		throw new CSQLiteException(GetDefaultSQL() + ": bad FX_Task");
		break;
	}
}

void CSQLiteRecordset::RFX_Bool(CFieldExchange* pFX, LPCTSTR szName, BOOL& value, DWORD dwFlags)
{
	switch (pFX->m_task)
	{
	case FX_Task::valClearAll:
		value = 0;
		break;
	case FX_Task::valReadAll:
		value = sqlite3_column_int(m_pStmt, pFX->m_nField++);
		break;
	case FX_Task::colNameValForUpdate:
	{
		CStringA s;
		s.Format("%S=%d", szName, value);
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::valStrings:
		pFX->AddSQL(value == 0 ? "0" : "1");
		break;
	case FX_Task::colParseBindForImport:
	{
		CStringA strData = pFX->NextImportField();
		int d = atoi(strData);
		if (d != 0 || (dwFlags & FX_NN) != 0)
			sqlite3_bind_int(m_pStmt, ++pFX->m_nField, d);
		else
			sqlite3_bind_null(m_pStmt, ++pFX->m_nField);
		break;
	}
	default:
		RFX_Gen(pFX, szName, SQLITE_INTEGER, dwFlags);
		break;
	}
}

void CSQLiteRecordset::RFX_Long(CFieldExchange* pFX, LPCTSTR szName, long& value, DWORD dwFlags)
{
	switch (pFX->m_task)
	{
	case FX_Task::valClearAll:
		value = 0;
		break;
	case FX_Task::valReadAll:
		value = sqlite3_column_int(m_pStmt, pFX->m_nField++);
		break;
	case FX_Task::colNameValForUpdate:
	{
		CStringA s;
		if (value != 0 || (dwFlags & FX_NN) != 0)
			s.Format("%S=%d", szName, value);
		else
			s.Format("%S=NULL", szName);
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::valStrings:
	{
		CStringA s("NULL");
		if (value != 0 || (dwFlags & FX_NN) != 0)
			s.Format("%d", value);
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::colParseBindForImport:
	{
		CStringA strData = pFX->NextImportField();
		int d = atoi(strData);
		if (d != 0 || (dwFlags & FX_NN) != 0)
			sqlite3_bind_int(m_pStmt, ++pFX->m_nField, d);
		else
			sqlite3_bind_null(m_pStmt, ++pFX->m_nField);
		break;
	}
	case FX_Task::pkString:
		if ((dwFlags & FX_PK) != 0)
		{
			CStringA s;
			s.Format("%d", value);
			pFX->AddSQL(s);
		}
		break;
	case FX_Task::pkAfterInsert:
		if ((dwFlags & FX_PK) != 0)
		{
			value = pFX->m_nRowId;
		}
		break;
	default:
		RFX_Gen(pFX, szName, SQLITE_INTEGER, dwFlags);
		break;
	}
}

void CSQLiteRecordset::RFX_Text(CFieldExchange* pFX, LPCTSTR szName, CStringW& value, DWORD dwFlags)
{
	switch (pFX->m_task)
	{
	case FX_Task::valClearAll:
		value.Empty();
		break;
	case FX_Task::valReadAll:
		value = (LPCWSTR)sqlite3_column_text16(m_pStmt, pFX->m_nField++);
		break;
	case FX_Task::colNameValForUpdate:
	{
		CStringA s = ToUtf8(szName) + '=';
		if (!value.IsEmpty() || (dwFlags & FX_NN) != 0)
			pFX->AddSQL(s + '"' + ToUtf8(value) + '"');
		else
			pFX->AddSQL(s + "NULL");
		break;
	}
	case FX_Task::valStrings:
	{
		if (!value.IsEmpty() || (dwFlags & FX_NN) != 0)
		{
			if (pFX->m_fmt == TxtFmt::isoGerman)
				pFX->AddSQL('"' + CStringA(value) + '"');
			else
				pFX->AddSQL('"' + ToUtf8(value) + '"');
		}
		else
			pFX->AddSQL("NULL");
		break;
	}
	case FX_Task::colParseBindForImport:
	{
		CStringA strData = pFX->NextImportField();
		if (!strData.IsEmpty() || (dwFlags & FX_NN) != 0)
		{
			if ((pFX->m_fmt == TxtFmt::standard && strData.FindOneOf("�������") >= 0) ||
				pFX->m_fmt == TxtFmt::isoGerman)
			{
				CStringW str(strData);
				strData = ToUtf8(str);
				sqlite3_bind_text16(m_pStmt, ++pFX->m_nField, (LPCWSTR)str, str.GetLength() * sizeof(WCHAR), SQLITE_TRANSIENT);
			}
			else
				sqlite3_bind_text(m_pStmt, ++pFX->m_nField, (LPCSTR)strData, strData.GetLength(), SQLITE_TRANSIENT);
		}
		else
			sqlite3_bind_null(m_pStmt, ++pFX->m_nField);
		break;
	}
	default:
		RFX_Gen(pFX, szName, SQLITE_TEXT, dwFlags);
		break;
	}
}

void CSQLiteRecordset::RFX_Double(CFieldExchange* pFX, LPCTSTR szName, double& value, DWORD dwFlags)
{
	switch (pFX->m_task)
	{
	case FX_Task::valClearAll:
		value = 0.0;
		break;
	case FX_Task::valReadAll:
		value = sqlite3_column_double(m_pStmt, pFX->m_nField++);
		break;
	case FX_Task::colNameValForUpdate:
	{
		CStringA s;
		if (value != 0.0 || (dwFlags & FX_NN) != 0)
			s.Format("%S=%f", szName, value);
		else
			s.Format("%S=NULL", szName);
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::valStrings:
	{
		CStringA s("NULL");
		if (value != 0.0 || (dwFlags & FX_NN) != 0)
			s.Format("%f", value);
		if (pFX->m_fmt == TxtFmt::isoGerman)
			s.Replace('.', ',');
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::colParseBindForImport:
	{
		CStringA strData = pFX->NextImportField();
		strData.Replace(',', '.');
		double d = atof(strData);
		if (d != 0.0 || (dwFlags & FX_NN) != 0)
			sqlite3_bind_double(m_pStmt, ++pFX->m_nField, d);
		else
			sqlite3_bind_null(m_pStmt, ++pFX->m_nField);
		break;
	}
	default:
		RFX_Gen(pFX, szName, SQLITE_FLOAT, dwFlags);
		break;
	}
}

void CSQLiteRecordset::RFX_Date(CFieldExchange* pFX, LPCTSTR szName, CDateLong& value, DWORD dwFlags)
{
	switch (pFX->m_task)
	{
	case FX_Task::valClearAll:
		value = 0;
		break;
	case FX_Task::valReadAll:
	{
		int t = sqlite3_column_int(m_pStmt, pFX->m_nField++);
		value = CDateLong(t);
		break;
	}
	case FX_Task::colNameValForUpdate:
	{
		CStringA s = szName;
		s += '=' + value.ToSQL();
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::valStrings:
	{
		CStringA s;
		if (pFX->m_fmt == TxtFmt::utf8International)
			s = value.ToStringInt();
		else if (pFX->m_fmt == TxtFmt::utf8MarkGerman || 
				 pFX->m_fmt == TxtFmt::isoGerman)
			s = value.ToStringGer();
		else
			s = value.ToSQL();
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::colParseBindForImport:
	{
		CStringA strData = pFX->NextImportField();
		CDateLong dl(strData);
		int d = dl.ToLong();
		if (d != 0 || (dwFlags & FX_NN) != 0)
			sqlite3_bind_int(m_pStmt, ++pFX->m_nField, d);
		else
			sqlite3_bind_null(m_pStmt, ++pFX->m_nField);
		break;
	}
	default:
		RFX_Gen(pFX, szName, SQLITE_INTEGER, dwFlags);
		break;
	}
}

void CSQLiteRecordset::RFX_DateTime(CFieldExchange* pFX, LPCTSTR szName, COleDateTime& value, DWORD dwFlags)
{
	switch (pFX->m_task)
	{
	case FX_Task::valClearAll:
		value = COleDateTime();
		break;
	case FX_Task::valReadAll:
	{
		DATE t = sqlite3_column_double(m_pStmt, pFX->m_nField++);
		value = COleDateTime(t);
		break;
	}
	case FX_Task::colNameValForUpdate:
	{
		CStringA s = szName;
		if ((DATE)value != 0.0 || (dwFlags & FX_NN) != 0)
			s.Format("%S=%f", szName, (DATE)value);
		else
			s.Format("%S=NULL", szName);
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::valStrings:
	{
		CStringA s;
		if ((DATE)value == 0.0)
			s = "NULL";
		else
		{
			if (pFX->m_fmt == TxtFmt::utf8International)
				s = value.Format(L"%Y-%m-%d %H:%M:%S");
			else if (pFX->m_fmt == TxtFmt::utf8MarkGerman ||
				pFX->m_fmt == TxtFmt::isoGerman)
				s = value.Format(L"%d.%m.%Y %H:%M:%S");
			else
				s.Format("%S=%f", szName, (DATE)value);
		}
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::colParseBindForImport:
	{
		CStringA str = pFX->NextImportField();
		COleDateTime dt;
		if (!dt.ParseDateTime(CString(str)))
		{
			if (!dt.ParseDateTime(CString(str)), 0, 1053)		// ISO fmt
			{
				int p = str.Find('.');
				if (p > 0)
					dt = atof(str);
				else
					dt = CDateLong(str).ToOleDateTime();
			}
		}
		if (dt.GetStatus() != 0)
			throw new CSQLiteException(L"Bad date/time");
/*
		CStringA s;
		DATE da = 0.0;
		int p = str.Find('.');
		if (p > 0 && p < 3)
		{
			p = 0;
			s = str.Tokenize(".", p);
			if (p > 0)
			{
				int d = atoi(s);
				s = str.Tokenize(".", p);
				if (p > 0)
				{
					int mo = atoi(s);
					s = str.Tokenize(" ", p);
					if (p > 0)
					{
						int y = atoi(s);
						s = str.Tokenize(":", p);
						int h = 0;
						int mn = 0;
						int sec = 0;
						if (p > 0)
						{
							h = atoi(s);
							s = str.Tokenize(":", p);
							if (p > 0)
							{
								mn = atoi(s);
								s = str.Tokenize(" ", p);
								sec = atoi(s);
							}
						}
						ASSERT(y >= 1000 && y <= 9999);
						ASSERT(mo >= 1 && mo <= 12);
						ASSERT(d >= 1 && d <= 31);
						ASSERT(h >= 0 && h < 24);
						ASSERT(mn >= 0 && mn < 60);
						ASSERT(sec >= 0 && sec < 60);
						dt = COleDateTime(y, mo, d, h, mn, sec);
					}
				}
				else
					dt = atof(str);
			}
		}
		else
		{
			p = 0;
			s = str.Tokenize("-", p);
			if (p > 0)
			{
				int y = atoi(s);
				s = str.Tokenize("-", p);
				if (p > 0)
				{
					int mo = atoi(s);
					s = str.Tokenize(" ", p);
					if (p > 0)
					{
						int d = atoi(s);
						s = str.Tokenize(":", p);
						if (p > 0)
						{
							int h = atoi(s);
							s = str.Tokenize(":", p);
							if (p > 0)
							{
								int mn = atoi(s);
								s = str.Tokenize(" ", p);
								int sec = atoi(s);
								ASSERT(y >= 1000 && y <= 9999);
								ASSERT(mo >= 1 && mo <= 12);
								ASSERT(d >= 1 && d <= 31);
								ASSERT(h >= 0 && h < 24);
								ASSERT(mn >= 0 && mn < 60);
								ASSERT(sec >= 0 && sec < 60);
								dt = COleDateTime(y, mo, d, h, mn, sec);
							}
						}
					}
				}
			}
			else
			{
				p = 0;
				s = str.Tokenize(":", p);
				if (p > 0)
				{
					int h = atoi(s);
					s = str.Tokenize(":", p);
					if (p > 0)
					{
						int mn = atoi(s);
						s = str.Tokenize(" ", p);
						int sec = atoi(s);
						ASSERT(h >= 0 && h < 24);
						ASSERT(mn >= 0 && mn < 60);
						ASSERT(sec >= 0 && sec < 60);
						dt = COleDateTime(0, 0, 0, h, mn, sec);
					}
				}
				else
				{
					CDateLong dl (str);
					dt = COleDateTime(dl.GetYear(), dl.GetMonth(), dl.GetDay(), 0, 0, 0);
				}		
			}
		}
*/
		if ((DATE)dt != 0.0 || (dwFlags & FX_NN) != 0)
			sqlite3_bind_double(m_pStmt, ++pFX->m_nField, (DATE)dt);
		else
			sqlite3_bind_null(m_pStmt, ++pFX->m_nField);
		break;
	}
	default:
		RFX_Gen(pFX, szName, SQLITE_FLOAT, dwFlags);
		break;
	}
}

void CSQLiteRecordset::RFX_Euro(CFieldExchange* pFX, LPCTSTR szName, CEuro& value, DWORD dwFlags)
{
	switch (pFX->m_task)
	{
	case FX_Task::valStrings:
	{
		if (value.GetCentsRef() == 0)
			pFX->AddSQL("NULL");
		else
		{
			CString s;
			s.Format(_T("%d"), value.GetCentsRef());
			if (pFX->m_fmt == TxtFmt::utf8MarkGerman ||
				pFX->m_fmt == TxtFmt::isoGerman)
			{
				s = value.ToString();
			}
			else if (pFX->m_fmt == TxtFmt::utf8International)
			{
				s = value.ToString();
				s.Replace(',', '.');
			}
			pFX->AddSQL(s);
		}
		break;
	}
	case FX_Task::colParseBindForImport:
	{
		CStringA strData = pFX->NextImportField();
		CEuro e;
		e.FromString(strData);
		int d = e.GetCentsRef();
		if (d != 0 || (dwFlags & FX_NN) != 0)
			sqlite3_bind_int(m_pStmt, ++pFX->m_nField, d);
		else
			sqlite3_bind_null(m_pStmt, ++pFX->m_nField);
		break;
	}
	default:
		RFX_Long(pFX, szName, value.GetCentsRef(), dwFlags);
		break;
	}
}

void CSQLiteRecordset::CFieldExchange::AddSQL(LPCSTR psz, char cSep)
{
	if (*psz == 0)
		return;
	m_nField++;
	if (!m_utf8SQL.IsEmpty())
		m_utf8SQL += cSep == 0 ? m_cSQLSep : cSep;
	m_utf8SQL += psz;
}

CStringA CSQLiteRecordset::CFieldExchange::NextImportField()
{
	int s = m_nStartField;
	CStringA strLim = m_cSQLSep;
	if (m_strImportLine[s] == '"')
		strLim = _T("\"") + CStringA(m_cSQLSep);
	int p = m_strImportLine.Find(strLim, s);
	if (p > 0)
	{
		m_nStartField = p + strLim.GetLength();
		int c = p - s;
		if (strLim.GetLength() > 1)	// strip quotes
		{
			s++;
			c--;
		}
		return m_strImportLine.Mid(s, c);
	}
	else
	{
		m_nStartField = m_strImportLine.GetLength();
		return m_strImportLine.Mid(s);
	}
}
