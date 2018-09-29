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

void        GetFileBasename (char *string, const char *filename);
void        GetSizeString   (char* string, u64 size);
u64         GetFileSize     (const char* filename);
const char* GetFileExt      (const char *filename);

int  Navigate       (char* cwd, DirEntry& entry, bool parent);
void PopulateFiles  (const char* dir,
                     std::vector<DirEntry>&    dirEntries,
                     std::vector<std::string>& extFilter);
int  ScanDir        (const char *dir,
                     std::vector<dirent *>& namelist,
                     int (*compar)(const dirent **, const dirent **));

int  RmDirRecursive (const char* dir);

}
