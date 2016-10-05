//
//  ZipMemFileEx.cpp
//  TaiG8_arch
//
//  Created by Vladimir Putin on 04.10.16.
//  Copyright © 2016 FriedApple Team. All rights reserved.
//

#include "stdafx.h"
#include "ZipMemFileEx.h"
#include "ZipException.h"

uint8_t g_zipKey = 0x58;

#if defined _ZIP_IMPL_MFC && (_MSC_VER >= 1300 || _ZIP_FILE_IMPLEMENTATION != ZIP_ZFI_WIN)
IMPLEMENT_DYNAMIC(CZipMemFileEx, CFile)
#endif

UINT CZipMemFileEx::Read(void *lpBuf, UINT nCount)
{
    if (m_nPos >= m_nDataSize)
        return 0;
    UINT nToRead = (m_nPos + nCount > m_nDataSize) ? (UINT)(m_nDataSize - m_nPos) : nCount;
    memcpy(lpBuf, m_lpBuf + m_nPos, nToRead);
    UINT toDecrypt = nToRead;
    uint8_t *buf = static_cast<uint8_t *>(lpBuf);
    while (toDecrypt) {
        *buf++ ^= g_zipKey;
        --toDecrypt;
    }
    m_nPos += nToRead;
    return nToRead;
    
}

void CZipMemFileEx::Write(const void *lpBuf, UINT nCount)
{
    if (!nCount)
        return;
    
    if (m_nPos + nCount > m_nBufSize)
        Grow(m_nPos + nCount);
    memcpy(m_lpBuf + m_nPos, lpBuf, nCount);
    UINT toEncrypt = nCount;
    uint8_t *buf = static_cast<uint8_t *>(m_lpBuf + m_nPos);
    while (toEncrypt) {
        *buf++ ^= g_zipKey;
        --toEncrypt;
    }
    m_nPos += nCount;
    if (m_nPos > m_nDataSize)
        m_nDataSize = m_nPos;
}

