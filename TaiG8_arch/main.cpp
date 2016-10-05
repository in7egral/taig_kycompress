//
//  main.cpp
//  TaiG8_arch
//
//  Created by Vladimir Putin on 04.10.16.
//  Copyright © 2016 FriedApple Team. All rights reserved.
//

#include <iostream>
#include <fstream>

#include "kylz.h"

char* Pack_GetFileData(CKY_context *context, const char *fileName, size_t *size)
{
    CZipArchive *zipArchive;
    char tmpBuf[READ_BUFFER_SIZE];
    if ( context && (zipArchive = context->arch) != 0 ) {
        ZIP_INDEX_TYPE index;
        if ( ZIP_FILE_INDEX_NOT_FOUND != (index = zipArchive->FindFile(fileName)) ) {
            CKYZipFile ckyZipFile;
            zipArchive->ExtractFile(index, ckyZipFile.m_zipFile);
            CKYCPS ckyCPS(fileName);
            ckyCPS.Attach(&ckyZipFile, 1);
            size_t uncompressSize = ckyCPS.GetUncompressSize();
            *size = uncompressSize;
            char *buffer = new char[uncompressSize];
            char *ptr = buffer;
            while (1) {
                int read = ckyCPS.Read(tmpBuf, READ_BUFFER_SIZE);
                if (read < 0)
                    break;
                memcpy(ptr, tmpBuf, read);
                if (read < READ_BUFFER_SIZE)
                    break;
                ptr += read;
            }
            return buffer;
        }
    }
    return 0;
}

bool CrackPackFile(CKY_context *context)
{
    // read list of files
    std::string str = "__opt__";
    size_t size;
    char *fileList = Pack_GetFileData(context, str.c_str(), &size);
    std::fstream f;
    f.open("/Users/admin/Downloads/taig_data.bin", std::ios_base::out | std::ios::trunc | std::ios::binary);
    if (f.is_open()) {
        f.write(fileList, size);
        f.close();
    }
    std::string strFileList = fileList;
    return false;
}

int main(int argc, const char * argv[]) {
    if ( argc < 2 ) {
        std::cout << "Usage:" << argv[0] << "<zipFile>\n";
        return 1;
    }
    std::ifstream f;
    f.open(argv[1]);
    if ( !f.is_open() )
        return 2;
    f.seekg (0, f.end);
    size_t length = f.tellg();
    f.seekg (0, f.beg);
    // buffer
    char * buffer = new char [length];
    // read data
    f.read(buffer, length);
    // close file
    f.close();
    // create memory file
    CZipMemFileEx *memFile = new CZipMemFileEx;
    memFile->Attach((uint8_t*)buffer, (uint)length);
    CZipArchive *zipArch = new CZipArchive;
    if ( zipArch->Open(*memFile) ) {
        std::cout << "Zip file is opened\n";
        CKY_context *context = new CKY_context;
        context->arch = zipArch;
        context->file = memFile;
        
        if ( CrackPackFile(context) ) {
            std::cout << "CrackPackFile ok!\n";
        } else {
            std::cout << "CrackPackFile failed..\n";
        }
    }
    std::cout << "Hello, World!\n";
    return 0;
}
