#pragma once
#include "ATLComTime.h"

class CDateLong
{
public:
	CDateLong() {}
	CDateLong(const CDateLong& dl) { m_nValue = dl.m_nValue; }
	CDateLong(long n) { m_nValue = n; }
	CDateLong(int y, int m, int d) { m_nValue = (y * 100 + m) * 100 + d; }
	CDateLong(CTime t) { m_nValue = (t.GetYear() * 100 + t.GetMonth()) * 100 + t.GetDay(); }
	CDateLong(COleDateTime t) { m_nValue = (t.GetYear() * 100 + t.GetMonth()) * 100 + t.GetDay(); }
	CDateLong(const CStringA& str);
	int GetYear() { return m_nValue / 10000; }
	int GetMonth() { return (m_nValue / 100) % 100; }
	int GetDay() { return m_nValue % 100; }
	long ToLong() { return m_nValue; }
	CTime ToTime();
	COleDateTime ToOleDateTime();
	CStringA ToSQL();
	CStringA ToStringIntA();
	CString ToStringInt() { return CString(ToStringIntA()); }
	CStringA ToStringGerA();
	CString ToStringGer() { return CString(ToStringGerA()); }

protected:
	long m_nValue = 0;
};

