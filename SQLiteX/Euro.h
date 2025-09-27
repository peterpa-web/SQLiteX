#pragma once
class CEuro
{
public:
	CEuro() {}
	CEuro(long nCents) { m_nCents = nCents; }
	CEuro(double e);
	static CString ToString(int nCents);			// euro,cents
	CString ToString() { return ToString(m_nCents); }
	static CString EurToStringDots(int nEur);
	static CString ToStringDots(int nCents);		// xxx.euro,cents
	CString ToStringDots() { return ToStringDots(m_nCents); }
	void FromString(const CStringA& str);	// read cents or euro if containing '.' or ','
	double ToDouble() { return ((double)m_nCents) / 100; }
	long& GetCentsRef() { return m_nCents; }
	CEuro operator +=(const CEuro& obj) { m_nCents += obj.m_nCents; return CEuro(m_nCents); }
	bool operator ==(const CEuro& obj) const { return m_nCents == obj.m_nCents; }
	bool IsInRange(int nPercent, const CEuro& obj) const;
	long AbsDiff(const CEuro& obj) const;	// returns cents diff

protected:
	long m_nCents = 0;
};

