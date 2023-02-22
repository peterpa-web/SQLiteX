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
		utf8SQL += fx.m_strSQL;
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
	fx.m_strSQL = "_rowid_";
	DoFieldExchange(&fx);

	CString strSQL;
	strSQL.Format(L"SELECT * FROM %s WHERE %s = %d;", (LPCTSTR)GetDefaultSQL(), 
		(LPCWSTR)FromUtf8(fx.m_strSQL), nRow);
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

/*
bool CSQLiteRecordset::IsBOF()
{
	throw new CSQLiteException(GetDefaultSQL() + ": IsBOF() not supported");
	return false;
}
*/

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
	utf8Sql += '(' + fx.m_strSQL;
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

void CSQLiteRecordset::ImportTxt()
{
	if (m_updState != UpdState::done)
		throw new CSQLiteException(GetDefaultSQL() + ": ImportTxt() bad updState");
	CFieldExchange fx(FX_Task::sqlSelect);
	DoFieldExchange(&fx);

	CStringA utf8InsertStart = "INSERT INTO " + ToUtf8(GetDefaultSQL());
	utf8InsertStart += " (";
	utf8InsertStart += fx.m_strSQL;
	utf8InsertStart += ") VALUES (";
	CString strFileName = GetDefaultSQL();
	if (strFileName[0] == '[')	// then strip
		strFileName = strFileName.Mid(1, strFileName.GetLength() - 2);
	CString strFilePath = m_pDB->GetImportPath() + '\\' + strFileName + _T(".txt");
	CStdioFile fin(strFilePath, CFile::modeRead | CFile::typeText);
	CString strLine;
	int nLine = 0;
	while (fin.ReadString(strLine))
	{
		++nLine;
		CFieldExchange fx(FX_Task::dataImport);
		fx.m_strImportLine = strLine;
		DoFieldExchange(&fx);
		CStringA utf8Insert = utf8InsertStart;
		utf8Insert += fx.m_strSQL;
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
			throw pe;
		}
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

/*
void CSQLiteRecordset::MovePrev()
{
	throw new CSQLiteException(GetDefaultSQL() + ": MovePrev() not supported");
}

void CSQLiteRecordset::MoveFirst()
{
	throw new CSQLiteException(GetDefaultSQL() + ": MoveFirst() not supported");
}

void CSQLiteRecordset::MoveLast()
{
	throw new CSQLiteException(GetDefaultSQL() + ": MoveLast() not supported");
}
*/

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
		utf8Insert += " (" + fx1.m_strSQL + ") VALUES (";

		CFieldExchange fx2(FX_Task::dataWrite);
		DoFieldExchange(&fx2);
		utf8Insert += fx2.m_strSQL + ");";
		try
		{
			m_pDB->ExecuteSQL(utf8Insert);
		}
		catch (CSQLiteException* pe)
		{
			pe->AddContext(GetDefaultSQL());
			throw pe;
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

		CStringA utf8Update = "UPDATE " + ToUtf8(GetDefaultSQL()) + " SET " + fx1.m_strSQL;

		CFieldExchange fx2(FX_Task::sqlKey);
		DoFieldExchange(&fx2);
		utf8Update += " WHERE " + fx2.m_strSQL;

		CFieldExchange fx3(FX_Task::dataIdent);
		DoFieldExchange(&fx3);

		utf8Update += '=' + fx3.m_strSQL + ';';
		try
		{
			m_pDB->ExecuteSQL(utf8Update);
		}
		catch (CSQLiteException* pe)
		{
			pe->AddContext(GetDefaultSQL());
			throw pe;
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
	utf8Delete += " WHERE " + fx1.m_strSQL;

	CFieldExchange fx2(FX_Task::dataIdent);
	DoFieldExchange(&fx2);

	utf8Delete += '=' + fx2.m_strSQL + ';';
	try
	{
		m_pDB->ExecuteSQL(utf8Delete);
	}
	catch (CSQLiteException* pe)
	{
		pe->AddContext(GetDefaultSQL());
		throw pe;
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
			pFX->m_strSQL += " INTEGER";
			break;
		case SQLITE_FLOAT:
			pFX->m_strSQL += " FLOAT";
			break;
		case SQLITE_TEXT:
			pFX->m_strSQL += " TEXT";
			break;
//		case SQLITE_BLOB:		// to be implemented
//			break;
		default:
			break;
		}
		if ((dwFlags & FX_NN) != 0)
			pFX->m_strSQL += " NOT NULL";
		if ((dwFlags & FX_PK) != 0 && nType == SQLITE_INTEGER)
			pFX->m_strSQL += " PRIMARY KEY";
		if ((dwFlags & FX_AN) != 0)
			pFX->m_strSQL += " AUTOINCREMENT";
		if ((dwFlags & FX_UN) != 0)
			pFX->m_strSQL += " UNIQUE";
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
			pFX->m_strSQL = ToUtf8(szName);
		break;
	case FX_Task::dataImport:
	{
		CString strData = pFX->NextImportField();
		pFX->AddSQL(strData);
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
			s.Format("%S=%d", szName, value);
			pFX->AddSQL(s);
		}
		break;
	case FX_Task::dataWrite:
		if ((dwFlags & FX_PK) == 0)
		{
			CStringA s;
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
		CString strData = pFX->NextImportField();
		if (strData[0] != '"')
			strData = '"' + strData + '"';
		pFX->AddSQL(strData);
		break;
	}
	case FX_Task::dataRead:
		value = FromUtf8((LPCSTR)sqlite3_column_text(m_pStmt, pFX->m_nField++));
		break;
	case FX_Task::dataUpdate:
	{
		CStringA s = ToUtf8(szName) + '=';
		pFX->AddSQL(s + '"' + ToUtf8(value) + '"');
		break;
	}
	case FX_Task::dataWrite:
	{
		pFX->AddSQL('"' + ToUtf8(value) + '"');
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
		s.Format("%S=%f", szName, value);
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::dataWrite:
	{
		CStringA s;
		s.Format("%f", value);
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::dataImport:
		{
			CString strData = pFX->NextImportField();
			strData.Replace(',', '.');
			pFX->AddSQL(strData);
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
		int d = t % 100;
		int m = (t / 100) % 100;
		int y = t / 10000;
		value = CTime(y, m, d, 0, 0, 0);
		break;
	}
	case FX_Task::dataUpdate:
	{
		long l = (value.GetYear() * 100 + value.GetMonth()) * 100 + value.GetDay();
		CStringA s;
		s.Format("%S=%d", szName, l);
		pFX->AddSQL(s);
		break;
	}
	case FX_Task::dataWrite:
	{
		long l = (value.GetYear() * 100 + value.GetMonth()) * 100 + value.GetDay();
		CStringA s;
		s.Format("%d", l);
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
		int d = atoi(s);
		s = strData.Tokenize(".", p);
		int m = atoi(s);
		s = strData.Tokenize(" ", p);
		int y = atoi(s);
		strData.Format("%4.4d%2.2d%2.2d", y, m, d);
		pFX->AddSQL(strData);
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
		CString strData = pFX->NextImportField();
		CEuro e;
		e.FromString(CStringA(strData));
		CStringA str;
		str.Format("%d", e.GetCentsRef());
		pFX->AddSQL(str);
	}
	break;
	default:
		RFX_Long(pFX, szName, value.GetCentsRef(), dwFlags);
		break;
	}
}

void CSQLiteRecordset::CFieldExchange::AddSQL(LPCSTR psz)
{
	m_nField++;
	if (!m_strSQL.IsEmpty())
		m_strSQL += ',';
	m_strSQL += psz;
}

CStringW CSQLiteRecordset::CFieldExchange::NextImportField()
{
	int s = m_nStartField;
	CString strLim = ';';
	if (m_strImportLine[s] == '"')
		strLim = _T("\";");
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
