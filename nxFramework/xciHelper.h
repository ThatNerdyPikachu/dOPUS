#pragma once
#include <string>
#include <switch.h>

struct Progress;

namespace NXFramework
{

int ExtractXCI( const std::string&  filename,
                const bool          saveNSP              = false,
                Progress*           progress             = nullptr);
int ConvertXCI( const std::string&  filename,
                Progress*           progress             = nullptr);
int InstallXCI( const std::string&  filename,
                const FsStorageId   destStorageId        = FsStorageId_SdCard,
                const bool          ignoreReqFirmVersion = true,
                const bool          deleteXCI            = false,
                Progress*           progress             = nullptr);

}
