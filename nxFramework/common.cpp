#include "common.h"

void printBytes(u8 *bytes, size_t size, bool includeHeader)
{
    int count = 0;

    if (includeHeader)
    {
        printf("\n\n00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
        printf("-----------------------------------------------\n");
    }

    for (int i = 0; i < (int)size; i++)
    {
        printf("%02x ", bytes[i]);
        count++;
        if ((count % 16) == 0)
            printf("\n");
    }
    printf("\n");
}

/*
#include <map>
std::map<FILE*, std::string> openFiles;
FILE *do_fopen(const char *fn, const char *mode)
{
    FILE *f = (fopen)(fn, mode);
    fprintf(stderr, "HOOK: fopen(\"%s\", \"%s\") == %p\n", fn, mode, f);

    // Add the entry to the open file list
    openFiles[f] = std::string(fn);

    return f;
}

int do_fclose (FILE* f)
{
    // Remove the entry from the open file list
    auto elt = openFiles.find(f);
    if(elt != openFiles.end())
    {
        fprintf(stderr, "HOOK: fclose(%p) %s\n", f, elt->second.c_str());
        openFiles.erase(elt);
    }
    fprintf(stderr, "HOOK: open files: %d\n", (int)openFiles.size());
    return (fclose)(f);
}
*/

/*
#include <stdio.h>
#include <dlfcn.h>
FILE *(*original_fopen)(const char*, const char*) = NULL;

FILE *fopen(const char *path, const char *mode)
{
    printf("In our own fopen, opening %s\n", path);
    fflush(stdout);

    if (!original_fopen) {
        original_fopen = (FILE * (*)(const char*, const char*))dlsym(RTLD_NEXT, "fopen");
    }
    return (*original_fopen)(path, mode);

}
*/

