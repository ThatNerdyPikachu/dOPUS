#include "nspHelper.h"
#include "../tinfoil/install/simple_filesystem.hpp"
#include "../tinfoil/nx/content_meta.hpp"
#include "../tinfoil/util/title_util.hpp"
#include <iostream>
#include <memory>

using namespace tin::install::nsp;

void InstallTicketCert(SimpleFileSystem& simpleFS)
{
    // Read the tik file and put it into a buffer
    auto tikName    = simpleFS.GetFileNameFromExtension("", "tik");     LOG("> Getting tik size\n");
    auto tikFile    = simpleFS.OpenFile(tikName);
    u64 tikSize     = tikFile.GetSize();
    auto tikBuf     = std::make_unique<u8[]>(tikSize);                  LOG("> Reading tik\n");
    tikFile.Read(0x0, tikBuf.get(), tikSize);

    // Read the cert file and put it into a buffer
    auto certName   = simpleFS.GetFileNameFromExtension("", "cert");    LOG("> Getting cert size\n");
    auto certFile   = simpleFS.OpenFile(certName);
    u64 certSize    = certFile.GetSize();
    auto certBuf    = std::make_unique<u8[]>(certSize);                 LOG("> Reading cert\n");
    certFile.Read(0x0, certBuf.get(), certSize);

    // Finally, let's actually import the ticket
    if(R_FAILED(esImportTicket(tikBuf.get(), tikSize, certBuf.get(), certSize)))
        LOG("Failed to import ticket");
}

void InstallContentMetaRecords(tin::util::ByteBuffer&   installContentMetaBuf,
                               nx::ncm::ContentMeta&    contentMeta,
                               const FsStorageId&       destStorageId)
{
    NcmContentMetaDatabase contentMetaDatabase;
    NcmMetaRecord contentMetaKey = contentMeta.GetContentMetaKey();
    try
    {
        if(R_FAILED(ncmOpenContentMetaDatabase(destStorageId, &contentMetaDatabase)))
            LOG("Failed to open content meta database");
        if(R_FAILED(ncmContentMetaDatabaseSet(&contentMetaDatabase, &contentMetaKey, installContentMetaBuf.GetSize(), (NcmContentMetaRecordsHeader*)installContentMetaBuf.GetData())))
            LOG("Failed to set content records");
        if(R_FAILED(ncmContentMetaDatabaseCommit(&contentMetaDatabase)))
            LOG("Failed to commit content records");
    }
    catch (std::runtime_error& e)
    {
        serviceClose(&contentMetaDatabase.s);
        throw e;
    }
}

void InstallApplicationRecord(nx::ncm::ContentMeta& contentMeta, const FsStorageId& destStorageId)
{
    Result rc = 0;
    std::vector<ContentStorageRecord> storageRecords;
    u64 baseTitleId = tin::util::GetBaseTitleId(
            contentMeta.GetContentMetaKey().titleId,
            static_cast<nx::ncm::ContentMetaType>(contentMeta.GetContentMetaKey().type));

    LOG("Base title Id: 0x%lx", baseTitleId);

    // TODO: Make custom error with result code field
    // 0x410: The record doesn't already exist
    u32 contentMetaCount = 0;
    if (R_FAILED(rc = nsCountApplicationContentMeta(baseTitleId, &contentMetaCount)) && rc != 0x410)
    {
        throw std::runtime_error("Failed to count application content meta");
    }
    LOG("Content meta count: %u\n", contentMetaCount);

    // Obtain any existing app record content meta and append it to our vector
    if (contentMetaCount > 0)
    {
        storageRecords.resize(contentMetaCount);
        size_t contentStorageBufSize    = contentMetaCount * sizeof(ContentStorageRecord);
        auto contentStorageBuf          = std::make_unique<ContentStorageRecord[]>(contentMetaCount);
        u32 entriesRead;

        if (R_FAILED(nsListApplicationRecordContentMeta(0, baseTitleId, contentStorageBuf.get(), contentStorageBufSize, &entriesRead)))
            LOG("Failed to list application record content meta");

        if (entriesRead != contentMetaCount)
        {
            throw std::runtime_error("Mismatch between entries read and content meta count");
        }
        memcpy(storageRecords.data(), contentStorageBuf.get(), contentStorageBufSize);
    }

    // Add our new content meta
    ContentStorageRecord storageRecord;
    storageRecord.metaRecord = contentMeta.GetContentMetaKey();
    storageRecord.storageId  = destStorageId;
    storageRecords.push_back(storageRecord);

    // Replace the existing application records with our own
    try
    {
        nsDeleteApplicationRecord(baseTitleId);
    }
    catch (...) {}
    LOG("Pushing application record...\n");
    if (R_FAILED(nsPushApplicationRecord(baseTitleId, 0x3, storageRecords.data(), storageRecords.size() * sizeof(ContentStorageRecord))))
        LOG("Failed to push application record");
}

/*
InstallNSP(std::string filename)
{
    try
    {
        nx::fs::IFileSystem fileSystem;
        fileSystem.OpenFileSystemWithId(filename.c_str(), FsFileSystemType_ApplicationPackage, 0);
        tin::install::nsp::SimpleFileSystem simpleFS(fileSystem, "/", filename.c_str() + "/");

        tin::install::nsp::NSPInstallTask task(simpleFS, m_destStorageId, m_ignoreReqFirmVersion);

        printf("Preparing install...\n");


/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

//        task.Prepare();
        tin::util::ByteBuffer cnmtBuf;
        auto cnmtTuple = this->ReadCNMT();
        m_contentMeta = std::get<0>(cnmtTuple);
        nx::ncm::ContentRecord cnmtContentRecord = std::get<1>(cnmtTuple);

        nx::ncm::ContentStorage contentStorage(m_destStorageId);

        if (!contentStorage.Has(cnmtContentRecord.ncaId))
        {
            printf("Installing CNMT NCA...\n");
            //InstallNCA(cnmtContentRecord.ncaId);
            /////////////////////////////////////////////
            /////////////////////////////////////////////

        }
        else
        {
            printf("CNMT NCA already installed. Proceeding...\n");
        }

        // Parse data and create install content meta
        if (m_ignoreReqFirmVersion)
            printf("WARNING: Required system firmware version is being IGNORED!\n");

        tin::util::ByteBuffer installContentMetaBuf;
        m_contentMeta.GetInstallContentMeta(installContentMetaBuf, cnmtContentRecord, m_ignoreReqFirmVersion);

        //this->InstallContentMetaRecords(installContentMetaBuf);
        //this->InstallApplicationRecord();

        printf("Installing ticket and cert...\n");
        try
        {
            //this->InstallTicketCert();
        }
        catch (std::runtime_error& e)
        {
            printf("WARNING: Ticket installation failed! This may not be an issue, depending on your usecase.\nProceed with caution!\n");
        }
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

        LOG("Pre Install Records: \n");
        //task.DebugPrintInstallData();

        task.Begin();

        LOG("Post Install Records: \n");
        //task.DebugPrintInstallData();
    }
    catch (std::exception& e)
    {
        LOG("Failed to install NSP!\n");
        LOG("%s", e.what());
        break;
    }
}




    void NSPInstallTask::InstallNCA(const NcmNcaId &ncaId)
    {
        std::string ncaName = tin::util::GetNcaIdString(ncaId);

        if (m_simpleFileSystem->HasFile(ncaName + ".nca"))
            ncaName += ".nca";
        else if (m_simpleFileSystem->HasFile(ncaName + ".cnmt.nca"))
            ncaName += ".cnmt.nca";
        else
        {
            throw std::runtime_error(("Failed to find NCA file " + ncaName + ".nca/.cnmt.nca").c_str());
        }

        LOG_DEBUG("NcaId: %s\n", ncaName.c_str());
        LOG_DEBUG("Dest storage Id: %u\n", m_destStorageId);

        nx::ncm::ContentStorage contentStorage(m_destStorageId);

        // Attempt to delete any leftover placeholders
        try
        {
            contentStorage.DeletePlaceholder(ncaId);
        }
        catch (...) {}

        auto ncaFile = m_simpleFileSystem->OpenFile(ncaName);
        size_t ncaSize = ncaFile.GetSize();
        u64 fileOff = 0;
        size_t readSize = 0x400000; // 4MB buff
        auto readBuffer = std::make_unique<u8[]>(readSize);

        if (readBuffer == NULL)
            throw std::runtime_error(("Failed to allocate read buffer for " + ncaName).c_str());

        LOG_DEBUG("Size: 0x%lx\n", ncaSize);
        contentStorage.CreatePlaceholder(ncaId, ncaId, ncaSize);

        float progress;

        while (fileOff < ncaSize)
        {
            // Clear the buffer before we read anything, just to be sure
            progress = (float)fileOff / (float)ncaSize;

            if (fileOff % (0x400000 * 3) == 0)
                printf("> Progress: %lu/%lu MB (%d%s)\r", (fileOff / 1000000), (ncaSize / 1000000), (int)(progress * 100.0), "%");

            if (fileOff + readSize >= ncaSize) readSize = ncaSize - fileOff;

            ncaFile.Read(fileOff, readBuffer.get(), readSize);
            contentStorage.WritePlaceholder(ncaId, fileOff, readBuffer.get(), readSize);
            fileOff += readSize;
        }

        // Clean up the line for whatever comes next
        printf("                                                           \r");
        printf("Registering placeholder...\n");

        try
        {
            contentStorage.Register(ncaId, ncaId);
        }
        catch (...)
        {
            printf(("Failed to register " + ncaName + ". It may already exist.\n").c_str());
        }

        try
        {
            contentStorage.DeletePlaceholder(ncaId);
        }
        catch (...) {}
    }
*/
