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
    return 0;
}
