#include "guiFileBrowser.h"
#include <SDL/SDLRender.h>
#include <SDL/SDLHelper.h>
#include <nspHelper.h>
#include <xciHelper.h>
#include <algorithm>
#include "gui.h"
#define START_PATH ROOT_PATH
#define FILES_PER_PAGE 8

using namespace NXFramework;

GUIFileBrowser::GUIFileBrowser(const GUI* gui)
: rootGui(gui)
{
    assert(gui != nullptr);
    strcpy(curDir, START_PATH);
}
void GUIFileBrowser::Initialize()
{
    extFilters.push_back("nsp");
    extFilters.push_back("xci");
    PopulateFiles(curDir, dirEntries, extFilters);
}
void GUIFileBrowser::Shutdown()
{

}
void GUIFileBrowser::Render(const double timer)
{
	int title_height = 0;
	SDL::DrawImage(SDL::Renderer, rootGui->TextureHandle(GUI::Icon_nav_drawer), 20, 58);
	SDL::DrawImage(SDL::Renderer, rootGui->TextureHandle(GUI::Icon_actions), (1260 - 64), 58);
	TTF_SizeText(rootGui->FontHandle(GUI::Roboto_large), curDir, NULL, &title_height);
	SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto_large), 170, 40 + ((100 - title_height)/2), WHITE, curDir);

	int printed = 0;
	for(int i = 0 ; i < (int)dirEntries.size() ; ++i)
	{
		if (printed == FILES_PER_PAGE) // Limit the files per page
			break;

		if(i >= topPosition)
		{
			if (i == curPosition)
				SDL::DrawRect(SDL::Renderer, 0, 140 + (73 * printed), 1280, 73, SELECTOR_COLOUR_LIGHT);

			SDL::DrawImage(SDL::Renderer, rootGui->TextureHandle(GUI::Icon_uncheck), 20, 156 + (73 * printed));

			char path[512];
			strcpy(path, curDir);
			strcpy(path + strlen(path), (char*)dirEntries[i].name);

			if (dirEntries[i].isDir)
                SDL::DrawImageScale(SDL::Renderer, rootGui->TextureHandle(GUI::Icon_dir) , 80, 141 + (73 * printed), 72, 72);
			else
                SDL::DrawImageScale(SDL::Renderer, rootGui->TextureHandle(GUI::Icon_file), 80, 141 + (73 * printed), 72, 72);

			char buf[64];
			char size[16];
			strncpy(buf, (char*)dirEntries[i].name, sizeof(buf));
			buf[sizeof(buf) - 1] = '\0';

			if (!dirEntries[i].isDir)
			{
				GetSizeString(size, dirEntries[i].size);
				int width = 0;
				TTF_SizeText(rootGui->FontHandle(GUI::Roboto_small), size, &width, NULL);
				SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto_small), 1260 - width, 180 + (73 * printed), TEXT_MIN_COLOUR_LIGHT, size);
			}

			int height = 0;
			TTF_SizeText(rootGui->FontHandle(GUI::Roboto), buf, NULL, &height);

			if (strncmp((char*)dirEntries[i].name, "..", 2) == 0)
				SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto), 170, 140 + ((73 - height)/2) + (73 * printed), BLACK, "Parent folder");
			else
				SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto), 170, 140 + ((73 - height)/2) + (73 * printed), BLACK, buf);

			printed++; // Increase printed counter
		}
	}
}

void GUIFileBrowser::Update(const double timer, const u64 kDown)
{
    if(dirEntries.size() > 0)
    {
        if (kDown & KEY_DUP)
        {
            curPosition = std::max(curPosition - 1, 0);
            if(curPosition < topPosition)
                topPosition--;
        }
        else
        if (kDown & KEY_DDOWN)
        {
            curPosition = std::min(curPosition + 1, static_cast<int>(dirEntries.size()) - 1);
            if(curPosition > topPosition + FILES_PER_PAGE - 1)
                topPosition++;
        }
        else
   		if (kDown & KEY_LEFT)
        {
  			wait(5);
			curPosition = 0;
  			topPosition = 0;
        }
		else
        if (kDown & KEY_RIGHT)
        {
            wait(5);
            curPosition = dirEntries.size() - 1;
   			topPosition = std::max(curPosition - FILES_PER_PAGE + 1, 0);
        }
        else
        if (kDown & KEY_A)
		{
		    DirEntry& entry = dirEntries[curPosition];
		    if(entry.isDir)
            {
                Navigate(curDir, dirEntries[curPosition], false);
                PopulateFiles(curDir, dirEntries, extFilters);
                curPosition = 0;
                topPosition = 0;
            }
            else
            {
                // Do it here for now
                if(strncasecmp(GetFileExt((char*)entry.name), "nsp", 3) == 0)
                {
                    std::string NSPPath = std::string(std::string("@Sdcard:/") + curDir + std::string((char*)entry.name));
                    LOG("Installing %s... TODO :)\n", NSPPath.c_str());
                    InstallNSP(NSPPath);
                }
                else
                if(strncasecmp(GetFileExt((char*)entry.name), "xci", 3) == 0)
                {
                    std::string XCIPath = std::string(curDir + std::string((char*)entry.name));
                    LOG("Converting %s... TODO :)\n", XCIPath.c_str());
                    ConvertXCI(XCIPath);
                }
            }
		}
		else
		if ((strcmp(curDir, ROOT_PATH) != 0) && (kDown & KEY_B))
		{
			Navigate(curDir, dirEntries[curPosition], true);
            PopulateFiles(curDir, dirEntries, extFilters);
            curPosition = 0;
   			topPosition = 0;
		}
    }
}
