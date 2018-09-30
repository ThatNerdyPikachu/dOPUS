#include "guiFileBrowser.h"
#include <nxFramework/SDL/SDLRender.h>
#include <nxFramework/SDL/SDLHelper.h>
#include <nspHelper.h>
#include <xciHelper.h>
#include <algorithm>
#include "gui.h"

#define START_PATH ROOT_PATH
#define FILES_PER_PAGE 15

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
    extFilters.push_back("nca");
    extFilters.push_back("cnmt");
    extFilters.push_back("xml");
    extFilters.push_back("cert");
    extFilters.push_back("tik");
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

		if(i >= cursor.top)
		{
			if (i == cursor.current)
				SDL::DrawRect(SDL::Renderer, 0, 140 + (37 * printed), 1280, 36, SELECTOR_COLOUR_LIGHT);

			char path[512];
			strcpy(path, curDir);
			strcpy(path + strlen(path), (char*)dirEntries[i].name);

            SDL::DrawImageScale(SDL::Renderer,
                                (dirEntries[i].isDir)?
                                rootGui->TextureHandle(GUI::Icon_dir):
                                rootGui->TextureHandle(GUI::Icon_file),
                                20, 140 + (37 * printed), 36, 36);

			char buf[64];
			char size[16];
			strncpy(buf, (char*)dirEntries[i].name, sizeof(buf));
			buf[sizeof(buf) - 1] = '\0';

			if (!dirEntries[i].isDir)
			{
				GetSizeString(size, dirEntries[i].size);
				int width = 0;
				TTF_SizeText(rootGui->FontHandle(GUI::Roboto_small), size, &width, NULL);
				SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto_small), 1260 - width, 140 + (37 * printed), TEXT_MIN_COLOUR_LIGHT, size);
			}

			int height = 0;
			TTF_SizeText(rootGui->FontHandle(GUI::Roboto), buf, NULL, &height);

			SDL::DrawText(SDL::Renderer, rootGui->FontHandle(GUI::Roboto), 65, 140 + ((37 - height)/2) + (37 * printed), BLACK,
                 (strncmp((char*)dirEntries[i].name, "..", 2) == 0)?"Parent folder":buf);

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
            cursor.current = std::max(cursor.current - 1, 0);
            if(cursor.current < cursor.top)
                cursor.top--;
        }
        else
        if (kDown & KEY_DDOWN)
        {
            cursor.current = std::min(cursor.current + 1, static_cast<int>(dirEntries.size()) - 1);
            if(cursor.current > cursor.top + FILES_PER_PAGE - 1)
                cursor.top++;
        }
        else
   		if (hidKeysHeld(CONTROLLER_P1_AUTO) & KEY_LSTICK_UP)
		{
            cursor.current = std::max(cursor.current - 1, 0);
            if(cursor.current < cursor.top)
                cursor.top--;
			wait(2);
		}
		else
        if (hidKeysHeld(CONTROLLER_P1_AUTO) & KEY_LSTICK_DOWN)
		{
            cursor.current = std::min(cursor.current + 1, static_cast<int>(dirEntries.size()) - 1);
            if(cursor.current > cursor.top + FILES_PER_PAGE - 1)
                cursor.top++;
   			wait(2);
		}

   		if (kDown & KEY_DLEFT)
        {
            cursor.current = std::max(cursor.current - FILES_PER_PAGE + 1, 0);
            if(cursor.current < cursor.top)
                cursor.top = cursor.current;
  			wait(5);
        }
		else
        if (kDown & KEY_DRIGHT)
        {
            cursor.current = std::min(cursor.current + FILES_PER_PAGE - 1, static_cast<int>(dirEntries.size()) - 1);
            if(cursor.current > cursor.top + FILES_PER_PAGE - 1)
                cursor.top = std::max(cursor.current - FILES_PER_PAGE + 1, 0);
            wait(5);
        }
        else
        if (kDown & KEY_A) // Install
		{
		    DirEntry& entry = dirEntries[cursor.current];
		    if(entry.isDir)
            {
                Navigate(curDir, dirEntries[cursor.current], false);
                PopulateFiles(curDir, dirEntries, extFilters);

                // Backup cursor
                cursorStack.push_back(cursor);
                cursor.current  = 0;
                cursor.top      = 0;
            }
            else
            {
                if(strncasecmp(GetFileExt((char*)entry.name), "nsp", 3) == 0)
                {
                    std::string NSPPath = curDir + std::string((char*)entry.name);
                    LOG("\nInstalling %s...\n", NSPPath.c_str());
                    InstallNSP(NSPPath);
                }
                else
                if(strncasecmp(GetFileExt((char*)entry.name), "xci", 3) == 0)
                {
                    std::string XCIPath = curDir + std::string((char*)entry.name);
                    LOG("\nInstalling %s...\n", XCIPath.c_str());
                    InstallXCI(XCIPath);

                    // Refresh folder view
                    PopulateFiles(curDir, dirEntries, extFilters);
                    cursor.current  = 0;
                    cursor.top      = 0;
                }
            }
		}
		else
        if (kDown & KEY_X) // Extract
        {
   		    DirEntry& entry = dirEntries[cursor.current];
		    if(entry.isDir)
            {
                std::string NCAPath = curDir + std::string((char*)entry.name);
                LOG("\nInstalling folder %s...\n", NCAPath.c_str());
                InstallExtracted(NCAPath.c_str());
            }
            else
            {
                if(strncasecmp(GetFileExt((char*)entry.name), "nsp", 3) == 0)
                {
                    std::string NSPPath = curDir + std::string((char*)entry.name);
                    LOG("\nExtracting %s...\n", NSPPath.c_str());
                    ExtractNSP(NSPPath);

                    // Refresh folder view
                    PopulateFiles(curDir, dirEntries, extFilters);
                    cursor.current  = 0;
                    cursor.top      = 0;
                }
                else
                if(strncasecmp(GetFileExt((char*)entry.name), "xci", 3) == 0)
                {
                    std::string XCIPath = curDir + std::string((char*)entry.name);
                    LOG("\nExtracting %s...:)\n", XCIPath.c_str());
                    ExtractXCI(XCIPath);

                    // Refresh folder view
                    PopulateFiles(curDir, dirEntries, extFilters);
                    cursor.current  = 0;
                    cursor.top      = 0;
                }
            }
        }
		else
        if (kDown & KEY_Y) // Convert
        {
   		    DirEntry& entry = dirEntries[cursor.current];
		    if(!entry.isDir)
            {
                if(strncasecmp(GetFileExt((char*)entry.name), "xci", 3) == 0)
                {
                    std::string XCIPath = curDir + std::string((char*)entry.name);
                    LOG("\nConverting %s...\n", XCIPath.c_str());
                    ConvertXCI(XCIPath);

                    // Refresh folder view
                    PopulateFiles(curDir, dirEntries, extFilters);
                    cursor.current  = 0;
                    cursor.top      = 0;
                }
            }
        }
		else
		if ((strcmp(curDir, ROOT_PATH) != 0) && (kDown & KEY_B))
		{
			Navigate(curDir, dirEntries[cursor.current], true);
            PopulateFiles(curDir, dirEntries, extFilters);

            // Restore cursor
            cursor = cursorStack.back();
            cursorStack.pop_back();
		}
    }
}
