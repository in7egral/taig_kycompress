//
//  kylz.cpp
//  TaiG8_arch
//
//  Created by Николай Стрельцов on 05.10.16.
//  Copyright © 2016 FriedApple Team. All rights reserved.
//

#include "kylz.h"

void kyap_crypt2(uint8_t *data, uint size)
{
    if ( size ) {
        do {
            *data++ ^= g_zipKey;
            --size;
        }
        while ( size );
    }
}

int BKDRHashEX(const char *str, unsigned int seed)
{
    size_t length = strlen(str);
    uint hash = 0;
    if (length > 0) {
        //char *tmpBuffer = operator new[length];
        //
        int _seed = seed;
        //for (int i = 0; i < length; ++i) {
        _seed = 214013 * _seed + 2531094;
        //tmpBuffer[i] = str[i] ^ (seed >> 16);
        //}
        char firstChar = str[0] ^ (_seed >> 16);
        // Fake BKDRHash
        const int BKDRHash_seed = 131;
        hash = 0;
        uint hash1 = firstChar + (uint)length;
        while (length--) {
            hash = (hash * BKDRHash_seed) + hash1;
        }
        hash &= 0x7FFFFFFF;
        //delete [] tmpBuffer;
    }
    return hash;
}

CKYCPS::CKYCPS(const char *path) {
    m_ckyHeader.magicSignature[0] = 0xFFu;
    m_ckyHeader.magicSignature[1] = 0xDDu;
    m_ckyHeader.magicSignature[2] = 0xAAu;
    m_ckyHeader.magicSignature[3] = 0;
    *(uint *)&m_ckyHeader.magicSignature[4] = 0x7788;
    m_ckyHeader.uncompressedSize = 0LL;
    *(uint *)&m_ckyHeader.magicSignature[8] = 0x1C7;
    m_ckyDataHeader.hashSeed = 0x68686868;
    m_ckyDataHeader.isSpecialDecompression = 0;
    m_ckyDataHeader.dataBlockSize = 0;
    m_ckyDataHeader.crc32_hash = 0;
    m_ckyDataHeader.lzUncompressedSize = 0;
    this->m_ckyDataHeader.yetAnotherUnknownField = 44;
    if (path) {
        size_t pathLen = strlen(path);
        const char *filename = path;
        while (1) {
            if (pathLen == 0)
                break;
            if (path[pathLen - 1] != '/' && path[pathLen - 1] != '\\') {
                --pathLen;
                continue;
            }
            filename = &path[pathLen];
            break;
        }
        size_t filenameLegth = strlen(filename) + 1;
        char *newPath = new char[filenameLegth];
        bzero(newPath, filenameLegth);
        memcpy(newPath, filename, filenameLegth);
        m_path = newPath;
    } else {
        m_path = "1111";
    }
    m_accessType = 0;
    m_compressedPart = new BYTE[0x20000];
    m_compressedPart2 = new BYTE[0x20000];
    //m_cycleBuffer = new CCycleBuffer;
    m_ckyZipFile = 0;
    m_unknonwBoolField = false;
}
CKYCPS::~CKYCPS() {
    if (m_path)
        delete [] m_path;
    if (m_compressedPart)
        delete [] m_compressedPart;
    if (m_compressedPart2)
        delete [] m_compressedPart2;
}
int CKYCPS::Attach(CKYZipFile *ckyZipFile, int accessType)
{
    m_accessType = accessType;
    m_ckyZipFile = ckyZipFile;
    if ( accessType == 2 )
        ckyZipFile->Write((uint8_t *)&m_ckyHeader, sizeof(m_ckyHeader));
    else
        ckyZipFile->Read((uint8_t *)&m_ckyHeader, sizeof(m_ckyHeader));
    return 0;
}
int CKYCPS::Read(char *dataBuffer, size_t dataBufferSize) {
    if ( READ_BUFFER_SIZE > 0x10000 )
        return 0;
    if (m_ckyZipFile->Read(&m_ckyDataHeader, sizeof(m_ckyDataHeader)) >= sizeof(m_ckyDataHeader)) {
        int dataBlockSize = m_ckyDataHeader.dataBlockSize;
        uint compressedPartSize = dataBlockSize - sizeof(m_ckyDataHeader);
        if (m_ckyZipFile->Read(m_compressedPart, compressedPartSize) == compressedPartSize) {
            uint crc32_value = 0;
            crc32_value = (uint)zarch_crc32(crc32_value, (const unsigned char *)m_compressedPart, compressedPartSize);
            if (crc32_value == m_ckyDataHeader.crc32_hash) {
                // get path hash
                unsigned int hash = BKDRHashEX(m_path, m_ckyDataHeader.hashSeed);
                // decode archive data
                if ( compressedPartSize > 0 )
                {
                    uint8_t *ptr = (uint8_t *)m_compressedPart;
                    for (int i = 0; i < compressedPartSize; ++i) {
                        hash = 214013 * hash + 2531094;
                        *ptr++ ^= hash >> 16;
                    }
                }
                BYTE *dataPtr;
                int dataSize;
                if (m_ckyDataHeader.isSpecialDecompression) {
                    if (!Decompress(m_compressedPart2, m_ckyDataHeader.lzUncompressedSize, m_compressedPart, compressedPartSize))
                        return 0;
                    dataPtr = m_compressedPart2;
                    dataSize = m_ckyDataHeader.lzUncompressedSize;
                } else {
                    dataPtr = m_compressedPart;
                    dataSize = compressedPartSize;
                }
                memcpy(dataBuffer, dataPtr, dataSize);
                return dataSize;
            }
        }
    }
    return 0;
}
u_long CKYCPS::GetUncompressSize() {
    return m_ckyHeader.uncompressedSize;
}
