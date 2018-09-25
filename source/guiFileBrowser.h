#pragma once

#include <common.h>
#include <fileHelper.h>
#include "gui.h"

using namespace NXFramework;

class GUIFileBrowser: public GUIComponent
{
public:
    GUIFileBrowser(const GUI* gui);
    void Initialize() override;
    void Shutdown() override;
    void Render(const double timer) override;
    void Update(const double timer, const u64 kDown) override;
private:
    const GUI*              rootGui;
    char                    curDir[512];
    int                     curPosition = 0;
    int                     topPosition = 0;
    std::vector<DirEntry>   dirEntries;
    std::vector<std::string>extFilters;
};
