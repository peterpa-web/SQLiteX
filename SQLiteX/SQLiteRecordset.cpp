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

	if (m_updState != UpdState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": Open() bad updState");

	CStringA utf8SQL;
	if (lpszSQL != nullptr)
		utf8SQL = ToUtf8(lpszSQL);
	else
	{
		utf8SQL = "SELECT ";
		CFieldExchange fx(FX_Task::sqlSelect);
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

	int iResult = sqlite3_prepare_v3(m_pDB->GetHandle(), utf8SQL, -1, 0, &m_pStmt, NULL);
	if (iResult != 0)
	{
		CString str;
		str.Format(_T("%s: %s"), (LPCWSTR)GetDefaultSQL(), (LPCWSTR)m_pDB->GetLastError());
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
	CFieldExchange fx(FX_Task::sqlKey);
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
	m_updState = UpdState::done;
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
	return m_pStmt != nullptr;
}

bool CSQLiteRecordset::IsDeleted() const
{
	return false;
}

void CSQLiteRecordset::Create()
{
	if (m_updState != UpdState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": Create() bad updState");
	CStringA utf8Sql = "CREATE TABLE ";
	CStringA utf8Table = ToUtf8(GetDefaultSQL());

	CFieldExchange fx(FX_Task::sqlCreate);
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
	if (IsOpen())
		throw new CSQLiteException(GetDefaultSQL() + ": Import() bad open state");

	if (m_updState != UpdState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": Import() bad updState");

	CFieldExchange fx(FX_Task::sqlSelect);
	DoFieldExchange(&fx);

	CStringA utf8InsertStart = "INSERT INTO " + ToUtf8(GetDefaultSQL());
	utf8InsertStart += " (";
	utf8InsertStart += fx.m_utf8SQL;
	utf8InsertStart += ") VALUES (";
	CString strFileName = GetDefaultSQL();
	if (strFileName[0] == '[')	// then strip
		strFileName = strFileName.Mid(1, strFileName.GetLength() - 2);
	CString strFilePath = m_pDB->GetImportPath() + '\\' + strFileName + '.' + pszExt;
	CStdioFile fin(strFilePath, CFile::modeRead | CFile::typeText);
	CStringA strLine;
	int nLine = 0;
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
		CFieldExchange fx(FX_Task::dataImport);
		fx.m_fmt = fmt;
		fx.m_cSQLSep = cSep;
		fx.m_strImportLine = strLine;
		DoFieldExchange(&fx);
		CStringA utf8Insert = utf8InsertStart;
		utf8Insert += fx.m_utf8SQL;
		utf8Insert += ");";
		try
		{
			m_pDB->ExecuteSQL(utf8Insert);
		}
		catch (CSQLiteException* pe)
		{
			CString str;
			str.Format(_T(" (%d)"), nLine);
			pe->AddContext(GetDefaultSQL() + str);
			throw;
		}
	}
}

void CSQLiteRecordset::Export(TxtFmt fmt, LPCTSTR pszExt, char cSep)
{
	ASSERT(fmt != TxtFmt::standard);

	if (m_updState != UpdState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": Export() bad updState");

	if (!IsOpen())
		throw new CSQLiteException(GetDefaultSQL() + ": Export() missing Open()");

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
			CFieldExchange fx(FX_Task::dataExport);
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
		CFieldExchange fx(FX_Task::dataClear);
		DoFieldExchange(&fx);
		return;
	}
	m_bEOF = false;

	CFieldExchange fx(FX_Task::dataRead);
	DoFieldExchange(&fx);
}

void CSQLiteRecordset::Edit()
{
	if (m_updState == UpdState::edit)
		return;
	if (m_nDefaultType == readOnly)
		throw new CSQLiteException(GetDefaultSQL() + ": readOnly");
	if (m_updState != UpdState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": Edit() bad updState");
	m_updState = UpdState::edit;
}

void CSQLiteRecordset::AddNew()
{
	if (m_updState == UpdState::addNew)
		return;
	if (m_nDefaultType == readOnly)
		throw new CSQLiteException(GetDefaultSQL() + ": readOnly");
	if (m_updState != UpdState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": AddNew() bad updState");
	m_updState = UpdState::addNew;
	CFieldExchange fx(FX_Task::dataClear);
	DoFieldExchange(&fx);
}

void CSQLiteRecordset::Update()
{
	if (m_updState == UpdState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": Update() missing AddNew() or Edit()");
	if (m_nDefaultType == readOnly)
		throw new CSQLiteException(GetDefaultSQL() + ": readOnly");
	if (m_updState == UpdState::addNew)
	{
		CFieldExchange fx1(FX_Task::sqlInsert);
		DoFieldExchange(&fx1);

		CStringA utf8Insert = "INSERT INTO " + ToUtf8(GetDefaultSQL());
		utf8Insert += " (" + fx1.m_utf8SQL + ") VALUES (";

		CFieldExchange fx2(FX_Task::dataWrite);
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
		CFieldExchange fx3(FX_Task::dataRowId);
		fx3.m_nRowId = m_pDB->GetLastRowId();
		DoFieldExchange(&fx3);
		m_updState = UpdState::done;
		return;
	}
	if (m_updState == UpdState::edit)
	{
		CFieldExchange fx1(FX_Task::dataUpdate);
		DoFieldExchange(&fx1);

		CStringA utf8Update = "UPDATE " + ToUtf8(GetDefaultSQL()) + " SET " + fx1.m_utf8SQL;

		CFieldExchange fx2(FX_Task::sqlKey);
		DoFieldExchange(&fx2);
		utf8Update += " WHERE " + fx2.m_utf8SQL;

		CFieldExchange fx3(FX_Task::dataIdent);
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
		m_updState = UpdState::done;
	}
}

void CSQLiteRecordset::Delete()
{
	if (m_nDefaultType == readOnly)
		throw new CSQLiteException(GetDefaultSQL() + ": readOnly");
	if (m_updState != UpdState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": Delete() bad updState");

	CStringA utf8Delete = "DELETE FROM " + ToUtf8(GetDefaultSQL());

	CFieldExchange fx1(FX_Task::sqlKey);
	DoFieldExchange(&fx1);
	utf8Delete += " WHERE " + fx1.m_utf8SQL;

	CFieldExchange fx2(FX_Task::dataIdent);
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
	case FX_Task::sqlCreate:
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
	case FX_Task::sqlSelect:
	{
		pFX->AddSQL(szName);
		break;
	}
	case FX_Task::sqlInsert:
	{
		if ((dwFlags & FX_PK) == 0)
			pFX->AddSQL(szName);
		break;
	}
	case FX_Task::sqlKey:
		if ((dwFlags & FX_PK) != 0 && nType == SQLITE_INTEGER)
			pFX->m_utf8SQL = ToUtf8(szName);
		break;
	case FX_Task::dataImport:
	{
		CStringA strData = pFX->NextImportField();
		pFX->AddSQL(strData, ',');
		break;
	}
	case FX_Task::dataIdent:
	case FX_Task::dataRowId:
		break;
	default:
		throw new CSQLiteException(GetDefaultSQL() + ": bad FX_Task");
		break;
	}
}

void CSQLiteRecordset::RFX_Bool(CFieldExchange* pFX, LPCTSTR szName, BOOL& value, DWORD dwFlags)
{
	switch (pFX->m_task)
	{
	case FX_Task::dataClear:
		value = 0;
		break;
	case FX_Task::dataRead:
		value = sqlite3_column_int(m_pStmt, pFX->m_nField++);
		break;
	case FX_Task::dataUpdate:
	{
		CStringA s;
		s.Format("%S=%d", szName, value);
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::dataWrite:
	case FX_Task::dataExport:
		pFX->AddSQL(value == 0 ? "0" : "1");
		break;
	default:
		RFX_Gen(pFX, szName, SQLITE_INTEGER, dwFlags);
		break;
	}
}

void CSQLiteRecordset::RFX_Long(CFieldExchange* pFX, LPCTSTR szName, long& value, DWORD dwFlags)
{
	switch (pFX->m_task)
	{
	case FX_Task::dataClear:
		value = 0;
		break;
	case FX_Task::dataRead:
		value = sqlite3_column_int(m_pStmt, pFX->m_nField++);
		break;
	case FX_Task::dataUpdate:
		if ((dwFlags & FX_PK) == 0)
		{
			CStringA s;
			if (value != 0 || (dwFlags & FX_NN) != 0)
				s.Format("%S=%d", szName, value);
			else
				s.Format("%S=NULL", szName);
			pFX->AddSQL(s);
		}
		break;
	case FX_Task::dataWrite:
		if ((dwFlags & FX_PK) != 0)
			break;
		// fall through
	case FX_Task::dataExport:
		{
			CStringA s("NULL");
			if (value != 0 || (dwFlags & FX_NN) != 0)
				s.Format("%d", value);
			pFX->AddSQL(s);
		}
		break;
	case FX_Task::dataIdent:
		if ((dwFlags & FX_PK) != 0)
		{
			CStringA s;
			s.Format("%d", value);
			pFX->AddSQL(s);
		}
		break;
	case FX_Task::dataRowId:
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
	case FX_Task::dataClear:
		value.Empty();
		break;
	case FX_Task::dataImport:
	{
		CStringA strData = pFX->NextImportField();
		if (strData[0] != '"')
			strData = '"' + strData + '"';
		if (pFX->m_fmt == TxtFmt::standard || pFX->m_fmt == TxtFmt::isoGerman)
		{
			CStringW str(strData);
			strData = ToUtf8(str);
		}
		pFX->AddSQL(strData, ',');
		break;
	}
	case FX_Task::dataRead:
		value = FromUtf8((LPCSTR)sqlite3_column_text(m_pStmt, pFX->m_nField++));
		break;
	case FX_Task::dataUpdate:
	{
		CStringA s = ToUtf8(szName) + '=';
		if (!value.IsEmpty() || (dwFlags & FX_NN) != 0)
			pFX->AddSQL(s + '"' + ToUtf8(value) + '"');
		else
			pFX->AddSQL(s + "NULL");
		break;
	}
	case FX_Task::dataWrite:
	case FX_Task::dataExport:
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
	default:
		RFX_Gen(pFX, szName, SQLITE_TEXT, dwFlags);
		break;
	}
}

void CSQLiteRecordset::RFX_Double(CFieldExchange* pFX, LPCTSTR szName, double& value, DWORD dwFlags)
{
	switch (pFX->m_task)
	{
	case FX_Task::dataClear:
		value = 0.0;
		break;
	case FX_Task::dataRead:
		value = sqlite3_column_double(m_pStmt, pFX->m_nField++);
		break;
	case FX_Task::dataUpdate:
	{
		CStringA s;
		if (value != 0.0 || (dwFlags & FX_NN) != 0)
			s.Format("%S=%f", szName, value);
		else
			s.Format("%S=NULL", szName);
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::dataWrite:
	case FX_Task::dataExport:
	{
		CStringA s("NULL");
		if (value != 0 || (dwFlags & FX_NN) != 0)
			s.Format("%f", value);
		if (pFX->m_fmt == TxtFmt::isoGerman)
			s.Replace('.', ',');
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::dataImport:
		{
			CStringA strData = pFX->NextImportField();
			strData.Replace(',', '.');
			pFX->AddSQL(strData, ',');
		}
		break;
	default:
		RFX_Gen(pFX, szName, SQLITE_FLOAT, dwFlags);
		break;
	}
}

void CSQLiteRecordset::RFX_Date(CFieldExchange* pFX, LPCTSTR szName, CTime& value, DWORD dwFlags)
{
	switch (pFX->m_task)
	{
	case FX_Task::dataClear:
		value = 0;
		break;
	case FX_Task::dataRead:
	{
		int t = sqlite3_column_int(m_pStmt, pFX->m_nField++);
		if (t == 0)
			value = 0;
		else
		{
			int d = t % 100;
			int m = (t / 100) % 100;
			int y = t / 10000;
			value = CTime(y, m, d, 0, 0, 0);
		}
		break;
	}
	case FX_Task::dataUpdate:
	{
		CStringA s;
		if (value == 0 )
			s.Format("%S=NULL", szName);
		else
		{
			long l = (value.GetYear() * 100 + value.GetMonth()) * 100 + value.GetDay();
			s.Format("%S=%d", szName, l);
		}
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::dataExport:
	{
		CStringA s("NULL");
		if (value != 0)
		{
			if (pFX->m_fmt == TxtFmt::utf8Native)
			{
				long l = (value.GetYear() * 100 + value.GetMonth()) * 100 + value.GetDay();
				s.Format("%d", l);
			}
			else
				s = value.Format("%d.%m.%Y");
		}
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::dataWrite:
	{
		CStringA s("NULL");
		if (value != 0)
		{
			long l = (value.GetYear() * 100 + value.GetMonth()) * 100 + value.GetDay();
			s.Format("%d", l);
		}
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::dataImport:
	{
		// stored as int: YYYYMMDD
		// import d.m.yyyy hh:mm:ss
		CStringA strData = CStringA(pFX->NextImportField());
		int p = 0;
		CStringA s = strData.Tokenize(".", p);
		if (p > 0)
		{
			int d = atoi(s);
			s = strData.Tokenize(".", p);
			if (p > 0)
			{
				int m = atoi(s);
				s = strData.Tokenize(" ", p);
				int y = atoi(s);
				strData.Format("%4.4d%2.2d%2.2d", y, m, d);
			}
		}
		pFX->AddSQL(strData, ',');
		break;
	}
	default:
		RFX_Gen(pFX, szName, SQLITE_INTEGER, dwFlags);
		break;
	}
}

void CSQLiteRecordset::RFX_Euro(CFieldExchange* pFX, LPCTSTR szName, CEuro& value, DWORD dwFlags)
{
	switch (pFX->m_task)
	{
	case FX_Task::dataImport:
	{
		CStringA strData = pFX->NextImportField();
		CEuro e;
		e.FromString(strData);
		CStringA str;
		str.Format("%d", e.GetCentsRef());
		pFX->AddSQL(str, ',');
		break;
	}
	case FX_Task::dataExport:
	{
		if (value.ToDouble() == 0.0)
			pFX->AddSQL("NULL");
		else
			pFX->AddSQL(value.ToString());
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
		strLim = _T("\"") + m_cSQLSep;
	int p = m_strImportLine.Find(strLim, s);
	if (p > 0)
	{
		m_nStartField = p + strLim.GetLength();
		return m_strImportLine.Mid(s, p - s + strLim.GetLength() - 1);
	}
	else
	{
		m_nStartField = m_strImportLine.GetLength();
		return m_strImportLine.Mid(s);
	}
}
