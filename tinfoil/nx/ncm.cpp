#include "nx/ncm.hpp"
#include "../../nxFramework/common.h"

namespace nx::ncm
{
    ContentStorage::ContentStorage(FsStorageId storageId) 
    {
        if(R_FAILED(ncmOpenContentStorage(storageId, &m_contentStorage)))
            LOG("Failed to open NCM ContentStorage");
    }

    ContentStorage::~ContentStorage()
    {
        serviceClose(&m_contentStorage.s);
    }

    void ContentStorage::CreatePlaceholder(const NcmNcaId &placeholderId, const NcmNcaId &registeredId, size_t size)
    {
        if(R_FAILED(ncmCreatePlaceHolder(&m_contentStorage, &placeholderId, &registeredId, size)))
            LOG("Failed to create placeholder");
    }
            
    void ContentStorage::DeletePlaceholder(const NcmNcaId &placeholderId)
    {
        if(R_FAILED(ncmDeletePlaceHolder(&m_contentStorage, &placeholderId)))
            LOG("Failed to delete placeholder");
    }

    void ContentStorage::WritePlaceholder(const NcmNcaId &placeholderId, u64 offset, void *buffer, size_t bufSize)
    {
        if(R_FAILED(ncmWritePlaceHolder(&m_contentStorage, &placeholderId, offset, buffer, bufSize)))
            LOG("Failed to write to placeholder");
    }

    void ContentStorage::Register(const NcmNcaId &placeholderId, const NcmNcaId &registeredId)
    {
        if(R_FAILED(ncmContentStorageRegister(&m_contentStorage, &registeredId, &placeholderId)))
            LOG("Failed to register placeholder NCA");
    }

    void ContentStorage::Delete(const NcmNcaId &registeredId)
    {
        if(R_FAILED(ncmDelete(&m_contentStorage, &registeredId)))
            LOG("Failed to delete registered NCA");
    }

    bool ContentStorage::Has(const NcmNcaId &registeredId)
    {
        bool hasNCA = false;
        if(R_FAILED(ncmContentStorageHas(&m_contentStorage, &registeredId, &hasNCA)))
            LOG("Failed to check if NCA is present");
        return hasNCA;
    }

    std::string ContentStorage::GetPath(const NcmNcaId &registeredId)
    {
        char pathBuf[FS_MAX_PATH] = {0};
        if(R_FAILED(ncmContentStorageGetPath(&m_contentStorage, &registeredId, pathBuf, FS_MAX_PATH)))
            LOG("Failed to get installed NCA path");
        return std::string(pathBuf);
    }
}
