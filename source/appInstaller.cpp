#include <common.h>
#include "appInstaller.h"

using namespace NXFramework;

namespace
{

}

void AppInstaller::Initialize()
{
    gui.Initialize();
}

void AppInstaller::Shutdown()
{
    gui.Shutdown();
}

void AppInstaller::Update(const double timer, const u64 kDown)
{
    gui.Update(timer, kDown);
}

void AppInstaller::Render(const double timer)
{
    SDL::ClearScreen(SDL::Renderer, BACKGROUND_COL);
    gui.Render(timer);
}

int main(int argc, char **argv)
{
    AppInstaller app;
    app.Run(argc, argv);
    return 0;
}
