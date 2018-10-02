#pragma once
#include <string>
#include <Tinfoil/include/nx/fs.hpp>

struct Progress;

namespace NXFramework
{

bool ExtractNSP     (   const std::string&  filename,
                        Progress*           progress             = nullptr);
bool InstallNSP     (   const std::string&  filename,
                        const FsStorageId   destStorageId        = FsStorageId_SdCard,
                        const bool          ignoreReqFirmVersion = true,
                        const bool          isFolder             = false,
                        Progress*           progress             = nullptr);
bool InstallExtracted(  const std::string&  filename,
                        const FsStorageId   destStorageId        = FsStorageId_SdCard,
                        const bool          ignoreReqFirmVersion = true,
                        Progress*           progress             = nullptr);

}
