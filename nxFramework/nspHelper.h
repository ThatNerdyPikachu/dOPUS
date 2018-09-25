#pragma once
#include <common.h>
#include <string>

namespace tin::util
{
    class ByteBuffer;
}
namespace tin::install::nsp
{
    class SimpleFileSystem;
}

bool InstallNSP(std::string filename);
void InstallContentMetaRecords(tin::util::ByteBuffer& installContentMetaBuf);
void InstallApplicationRecord();
void InstallTicketCert(tin::install::nsp::SimpleFileSystem& simpleFS);
void InstallNCA(const NcmNcaId &ncaId);
