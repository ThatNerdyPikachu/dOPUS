#pragma once
#include <common.h>
#include <string>
#include <Tinfoil/include/nx/content_meta.hpp>

namespace NXFramework
{

bool ExtractNSP(const std::string&  filename); //TODO
bool InstallNSP(const std::string&  filename,
                const FsStorageId   destStorageId        = FsStorageId_SdCard,
                const bool          ignoreReqFirmVersion = true);

bool InstallExtracted(const std::string&  filename, // TODO
                const FsStorageId   destStorageId        = FsStorageId_SdCard,
                const bool          ignoreReqFirmVersion = true);

}
