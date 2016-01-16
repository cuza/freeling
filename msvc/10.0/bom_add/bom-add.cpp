//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2004   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    General Public License for more details.
//
//    You should have received a copy of the GNU General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
//
//   Author: Stanilovsky Evgeny 
//
///////////////////////////////////////////////
#include <windows.h>
#include <tchar.h> 
#include <strsafe.h>
#include <string>
#include <set>
#include <fstream>
#include <iostream>
#pragma comment(lib, "User32.lib")

/* List the files in "dir_name". */

static void list_dir(std::wstring dir_name, std::set<std::wstring> &set)
{
    HANDLE hFind;
    WIN32_FIND_DATA ffd;
    static const std::wstring SUFFIX[] = {L".cc", L".h"};
    static const std::size_t SUFFIX_SIZE = sizeof(SUFFIX)/sizeof(std::wstring);

    std::wstring szDir = dir_name;

    hFind = FindFirstFile((szDir + L"\\*").c_str(), &ffd);

    if (INVALID_HANDLE_VALUE == hFind) 
        return;

    do
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (wcscmp(ffd.cFileName, L".") && wcscmp(ffd.cFileName, L".."))
                list_dir(szDir + L"\\" + std::wstring(ffd.cFileName), set);
        }
        else
        {
            std::wstring fileName = std::wstring(ffd.cFileName);
            for (int i=0; i<SUFFIX_SIZE; ++i)
            {
                std::size_t pos = fileName.rfind(SUFFIX[i]);
                if (pos != std::wstring::npos && pos + SUFFIX[i].size() == fileName.size())
                    set.insert(szDir + L"\\" + fileName);
            }
        }
   }
   while (FindNextFile(hFind, &ffd) != 0);
   FindClose(hFind);
}

int _tmain(int argc, wchar_t *argv[])
{
    if(argc != 2)
    {
        _tprintf(TEXT("\nbom_add: adds BOM prefix (http://en.wikipedia.org/wiki/Byte_order_mark) into all *.cc and *.h files in directory\n Usage: %s <directory name>\n"), argv[0]);
        return -1;
    }
    size_t length_of_arg;
    StringCchLength(argv[1], MAX_PATH, &length_of_arg);

    if (length_of_arg > (MAX_PATH - 3))
    {
        _tprintf(TEXT("\nDirectory path is too long.\n"));
        return (-1);
    }

    _tprintf(TEXT("\nTarget directory is %s\n\n"), argv[1]);
    TCHAR szDir[MAX_PATH];
    StringCchCopy(szDir, MAX_PATH, argv[1]);

    std::set<std::wstring> sIn;
    list_dir (szDir, sIn);
    
    const char bom_line[3] = {(char)0xEF, (char)0xBB, (char)0xBF};
    char *line;
    for (std::set<std::wstring>::iterator It = sIn.begin(); It != sIn.end(); ++It)
    {        
        std::wstring w = *It;
        std::fstream file (w.c_str(), std::ios::in| std::ios::binary);
        file.seekg(0, std::ios_base::end);
        size_t pos = file.tellg();
        line = new char[pos+1];
        ZeroMemory(line, pos+1);
        file.seekg(0, std::ios_base::beg);
        file.read(line, pos);
        if (line[0]==bom_line[0] && line[1]==bom_line[1] && line[2]==bom_line[2])
            _tprintf(TEXT("already have BOM: %s\n"), w.c_str());
        else
        {
            file.seekp(0, std::ios_base::beg);
            _tprintf(TEXT("add BOM: %s\n"), w.c_str());            
            std::string tt;
            tt.resize(3);
            tt[0]=bom_line[0];
            tt[1]=bom_line[1];
            tt[2]=bom_line[2];
            tt+=line;
            file.close();
            std::fstream file (w.c_str(), std::ios::binary| std::ios::out);
            file.seekp(0, std::ios_base::beg);
            file << tt;
        }
        file.close();
        delete[] line;
    }
    return 0;
}