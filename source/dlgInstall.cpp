#include "dlgInstall.h"
#include <nxFramework/SDL/SDLRender.h>
#include <nxFramework/SDL/SDLHelper.h>
#include <nxFramework/nspHelper.h>
#include <nxFramework/xciHelper.h>
#include <nxFramework/fileHelper.h>
#include <algorithm>
#include "gui.h"

using namespace NXFramework;

DLGInstall::DLGInstall(const GUI* gui)
: GUIComponent(gui)
{}

void DLGInstall::Initialize()
{

}
void DLGInstall::Shutdown()
{

}
void DLGInstall::Render(const double timer)
{
    SDL::DrawImageScale(SDL::Renderer, rootGui->TextureHandle(GUI::Properties_dialog_dark), 190, 247, 900, 225);

    std::string message;
  	switch(dlgState)
	{
		case DLG_PROGRESS:
            if(dlgMode == DLG_EXTRACT)
                message = "Extracting...";
            else
            if(dlgMode == DLG_INSTALL || dlgMode == DLG_INSTALL_EXTRACTED)
                message = "Installing...";
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

    // Message
    int message_height = 0;
    TTF_SizeText(rootGui->FontHandle(GUI::Roboto), message.c_str(), NULL, &message_height);
    SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto),
                  205, 255, TITLE_COL, message.c_str());

    // Filename
    SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto_small),
                  205, 255 + message_height, TITLE_COL, filename.c_str(), 600);

    int options_cancel_width  = 0;
    int options_cancel_height = 0;
    TTF_SizeText(rootGui->FontHandle(GUI::Roboto), "CANCEL", &options_cancel_width, &options_cancel_height);
    SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto),
                  1070 - options_cancel_width,
                  467  - options_cancel_height, dlgState == DLG_PROGRESS?DARK_GREY:TITLE_COL, "CANCEL");

    int options_ok_width  = 0;
    TTF_SizeText(rootGui->FontHandle(GUI::Roboto), "OK", &options_ok_width, NULL);
    SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto),
                  1070 - 50 - options_cancel_width - options_ok_width,
                  467       - options_cancel_height, dlgState == DLG_PROGRESS?DARK_GREY:TITLE_COL, "OK");

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
    }
    else
    if(strncasecmp(GetFileExt(args->filename.c_str()), "xci", 3) == 0)
    {
        InstallXCI(filePath, args->destStorageId, args->ignoreReqFirmVersion, args->progress);
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

void DLGInstall::Update(const double timer, const u64 kDown)
{
    if(dlgState == DLG_CONFIRMATION)
    {
        if (kDown & KEY_A)
        {
            if(dlgMode == DLG_INSTALL || dlgMode == DLG_INSTALL_DELETE)
            {
                // Make sure there is no processThread in flight
                processThreadArgs.filedir                = filedir;
                processThreadArgs.filename               = filename;
                processThreadArgs.destStorageId          = FsStorageId_SdCard;
                processThreadArgs.ignoreReqFirmVersion   = true;
                processThreadArgs.progress               = &progress;
                processThreadArgs.running                = &threadRunning;
                threadRunning                            = true;
                std::string filePath = filedir + filename;
                LOG("Installing %s...\n", filePath.c_str());
                thrd_create(&processThread, InstallThread, &processThreadArgs);
                dlgState = DLG_PROGRESS;
            }
            else
            if(dlgMode == DLG_INSTALL_EXTRACTED)
            {
                // Make sure there is no processThread in flight
                processThreadArgs.filedir                = filedir;
                processThreadArgs.filename               = filename;
                processThreadArgs.destStorageId          = FsStorageId_SdCard;
                processThreadArgs.ignoreReqFirmVersion   = true;
                processThreadArgs.progress               = &progress;
                processThreadArgs.running                = &threadRunning;
                threadRunning                            = true;
                std::string filePath = filedir + filename;
                LOG("Installing folder %s...\n", filePath.c_str());
                thrd_create(&processThread, InstallExtractedThread, &processThreadArgs);
                dlgState = DLG_PROGRESS;
            }
            else
            if(dlgMode == DLG_EXTRACT)
            {
                // Make sure there is no processThread in flight
                processThreadArgs.filedir                = filedir;
                processThreadArgs.filename               = filename;
                processThreadArgs.progress               = &progress;
                processThreadArgs.running                = &threadRunning;
                threadRunning                            = true;
                std::string filePath = filedir + filename;
                LOG("Extracting %s...\n", filePath.c_str());
                thrd_create(&processThread, ExtractThread, &processThreadArgs);
                dlgState = DLG_PROGRESS;
            }
            if(dlgMode == DLG_CONVERT)
            {
                // Make sure there is no processThread in flight
                processThreadArgs.filedir                = filedir;
                processThreadArgs.filename               = filename;
                processThreadArgs.progress               = &progress;
                processThreadArgs.running                = &threadRunning;
                threadRunning                            = true;
                std::string filePath = filedir + filename;
                LOG("Extracting %s...\n", filePath.c_str());
                thrd_create(&processThread, ConvertThread, &processThreadArgs);
                dlgState = DLG_PROGRESS;
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
            if(dlgMode == DLG_INSTALL_DELETE)
            {
                std::string filePath = filedir + filename;
                LOG("Deleting file %s...\n", filePath.c_str());
                if(remove(filePath.c_str()) != 0)
                    printf("Error deleting file\n");
                dlgMode = DLG_INSTALL;
            }
        }
    }
}
