#pragma once
#include <string>
#include <Tinfoil/include/nx/content_meta.hpp>

namespace NXFramework
{

int ExtractXCI( const std::string&  filename,
                const bool          saveNSP              = false,
                float*              progress             = nullptr);
int ConvertXCI( const std::string&  filename,
                float*              progress             = nullptr);
int InstallXCI( const std::string&  filename,
                const FsStorageId   destStorageId        = FsStorageId_SdCard,
                const bool          ignoreReqFirmVersion = true,
                const bool          deleteXCI            = false,
                float*              progress             = nullptr);

}
