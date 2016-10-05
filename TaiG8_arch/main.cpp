//
//  main.cpp
//  TaiG8_arch
//
//  Created by Vladimir Putin on 04.10.16.
//  Copyright © 2016 FriedApple Team. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

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

char * Pack_GetFileName(CKY_context *package_handle, ZIP_INDEX_TYPE index, BOOL *isDirectory)
{
    char *result = 0;
    if ( package_handle ) {
        if ( package_handle->arch ) {
            // get file info
            CZipFileHeader *fileInfo = package_handle->arch->GetFileInfo(index);
            if (fileInfo ) {
                std::string fileName = fileInfo->GetFileName(true);
                size_t size = fileName.size();
                result = new char[size + 1];
                bzero(result, size + 1);
                memcpy(result, fileName.c_str(), size);
                if ( isDirectory )
                    *isDirectory = fileInfo->IsDirectory();
            }
        }
    }
    return result;
}

ZIP_INDEX_TYPE Pack_GetFileCount(CKY_context *context)
{
    ZIP_INDEX_TYPE result;

    result = 0;
    if ( context ) {
        if ( context->arch ) {
            result = context->arch->GetCount();
        }
    }
    return result;
}

void writeFile(const char *name, char *buffer, size_t size)
{
    char *pos;
    if ( (pos = strchr(name, '/')) != NULL ) {
        char *path = new char[strlen(name) + 1];
        while (pos) {
            size_t length = pos - name;
            strncpy(path, name, length);
            path[length] = 0;
            struct stat st;
            if (stat(path, &st))
                mkdir(path, 0755);
            pos = strchr(pos + 1, '/');
        }
        delete [] path;
    }
    std::fstream f;
    f.open(name, std::ios_base::out | std::ios::trunc | std::ios::binary);
    if (f.is_open()) {
        f.write(buffer, size);
        f.close();
    }
}

bool CrackPackFile(CKY_context *context)
{
    // read list of files
    std::string str = "__opt__";
    size_t size;
    char *fileList = Pack_GetFileData(context, str.c_str(), &size);
    writeFile(str.c_str(), fileList, size);
    std::string strFileList = fileList;
    // TODO: check that all files are here
    return true;
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
            ZIP_INDEX_TYPE count = Pack_GetFileCount(context);
            std::cout << "Files count in the archive: " << count << "\n";
            std::cout << "File names are:\n";
            for (ZIP_INDEX_TYPE i = 0; i < count; ++i) {
                char *name = Pack_GetFileName(context, i, 0);
                size_t size;
                char *buffer = Pack_GetFileData(context, name, &size);
                writeFile(name, buffer, size);
                delete [] buffer;
                std::cout << name << "\n";
                delete [] name;
            }
        } else {
            std::cout << "CrackPackFile failed..\n";
        }
    }
    std::cout << "Have a nice day!\n";
    return 0;
}
