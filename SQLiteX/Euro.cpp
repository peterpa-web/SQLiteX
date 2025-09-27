#include "pch.h"
#include "Euro.h"

CEuro::CEuro(double e)
{
    double d = e * 100;
    d += (d >= 0) ? 0.5 : -0.5;
    m_nCents = (long)d;
}

CString CEuro::ToString(int nCents)
{
    CString str;
    str.Format(_T("%d,%2.2d"), nCents / 100, (nCents < 0 ? -nCents : nCents) % 100);
    return str;
}

CString CEuro::EurToStringDots(int nEur)
{
    CString str;
    LPWSTR pBuf = str.GetBufferSetLength(33);
    _itow_s(nEur, pBuf, 33, 10);
    str.ReleaseBuffer();
    int m = 0;
    if (str[0] == '-')
        m = 1;
    int p = str.GetLength();
    p -= 3;
    while (p > m)
    {
        str = str.Left(p) + '.' + str.Mid(p);
        p -= 3;
    }
    return str;
}

CString CEuro::ToStringDots(int nCents)
{
    CString str = ToString(nCents);
    int m = 0;
    if (str[0] == '-')
        m = 1;
    int p = str.GetLength() - 3;    // pos of decimal sign
    ASSERT(str[p] == ',');
    p -= 3;
    while (p > m)
    {
        str = str.Left(p) + '.' + str.Mid(p);
        p -= 3;
    }
    return str;
}

void CEuro::FromString(const CStringA& str)
{
    
    int p = str.FindOneOf(".,");
    if (p >= 0)
    {
        CStringA s(str);
        if (s[p] == ',')
            s.Replace(',', '.');
        double d = atof(s) * 100;
        d += (d >= 0) ? 0.5 : -0.5;
        m_nCents = (long)d;
    }
    else
        m_nCents = atol(str);
}

bool CEuro::IsInRange(int nPercent, const CEuro& obj) const
{
    long nDelta = nPercent * m_nCents / 100;
    long nMin = m_nCents;
    long nMax = m_nCents;
    if (m_nCents >= 0)
    {
        nMin -= nDelta;
        nMax += nDelta;
    }
    else {
        nMin += nDelta;
        nMax -= nDelta;
    }
    return obj.m_nCents >= nMin && obj.m_nCents <= nMax;
}

long CEuro::AbsDiff(const CEuro& obj) const
{
    long nDelta = m_nCents - obj.m_nCents;
    if (nDelta >= 0)
        return nDelta;
    return -nDelta;
}

