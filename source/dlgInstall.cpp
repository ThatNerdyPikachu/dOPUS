#include "dlgInstall.h"
#include <nxFramework/SDL/SDLRender.h>
#include <nxFramework/SDL/SDLHelper.h>
#include <nxFramework/nspHelper.h>
#include <nxFramework/xciHelper.h>
#include <nxFramework/fileHelper.h>
#include <algorithm>
#include "gui.h"

using namespace NXFramework;


namespace
{
    struct ProcessThreadArgs
    {
        std::string filedir;
        std::string filename;
        FsStorageId destStorageId;
        bool        ignoreReqFirmVersion;
        float*      progress;
        bool*       running;
        bool        deleteSourceFile;
    };
    thrd_t              processThread;
    ProcessThreadArgs   processThreadArgs;
    bool                threadRunning = false;
}


int InstallThread(void* in)
{
    ProcessThreadArgs* args = reinterpret_cast<ProcessThreadArgs*>(in);
    std::string filePath = args->filedir + args->filename;
    LOG("Installing %s...\n", filePath.c_str());
    if(strncasecmp(GetFileExt(args->filename.c_str()), "nsp", 3) == 0)
    {
        InstallNSP(filePath, args->destStorageId, args->ignoreReqFirmVersion, false, args->progress);
        if(args->deleteSourceFile)
        {
            LOG("Deleting file %s...\n", filePath.c_str());
            if(remove(filePath.c_str()) != 0)
                printf("Error deleting file\n");
        }
    }
    else
    if(strncasecmp(GetFileExt(args->filename.c_str()), "xci", 3) == 0)
    {
        InstallXCI(filePath, args->destStorageId, args->ignoreReqFirmVersion, args->deleteSourceFile, args->progress);
    }
    *(args->running) = false;
    return 0;
}

int InstallExtractedThread(void* in)
{
    ProcessThreadArgs* args = reinterpret_cast<ProcessThreadArgs*>(in);
    std::string filePath = args->filedir + args->filename;
    LOG("Installing folder %s...\n", filePath.c_str());
    InstallExtracted(filePath, args->destStorageId, args->ignoreReqFirmVersion, args->progress);
    *(args->running) = false;
    return 0;
}

int ExtractThread(void* in)
{
    ProcessThreadArgs* args = reinterpret_cast<ProcessThreadArgs*>(in);
    std::string filePath = args->filedir + args->filename;
    LOG("\nExtracting %s...:)\n", filePath.c_str());
    if(strncasecmp(GetFileExt(args->filename.c_str()), "nsp", 3) == 0)
    {
        ExtractNSP(filePath, args->progress);
    }
    else
    if(strncasecmp(GetFileExt(args->filename.c_str()), "xci", 3) == 0)
    {
        ExtractXCI(filePath, false, args->progress);
    }
    *(args->running) = false;
    return 0;
}

int ConvertThread(void* in)
{
    ProcessThreadArgs* args = reinterpret_cast<ProcessThreadArgs*>(in);
    std::string filePath = args->filedir + args->filename;
    LOG("\n Converting %s...:)\n", filePath.c_str());
    if(strncasecmp(GetFileExt(args->filename.c_str()), "xci", 3) == 0)
    {
        ConvertXCI(filePath, args->progress);
    }
    *(args->running) = false;
    return 0;
}


DLGInstall::DLGInstall(const GUI* gui)
: GUIComponent(gui)
{}

void DLGInstall::Initialize()
{

}
void DLGInstall::Shutdown()
{

}

void DLGInstall::Update(const double timer, const u64 kDown)
{
    if(dlgState == DLG_CONFIRMATION)
    {
        // SDCard or Nand selection
        if( dlgMode == DLG_INSTALL              ||
            dlgMode == DLG_INSTALL_DELETE       ||
            dlgMode == DLG_INSTALL_EXTRACTED)
        {
            if (kDown & KEY_DLEFT)
                destStorageId = FsStorageId_SdCard;
            else
            if (kDown & KEY_DRIGHT)
                destStorageId = FsStorageId_NandUser;
        }

        // Thread kick off
        if (kDown & KEY_A)
        {
            std::string filePath = filedir + filename;
            u64 freeSpace   = GetFreeSpace(destStorageId);
            u64 fileSize    = GetFileSize(filePath.c_str());
            enoughSpace     = true;

            // Check if enoug free space
            if(dlgMode == DLG_CONVERT)
            {
                // we need twice the XCI size: 1x for temp files, 1x for NSP
                if(freeSpace < 2 * fileSize)
                {
#ifdef DEBUG
                    char freeStr[256];
                    char neededStr[256];
                    GetSizeString(freeStr, freeSpace);
                    GetSizeString(neededStr, 2 * fileSize);
                    LOG("Not enough space to convert %s... %s free %s needed\n", filePath.c_str(), freeStr, neededStr);
#endif
                    enoughSpace = false;
                }
            }
            else
            if(dlgMode == DLG_INSTALL           ||
               dlgMode == DLG_INSTALL_DELETE    ||
               dlgMode == DLG_INSTALL_EXTRACTED ||
               dlgMode == DLG_EXTRACT)
            {
                // we need the file size
                if(freeSpace < fileSize)
                {
#ifdef DEBUG
                    char freeStr[256];
                    char neededStr[256];
                    GetSizeString(freeStr, freeSpace);
                    GetSizeString(neededStr, fileSize);
                    LOG("Not enough space to install/extract %s... %s free %s needed\n", filePath.c_str(), freeStr, neededStr);
#endif
                    enoughSpace = false;
                }
            }
            else
            {
#ifdef DEBUG
                char freeStr[256];
                GetSizeString(freeStr, freeSpace);
                LOG("Enough space to process %s... %s free\n", filePath.c_str(), freeStr);
#endif
            }

            if(enoughSpace)
            {
                if(dlgMode == DLG_INSTALL || dlgMode == DLG_INSTALL_DELETE)
                {
                    // Make sure there is no processThread in flight
                    processThreadArgs.filedir                = filedir;
                    processThreadArgs.filename               = filename;
                    processThreadArgs.destStorageId          = destStorageId;
                    processThreadArgs.ignoreReqFirmVersion   = true;
                    processThreadArgs.progress               = &progress;
                    processThreadArgs.running                = &threadRunning;
                    processThreadArgs.deleteSourceFile       = (dlgMode == DLG_INSTALL_DELETE);
                    threadRunning                            = true;
                    LOG("Installing %s to %s...\n", filePath.c_str(), destStorageId==FsStorageId_SdCard?"SD Card":"Nand");
                    thrd_create(&processThread, InstallThread, &processThreadArgs);
                    dlgState = DLG_PROGRESS;
                }
                else
                if(dlgMode == DLG_INSTALL_EXTRACTED)
                {
                    // Make sure there is no processThread in flight
                    processThreadArgs.filedir                = filedir;
                    processThreadArgs.filename               = filename;
                    processThreadArgs.destStorageId          = destStorageId;
                    processThreadArgs.ignoreReqFirmVersion   = true;
                    processThreadArgs.deleteSourceFile       = false;
                    processThreadArgs.progress               = &progress;
                    processThreadArgs.running                = &threadRunning;
                    threadRunning                            = true;
                    LOG("Installing folder %s to %s...\n", filePath.c_str(), destStorageId==FsStorageId_SdCard?"SD Card":"Nand");
                    thrd_create(&processThread, InstallExtractedThread, &processThreadArgs);
                    dlgState = DLG_PROGRESS;
                }
                else
                if(dlgMode == DLG_EXTRACT)
                {
                    // Make sure there is no processThread in flight
                    processThreadArgs.filedir                = filedir;
                    processThreadArgs.filename               = filename;
                    processThreadArgs.deleteSourceFile       = false;
                    processThreadArgs.progress               = &progress;
                    processThreadArgs.running                = &threadRunning;
                    threadRunning                            = true;
                    LOG("Extracting %s...\n", filePath.c_str());
                    thrd_create(&processThread, ExtractThread, &processThreadArgs);
                    dlgState = DLG_PROGRESS;
                }
                if(dlgMode == DLG_CONVERT)
                {
                    // Make sure there is no processThread in flight
                    processThreadArgs.filedir                = filedir;
                    processThreadArgs.filename               = filename;
                    processThreadArgs.deleteSourceFile       = false;
                    processThreadArgs.progress               = &progress;
                    processThreadArgs.running                = &threadRunning;
                    threadRunning                            = true;
                    LOG("Extracting %s...\n", filePath.c_str());
                    thrd_create(&processThread, ConvertThread, &processThreadArgs);
                    dlgState = DLG_PROGRESS;
                }
            }
        }
        else
        if (kDown & KEY_B)
        {
            // Quit this window
            SetEnabled(false);
        }
    }
    else
    if(dlgState == DLG_PROGRESS)
    {
        // we are done
        if(!threadRunning)
        {
            thrd_join(processThread, NULL);
            dlgState = DLG_DONE;
            dlgMode  = DLG_INSTALL;
        }
    }
}

void DLGInstall::Render(const double timer)
{
    std::string message;
  	switch(dlgState)
	{
		case DLG_PROGRESS:
            if(dlgMode == DLG_EXTRACT)
                message = "Extracting...";
            else
            if(dlgMode == DLG_INSTALL           ||
               dlgMode == DLG_INSTALL_EXTRACTED ||
               dlgMode == DLG_INSTALL_DELETE)
                message = std::string("Installing to ") +
                    (destStorageId==FsStorageId_SdCard?
                     std::string("SD Card"):std::string("Nand")) +
                     std::string("...");
            else
                message = "Converting...";
        break;
		case DLG_DONE:
            message = "Done!";
        break;
		case DLG_CONFIRMATION:
        default:
            if(dlgMode == DLG_EXTRACT)
                message = "Extract";
            else
            if(dlgMode == DLG_INSTALL || dlgMode == DLG_INSTALL_EXTRACTED)
                message = "Install";
            else
            if(dlgMode == DLG_INSTALL_DELETE)
                message = "Install and Delete";
            else
                message = "Convert";
	}
    // Main frame
    SDL::DrawImageScale(SDL::Renderer, rootGui->TextureHandle(GUI::Properties_dialog_dark),
                        190, 247, 900, 225);

    // Message
    int message_height = 0;
    TTF_SizeText(rootGui->FontHandle(GUI::Roboto), message.c_str(), NULL, &message_height);
    SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto),
                  205, 255, TITLE_COL, message.c_str());

    // Filename
    SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto_small),
                  205, 255 + message_height, TITLE_COL, filename.c_str(), 600);

    // Not enough space
    if(!enoughSpace)
    {
        std::string notEnoughSpace =
            std::string("Not enough free space on ")                                        +
            (destStorageId==FsStorageId_SdCard?std::string("SD Card"):std::string("Nand"))  +
            std::string("!");
        SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto_small),
                      205, 255 + message_height, RED, notEnoughSpace.c_str(), 600);
    }

    // Cancel
    int options_cancel_width  = 0;
    int options_cancel_height = 0;
    TTF_SizeText(rootGui->FontHandle(GUI::Roboto), "(B) Cancel", &options_cancel_width, &options_cancel_height);
    SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto),
                  1070 - options_cancel_width,
                  467  - options_cancel_height, dlgState == DLG_PROGRESS?DARK_GREY:TITLE_COL, "CANCEL");

    // OK
    if(enoughSpace)
    {
        int options_ok_width  = 0;
        TTF_SizeText(rootGui->FontHandle(GUI::Roboto), "(A) OK", &options_ok_width, NULL);
        SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto),
                      1070 - 50 - options_cancel_width - options_ok_width,
                      467       - options_cancel_height, dlgState != DLG_CONFIRMATION?DARK_GREY:TITLE_COL, "OK");
    }

    // SDCard or Nand
    if(dlgState == DLG_CONFIRMATION)
    {
        if(dlgMode == DLG_INSTALL           ||
           dlgMode == DLG_INSTALL_DELETE    ||
           dlgMode == DLG_INSTALL_EXTRACTED)
        {
            int txt_height = 0;
	    int txt_width  = 0;
            TTF_SizeText(rootGui->FontHandle(GUI::Roboto), "SD Card", &txt_width, &txt_height);
            SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto),
                          190+350, 375-txt_height,
                          (destStorageId == FsStorageId_SdCard)  ?TITLE_COL:DARK_GREY, "SD Card");
            
	    TTF_SizeText(rootGui->FontHandle(GUI::Roboto), "Nand", &txt_width, &txt_height);
            SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto),
                          190+900-350-txt_width, 375-txt_height,
                          (destStorageId == FsStorageId_NandUser)?TITLE_COL:DARK_GREY, "Nand");
        }
    }

    if(dlgState == DLG_PROGRESS)
    {
        int barSize           = 850;
        int borderSize        = 3;

        char percentStr[10];
        sprintf(percentStr, "%d%s", (int)(progress * 100.0), "%");
        int percent_witdh  = 0;
        int percent_height = 0;
        TTF_SizeText(rootGui->FontHandle(GUI::Roboto), percentStr, &percent_witdh, &percent_height);
        SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto), 215+barSize-percent_witdh, 375-borderSize-percent_height-5, TITLE_COL, percentStr);

        // Progress bar
        float progressBarSize = barSize * progress;
        SDL::DrawRect(SDL::Renderer, 215-borderSize , 375-borderSize, barSize+borderSize*2, 20+borderSize*2 , DARK_GREY);
        SDL::DrawRect(SDL::Renderer, 215            , 375           , (int)progressBarSize, 20              , CYAN);
    }
}
