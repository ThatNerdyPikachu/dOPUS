#pragma once

#include <app.h>
#include "gui.h"

using namespace NXFramework;

class AppInstaller : public App
{
public:
    virtual void Initialize() override;
    virtual void Shutdown() override;
    virtual void Render(const double timer) override;
    virtual void Update(const double timer, const u64 kDown) override;

    AppInstaller() {}

private:
    GUI gui;
};
