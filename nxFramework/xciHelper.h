#pragma once
#include <string>
#include <switch.h>

namespace NXFramework
{

int ExtractXCI( const std::string&  filename,
                const bool          saveNSP              = false);
int ConvertXCI( const std::string&  filename);
int InstallXCI( const std::string&  filename,
                const FsStorageId   destStorageId        = FsStorageId_SdCard,
                const bool          ignoreReqFirmVersion = true,
                const bool          deleteXCI            = false);

}
