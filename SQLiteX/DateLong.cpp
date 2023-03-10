#include "pch.h"
#include "DateLong.h"

// stored as int: YYYYMMDD
// import d.m.yyyy hh:mm:ss or yyyy-mm-dd hh:mm:ss

CDateLong::CDateLong(const CStringA& str)
{
	CStringA s;
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
				int m = atoi(s);
				s = str.Tokenize(" ", p);
				int y = atoi(s);
				m_nValue = CDateLong(y, m, d).ToLong();
				return;
			}
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
				int m = atoi(s);
				s = str.Tokenize(" ", p);
				int d = atoi(s);
				m_nValue = CDateLong(y, m, d).ToLong();
				return;
			}
		}
	}
	m_nValue = atoi(str);
}

CTime CDateLong::ToTime()
{
	if (m_nValue == 0)
		return 0;

	return CTime(GetYear(), GetMonth(), GetDay(), 0, 0, 0);
}

COleDateTime CDateLong::ToOleDateTime()
{
	if (m_nValue == 0)
		return COleDateTime();

	return COleDateTime(GetYear(), GetMonth(), GetDay(), 0, 0, 0);
}

CStringA CDateLong::ToSQL()
{
	if (m_nValue == 0)
		return "NULL";

	CStringA s;
	s.Format("%d", m_nValue);
	return s;
}

CStringA CDateLong::ToStringIntA()
{
	CStringA s;
	s.Format("%4.4d-%2.2d-%2.2d", GetYear(), GetMonth(), GetDay());
	return s;
}

CStringA CDateLong::ToStringGerA()
{
	CStringA s;
	s.Format("%2.2d.%2.2d.%4.4d", GetDay(), GetMonth(), GetYear());
	return s;
}
