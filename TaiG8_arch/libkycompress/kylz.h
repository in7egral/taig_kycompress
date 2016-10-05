//
//  kylz.h
//  TaiG8_arch
//
//  Created by Vladimir Putin on 05.10.16.
//  Copyright © 2016 FriedApple Team. All rights reserved.
//

#ifndef kylz_hpp
#define kylz_hpp

#include <stdio.h>
#include <iostream>
#include "zlib.h"
#include "ZipArchive.h"
#include "ZipMemFileEx.h"
#include "Lz77.h"

#define READ_BUFFER_SIZE 0x10000

typedef struct {
    CZipMemFile *file;
    CZipArchive *arch;
    std::string str;
    int unk1;
    int unk2;
} CKY_context;

void kyap_crypt2(uint8_t *data, uint size);

#pragma pack(push, 1)
typedef struct {
    char magicSignature[12];
    u_long uncompressedSize;
} CKYHeader;

typedef struct {
    int hashSeed;
    bool isSpecialDecompression;
    int dataBlockSize;
    int lzUncompressedSize;
    unsigned int crc32_hash;
    int yetAnotherUnknownField;
} CKYDataHeader;
#pragma pack(pop)

class CKYZipFile {
public:
    virtual ~CKYZipFile() {}
    virtual int Open()
    {
        return 0;
    }
    virtual int Read(void *lpBuf, UINT nCount)
    {
        return m_zipFile.Read(lpBuf, nCount);
    }
    virtual size_t Write(void *lpBuf, UINT nCount)
    {
        m_zipFile.Write(lpBuf, nCount);
        return nCount;
    }
    virtual void Close() {
        m_zipFile.Close();
    }
    virtual ZIP_FILE_USIZE Seekw(int lOff, int nFrom)
    {
        return m_zipFile.Seek(lOff, nFrom);
    }
    CZipMemFile m_zipFile;
};

class CKYCPS : public CCompressKYModel {
public:
    CKYCPS(const char *path);
    virtual ~CKYCPS();
    int Attach(CKYZipFile *ckyZipFile, int accessType);
    int Read(char *dataBuffer, size_t dataBufferSize);
    u_long GetUncompressSize();
    
private:
    bool m_unknonwBoolField;
    CKYHeader m_ckyHeader;
    CKYZipFile *m_ckyZipFile;
    const char *m_path;
    BYTE *m_compressedPart;
    BYTE *m_compressedPart2;
    int m_accessType;
    //CCycleBuffer *m_cycleBuffer;
    CKYDataHeader m_ckyDataHeader;
};

int BKDRHashEX(const char *str, unsigned int seed);

#endif /* kylz_hpp */
