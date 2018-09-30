#pragma once
#include <common.h>
#include <string>
#include <Tinfoil/include/nx/content_meta.hpp>

namespace NXFramework
{

bool ExtractNSP_NCAs(   const std::string&  filename);
bool ExtractNSP     (   const std::string&  filename);
bool InstallNSP     (   const std::string&  filename,
                        const FsStorageId   destStorageId        = FsStorageId_SdCard,
                        const bool          ignoreReqFirmVersion = true,
                        const bool          isFolder             = false);
bool InstallExtracted(  const std::string&  filename,
                        const FsStorageId   destStorageId        = FsStorageId_SdCard,
                        const bool          ignoreReqFirmVersion = true);

}
