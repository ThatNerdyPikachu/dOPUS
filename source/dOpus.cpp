#include <common.h>
#include "dOpus.h"

using namespace NXFramework;

namespace
{

}

void dOPUS::Initialize()
{
    gui.Initialize();
}

void dOPUS::Shutdown()
{
    gui.Shutdown();
}

void dOPUS::Update(const double timer, const u64 kDown)
{
#ifdef DEBUG
    if (kDown & KEY_ZL)
        displayOpenFiles();
#endif
    gui.Update(timer, kDown);
}

void dOPUS::Render(const double timer)
{
    SDL::ClearScreen(SDL::Renderer, BACKGROUND_COL);
    gui.Render(timer);
}

int main(int argc, char **argv)
{
    try
    {
        dOPUS app;
        app.Run(argc, argv);
    }
    catch (std::exception& e)
    {
        LOG_DEBUG("An error occurred:\n%s", e.what());
        u64 kDown = 0;
        while (!kDown)
        {
            hidScanInput();
            kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        }
    }
    catch (...)
    {
        LOG_DEBUG("An unknown error occurred:\n");
        u64 kDown = 0;
        while (!kDown)
        {
            hidScanInput();
            kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        }
    }
#ifdef DEBUG
    if(openFileCount() != 0)
    {
        LOG("Quitting dOPUS: some files are still open!");
        displayOpenFiles();
        return 0;
    }
#endif
    LOG("Quitting dOPUS: all good!");
    return 0;
}
