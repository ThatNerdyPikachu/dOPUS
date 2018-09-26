#include "nx/fs.hpp"

#include "../../nxFramework/common.h"
#include <cstring>
#include <stdexcept>
#include "nx/ipc/tin_ipc.h"

namespace nx::fs
{
    // IFile

    IFile::IFile(FsFile& file)
    {
        m_file = file;
    }

    IFile::~IFile()
    {
        fsFileClose(&m_file);
    }

    void IFile::Read(u64 offset, void* buf, size_t size)
    {
        u64 sizeRead;
        if(R_FAILED(fsFileRead(&m_file, offset, buf, size, &sizeRead)))
            LOG("Failed to read file\n");

        if (sizeRead != size)
        {
            std::string msg = "Size read " + std::string("" + sizeRead) + " doesn't match expected size " + std::string("" + size);
            throw std::runtime_error(msg.c_str());
        }
    }

    u64 IFile::GetSize()
    {
        u64 sizeOut;
        if(R_FAILED(fsFileGetSize(&m_file, &sizeOut)))
            LOG("Failed to get file size\n");
        return sizeOut;
    }

    // End IFile

    // IDirectory
    IDirectory::IDirectory(FsDir& dir)
    {
        m_dir = dir;
    }

    IDirectory::~IDirectory()
    {
        fsDirClose(&m_dir);
    }

    void IDirectory::Read(u64 inval, FsDirectoryEntry* buf, size_t numEntries)
    {
        size_t entriesRead;
        if(R_FAILED(fsDirRead(&m_dir, inval, &entriesRead, numEntries, buf)))
        	LOG("Failed to read directory\n");

        /*if (entriesRead != numEntries)
        {
            std::string msg = "Entries read " + std::string("" + entriesRead) + " doesn't match expected number " + std::string("" + numEntries);
            throw std::runtime_error(msg);
        }*/
    }

    u64 IDirectory::GetEntryCount()
    {
        u64 entryCount = 0;
        if(R_FAILED(fsDirGetEntryCount(&m_dir, &entryCount)))
            LOG("Failed to get entry count\n");
        return entryCount;
    }

    // End IDirectory

    IFileSystem::IFileSystem() {}

    IFileSystem::~IFileSystem()
    {
        this->CloseFileSystem();
    }

    Result IFileSystem::OpenSdFileSystem()
    {
        if(R_FAILED(fsMountSdcard(&m_fileSystem)))
            LOG("Failed to mount sd card\n");
        return 0;
    }

    void IFileSystem::OpenFileSystemWithId(std::string path, FsFileSystemType fileSystemType, u64 titleId)
    {
        Result rc = 0;
        if (path.length() >= FS_MAX_PATH)
            throw std::runtime_error("Directory path is too long!");

        // libnx expects a FS_MAX_PATH-sized buffer
        path.reserve(FS_MAX_PATH);

        std::string errorMsg = "Failed to open file system with id: " + path + "\n";
        rc = fsOpenFileSystemWithId(&m_fileSystem, titleId, fileSystemType, path.c_str());

        if (rc == 0x236e02)
            errorMsg = "File " + path + " is unreadable! You may have a bad dump, fs_mitm may need to be removed, or your firmware version may be too low to decrypt.\n";

        if(R_FAILED(rc))
            LOG(errorMsg.c_str());
    }

    void IFileSystem::CloseFileSystem()
    {
        fsFsClose(&m_fileSystem);
    }

    IFile IFileSystem::OpenFile(std::string path)
    {
        if (path.length() >= FS_MAX_PATH)
            throw std::runtime_error("Directory path is too long!");

        // libnx expects a FS_MAX_PATH-sized buffer
        path.reserve(FS_MAX_PATH);

        FsFile file;
        if(R_FAILED(fsFsOpenFile(&m_fileSystem, path.c_str(), FS_OPEN_READ, &file)))
        {
            LOG(("Failed to open file " + path + "\n").c_str());
            throw std::runtime_error(("Failed to open file " + path + "\n").c_str());
        }
        return IFile(file);
    }

    IDirectory IFileSystem::OpenDirectory(std::string path, int flags)
    {
        // Account for null at the end of c strings
        if (path.length() >= FS_MAX_PATH)
            throw std::runtime_error("Directory path is too long!");

        // libnx expects a FS_MAX_PATH-sized buffer
        path.reserve(FS_MAX_PATH);

        FsDir dir;
        if(R_FAILED(fsFsOpenDirectory(&m_fileSystem, path.c_str(), flags, &dir)))
            LOG(("Failed to open directory " + path + "\n").c_str());
        return IDirectory(dir);
    }
}
