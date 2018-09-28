#pragma once
#include "common.h"
#include <dirent.h>
#include <vector>
#include <iostream>

namespace NXFramework
{

struct DirEntry
{
	int  isDir;         // Folder flag
	int  isReadOnly;    // Read-only flag
	int  isHidden;      // Hidden file flag
	u8   name[256];     // File name
	char ext[4];        // File extension
	u64  size;          // File size
};

u64  GetFileSize    (const char* filename);
void GetSizeString  (char* string, u64 size);
int  Navigate       (char* cwd, DirEntry& entry, bool parent);
void PopulateFiles  (char* dir,
                     std::vector<DirEntry>&    dirEntries,
                     std::vector<std::string>& extFilter);
int  ScanDir        (const char *dir,
                     std::vector<dirent *>& namelist,
                     int (*compar)(const dirent **, const dirent **));
const char* GetFileExt(const char *filename);

}
