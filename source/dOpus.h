#pragma once

#include <app.h>
#include "gui.h"

using namespace NXFramework;

class dOPUS : public App
{
public:
    virtual void Initialize() override;
    virtual void Shutdown() override;
    virtual void Render(const double timer) override;
    virtual void Update(const double timer, const u64 kDown) override;

    dOPUS() {}

private:
    GUI gui;
};
