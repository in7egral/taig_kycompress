//
//  ZipMemFileEx.hpp
//  TaiG8_arch
//
//  Created by Vladimir Putin on 04.10.16.
//  Copyright © 2016 FriedApple Team. All rights reserved.
//

#ifndef ZipMemFileEx_hpp
#define ZipMemFileEx_hpp

#if _MSC_VER > 1000
#pragma once
#endif

#include "ZipMemFile.h"

extern uint8_t g_zipKey;

class ZIP_API CZipMemFileEx : public CZipMemFile
{
public:
#if defined _ZIP_IMPL_MFC && (_MSC_VER >= 1300 || _ZIP_FILE_IMPLEMENTATION != ZIP_ZFI_WIN)
    DECLARE_DYNAMIC(CZipMemFile)
#endif
    void Write(const void* lpBuf, UINT nCount);
    UINT Read(void* lpBuf, UINT nCount);
    virtual ~CZipMemFileEx(){Close();}
    
};

#endif /* ZipMemFileEx_hpp */
