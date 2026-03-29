#include "pch.h"
#include "SQLiteTypes.h"

#include "SQLiteSchema.h"

//    CREATE TABLE sqlite_schema(
//        type text,
//        name text,
//        tbl_name text,
//        rootpage integer,
//        sql text
//    );

CSQLiteSchema::~CSQLiteSchema()
{
	ResetTables();
}

bool CSQLiteSchema::ReadAll(const CString& strDbPath)
{
	ResetTables();
	CFileStatus fs;
	if (!CFile::GetStatus(strDbPath, fs) || (fs.m_attribute & CFile::directory) != 0)
		return false;

	CSQLiteDatabase db;
	bool b = db.Open(strDbPath);
	CSQLiteSchemaInt schemaInt(&db);
	schemaInt.m_strFilter = L"type=\"table\" OR type=\"view\"";
	schemaInt.Open();
	while (!schemaInt.IsEOF())
	{
		CSQLiteTable* pT = new CSQLiteTable;
		pT->m_TblName = schemaInt.m_TblName;
		CString strName = pT->m_TblName;
		if (strName[0] == '[')
			strName = strName.Mid(1, strName.GetLength() - 2);
		pT->m_FileName = L"Rec" + strName.Left(1).MakeUpper() + strName.Mid(1);
		pT->m_ClassName = 'C' + pT->m_FileName;
		CString strSql = pT->m_Sql = schemaInt.m_Sql;
		int p = 0;
		if (schemaInt.m_Type == L"table")
		{
			CString strStart = strSql.Tokenize(L"(", p);
			if (p > 0)
			{
				CString strFields = strSql.Mid(p, strSql.GetLength() - p - 1);
				pT->ParseFields(strFields);
			}
		}
		else if (schemaInt.m_Type == L"view")
		{
			pT->m_bView = true;
//			pT->m_Sql = schemaInt.m_Sql;
			CSQLiteTableInfo ti(&db);
			ti.OpenTable(pT->m_TblName);
			while (!ti.IsEOF())
			{
				pT->AddField(ti);
				ti.MoveNext();
			}
		}
		m_tables.AddTail(pT);
		schemaInt.MoveNext();

		CSQLiteSchemaInt schemaInt2(&db);
		schemaInt2.m_strFilter = L"type=\"index\" AND tbl_name=$tbl_name";
		schemaInt2.m_TblName = pT->m_TblName;
		schemaInt2.Open();
		while (!schemaInt2.IsEOF())
		{
			CSQLiteIndex si;
			si.m_Name = schemaInt2.m_Name;
			si.m_Sql = schemaInt2.m_Sql;
			pT->m_idx.AddTail(si);
			schemaInt2.MoveNext();
		}
	}
	schemaInt.Close();
	return true;
}

void CSQLiteSchema::ResetTables()
{
	POSITION pos = m_tables.GetTailPosition();
	while (pos != NULL)
	{
		POSITION p = pos;
		CSQLiteTable* pTable = m_tables.GetPrev(pos);
		delete pTable;
		m_tables.RemoveAt(p);
	}
}



CSQLiteSchemaInt::CSQLiteSchemaInt(CSQLiteDatabase* pdb)
	: CSQLiteRecordset(pdb)
{
}

CString CSQLiteSchemaInt::GetDefaultSQL()
{
	return L"sqlite_schema";
}

void CSQLiteSchemaInt::DoFieldExchange(CFieldExchange* pFX)
{
	RFX_Text(pFX, L"type", m_Type);
	RFX_Text(pFX, L"name", m_Name);
	RFX_Text(pFX, L"tbl_name", m_TblName);
	RFX_Long(pFX, L"rootpage", m_RootPage);
	RFX_Text(pFX, L"sql", m_Sql, FX_NN);
}

void CSQLiteTable::ParseFields(const CString& strFields)
{
	int p = 0;
	CString strField;;
	while (!(strField = strFields.Tokenize(L",(", p)).IsEmpty())
	{
		strField = strField.Trim();
		if (strField.IsEmpty())
			continue;
		if (strField.Left(11).CompareNoCase(L"PRIMARY KEY") == 0)
		{
			CString strField2 = strFields.Tokenize(L")", p);
			if (strField2.Find(',') < 0)
			{
				CSQLiteField* pF = FindField(strField2);
				if (pF != nullptr)
				{
					pF->m_dwFlags |= FX_PK;
					continue;
				}
			}
			AddContraints(strField + '(' + strField2 + ')');
			continue;
		}
		if (strField.Left(6).CompareNoCase(L"UNIQUE") == 0)
		{
			CString strField2 = strFields.Tokenize(L")", p);
			CSQLiteField* pF = FindField(strField2);
			if (pF != nullptr)
			{
				pF->m_dwFlags |= FX_UN;
				continue;
			}
			AddContraints(strField + '(' + strField2 + ')');
			continue;
		}
		if (strField.Left(11).CompareNoCase(L"FOREIGN KEY") == 0)
		{
			CString strField2 = strFields.Tokenize(L")", p);
			CString strField3 = strFields.Tokenize(L")", p);
			AddContraints(strField + '(' + strField2 + ')' + strField3 + ')');
			continue;
		}
		if (strField.Left(5).CompareNoCase(L"CHECK") == 0)
		{
			CString strField2 = strFields.Tokenize(L")", p);
			AddContraints(strField + '(' + strField2 + ')');
			continue;
		}

		CSQLiteField field;
		int q = 0;
		if (strField[0] == '"')
		{
			q = strField.Find('"', 1);
			field.m_SqlName = strField.Mid(1, q - 1);
			while (strField[++q] == ' ' || strField[q] == '\t');
		}
		else
			field.m_SqlName = strField.Tokenize(L" \t", q);
		field.m_SqlTypeRaw = strField.Tokenize(L" ", q);
		field.m_nSqlType = CSQLiteTypes::GetSqlType(field.m_SqlTypeRaw);
		field.SetDefaultType();
		field.m_SqlName = StripDeco(field.m_SqlName);
		field.InitVarName();
		if (q > 0)
		{
			CString strFlags = strField.Tokenize(L")", q);
			field.SetFlags(strFlags);
		}
		field.m_dwFlagsOld = field.m_dwFlags;
		m_fields.AddTail(field);
	}
}

void CSQLiteTable::AddField(const CSQLiteTableInfo& ti)
{
	CSQLiteField field;
	field.m_SqlName = ti.m_Name;
	field.m_SqlTypeRaw = ti.m_Type;
	field.m_nSqlType = CSQLiteTypes::GetSqlType(field.m_SqlTypeRaw);
	field.SetDefaultType();
	field.m_SqlName = StripDeco(field.m_SqlName);
	field.InitVarName();
	if (ti.m_NotNull)
		field.m_dwFlags |= FX_NN;
	if (ti.m_Pk)
		field.m_dwFlags |= FX_PK;
	field.m_dwFlagsOld = field.m_dwFlags;
	m_fields.AddTail(field);
}

void CSQLiteTable::ResetFields()
{
	POSITION pos = m_fields.GetHeadPosition();
	while (pos != NULL)
	{
		CSQLiteField& f = m_fields.GetNext(pos);
		f.m_dwModified = f.MOD_ALL;
		f.m_dwFlags = f.m_dwFlagsOld;
		f.InitVarName();
	}
}

void CSQLiteTable::FillList(CListCtrl& list)
{
	list.DeleteAllItems();
	if (m_ClassName.IsEmpty())
		return;
	POSITION pos = m_fields.GetHeadPosition();
	int i = 0;
	while (pos != NULL)
	{
		CSQLiteField& f = m_fields.GetNext(pos);
		int n = list.InsertItem(i, f.m_SqlName);
		CString s1 = f.m_SqlTypeRaw + ' ' + f.GetFlagsShort();
		if (f.m_dwModified != 0)
			s1 = L"* " + s1;
		list.SetItemText(i, 1, s1);
		list.SetItemText(i, 2, f.GetDescr());
		list.SetItemData(i++, (DWORD_PTR) &f);
	}
}

void CSQLiteTable::GetDefs(CStringList& list)
{
	POSITION posF = m_fields.GetHeadPosition();
	while (posF != NULL)
	{
		CSQLiteField& f = m_fields.GetNext(posF);
		CString s = f.GetDef();
		list.AddTail(s);
	}
}

void CSQLiteTable::GetFunctions(CStringList& list)
{
	POSITION posF = m_fields.GetHeadPosition();
	while (posF != NULL)
	{
		CSQLiteField& f = m_fields.GetNext(posF);
		CString s = f.GetFunction();
		list.AddTail(s);
	}
}

bool CSQLiteTable::GetOldDefs(const CString& strTargetPath)
{
	if (!m_OldClass.IsEmpty() && m_OldClass.GetOldClassName() == m_ClassName)
		return true;
	if (!m_OldClass.Read(strTargetPath, m_FileName))
		return false;
	m_ClassName = m_OldClass.GetOldClassName();
	m_dwModified = 0;
	POSITION posF = m_fields.GetHeadPosition();
	while (posF != NULL)
	{
		CSQLiteField& f = m_fields.GetNext(posF);
		const COldClass::CFldData* pd = m_OldClass.GetFldBySql(f.m_SqlName);
		if (pd != nullptr)
		{
			f.m_dwModified = 0;
			int nType = pd->m_nType;
			if (nType >= 0)
				f.m_nFktType = f.m_nFktTypeOld = nType;
		//	else
		//		f.m_dwModified |= f.MOD_TYPE;
			CString strFlags = pd->m_strFlags;
			DWORD dwFlags = f.GetFlagsFromClass(strFlags);
			f.m_dwFlags = f.m_dwFlagsOld = dwFlags;
		//	if (dwFlags != f.m_dwFlagsOld)
		//		f.m_dwModified |= f.MOD_FLAGS;
			CString strVar = pd->m_strVar;
			if (!strVar.IsEmpty())
				f.m_strVarName = f.m_strVarNameOld = strVar;
		//	else
		//		f.m_dwModified |= f.MOD_VAR;
		}
		else
			f.m_dwModified = f.MOD_ALL;
	}
	if (m_OldClass.GetFieldCount() != m_fields.GetCount())
		m_dwModified |= MOD_COUNT;
	return true;
}

void CSQLiteTable::SetDefClassName()
{
	CString strName = m_TblName;
	if (strName[0] == '[')
		strName = strName.Mid(1, strName.GetLength() - 2);
	m_FileName = L"Rec" + strName.Left(1).MakeUpper() + strName.Mid(1);
	m_ClassName = 'C' + m_FileName;
}

bool CSQLiteTable::IsModified()
{
	if (m_dwModified != 0)
	{
		TRACE2("IsModif %s %d\n", m_FileName, m_dwModified);
		return true;
	}
	POSITION pos = m_fields.GetHeadPosition();
	while (pos != NULL)
	{
		CSQLiteField& f = m_fields.GetNext(pos);
		if (f.m_dwModified != 0)
		{
			TRACE3("IsModif %s %s %d\n", m_FileName, f.m_SqlName, f.m_dwModified);
			return true;
		}
	}
	return false;
}

CString CSQLiteTable::StripDeco(CString strName)
{
	if (strName[0] == '[' || strName[0] == '\"')
		strName = strName.Mid(1, strName.GetLength() - 2);
	return strName;
}

CSQLiteField* CSQLiteTable::FindField(const CString& strName)
{
	CString strField = StripDeco(strName);
	POSITION pos = m_fields.GetHeadPosition();
	while (pos != NULL)
	{
		CSQLiteField* pF = &m_fields.GetNext(pos);
		if (strField.CompareNoCase(pF->m_SqlName) == 0)
			return pF;
	}
	return nullptr;
}

void CSQLiteTable::AddContraints(CString str)
{
	if (!m_strConstraintsQuoted.IsEmpty())
		m_strConstraintsQuoted += L",\\n\"\n\t\tL\"";
	str.Replace(L"\"", L"");		// or L"\\\""
	m_strConstraintsQuoted += str;
}

void CSQLiteField::SetDefaultType()
{
	m_nFktType = CSQLiteTypes::GetDefaultType(m_nSqlType);
}

void CSQLiteField::SetFlags(const CString& strFlags)
{
	if (strFlags.IsEmpty())
		return;

	CString strF = strFlags;
	strF.MakeLower();

	if (strF.Find(L"not null") >= 0)
		m_dwFlags |= FX_NN;
	if (strF.Find(L"primary key") >= 0)
		m_dwFlags |= FX_PK;
	if (strF.Find(L"autoincrement") >= 0)
		m_dwFlags |= FX_AN;
	if (strF.Find(L"unique") >= 0)
		m_dwFlags |= FX_UN;
}

DWORD CSQLiteField::GetFlagsFromClass(const CString& strFlags)
{
	if (strFlags.IsEmpty())
		return 0;

	CString strF = strFlags;
	DWORD dwFlags = 0;
	if (strF.Find(L"FX_NN") >= 0)
		dwFlags |= FX_NN;
	if (strF.Find(L"FX_PK") >= 0)
		dwFlags |= FX_PK;
	if (strF.Find(L"FX_AN") >= 0)
		dwFlags |= FX_AN;
	if (strF.Find(L"FX_UN") >= 0)
		dwFlags |= FX_UN;
	return dwFlags;
}

CString CSQLiteField::GetFlagsShort()
{
	CString s;
	if (m_dwFlags & FX_NN)
		s += 'N';
	if (m_dwFlags & FX_PK)
		s += 'P';
	if (m_dwFlags & FX_AN)
		s += 'A';
	if (m_dwFlags & FX_UN)
		s += 'U';
	return s;
}

CString CSQLiteField::GetDef()
{
	ASSERT(m_strVarName.GetLength() > 2);
	return CSQLiteTypes::GetDeclLine(m_nFktType, m_strVarName);
}

CString CSQLiteField::GetFunction()
{
	return CSQLiteTypes::GetFktLine(m_nFktType, m_SqlName, m_strVarName, m_dwFlags);
}

void CSQLiteField::InitVarName()
{
	CString strName = m_SqlName;
	strName.Replace(' ', '_');
	strName.Replace(':', '_');
	m_strVarName = L"m_" + strName;
}

CSQLiteTableInfo::CSQLiteTableInfo(CSQLiteDatabase* pdb)
	: CSQLiteRecordset(pdb)
{
}

CString CSQLiteTableInfo::GetDefaultSQL()
{
	return L"(table_info)";
}

void CSQLiteTableInfo::DoFieldExchange(CFieldExchange* pFX)
{
	RFX_Long(pFX, L"cid", m_Cid);
	RFX_Text(pFX, L"name", m_Name);
	RFX_Text(pFX, L"type", m_Type);
	RFX_Bool(pFX, L"notnull", m_NotNull);
	RFX_Text(pFX, L"dflt_value", m_Dflt_Value);
	RFX_Bool(pFX, L"pk", m_Pk);
}
