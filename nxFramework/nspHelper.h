#pragma once
#include <common.h>
#include <string>
#include <Tinfoil/include/nx/content_meta.hpp>

namespace tin::util
{
    class ByteBuffer;
}

namespace tin::install::nsp
{
    class SimpleFileSystem;
}

std::tuple<nx::ncm::ContentMeta , nx::ncm::ContentRecord> ReadCNMT(tin::install::nsp::SimpleFileSystem& simpleFS);
std::tuple<std::string          , nx::ncm::ContentRecord> GetCNMTNCAInfo        (std::string& nspPath);
nx::ncm::ContentMeta                                      GetContentMetaFromNCA (std::string& ncaPath);

void InstallContentMetaRecords( tin::util::ByteBuffer&                      installContentMetaBuf,
                                nx::ncm::ContentMeta&                       contentMeta,
                                const FsStorageId                           destStorageId);

void InstallApplicationRecord(  nx::ncm::ContentMeta&                       contentMeta,
                                const FsStorageId                           destStorageId);

void InstallTicketCert(         tin::install::nsp::SimpleFileSystem&        simpleFS);

void InstallNCA(                tin::install::nsp::SimpleFileSystem&        simpleFS,
                                const NcmNcaId&                             ncaId,
                                const FsStorageId                           destStorageId);

bool InstallNSP(                const std::string&                          filename,
                                const FsStorageId                           destStorageId        = FsStorageId_SdCard, // FsStorageId_SdCard or FsStorageId_NandUser
                                const bool                                  ignoreReqFirmVersion = true);

void DebugPrintInstallData(     nx::ncm::ContentMeta& contentMeta,
                                const FsStorageId destStorageId);
