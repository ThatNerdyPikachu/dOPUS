#include "gui.h"

#include <utility>
#include <nxFramework/SDL/SDLRender.h>
#include <nxFramework/SDL/SDLHelper.h>
#include <nxFramework/nxUtils.h>
#include "guiFileBrowser.h"

using namespace NXFramework;

const char* imgFiles[] =
{
    "romfs:/res/images/ic_fso_type_executable.png",
    "romfs:/res/images/ic_fso_type_compress.png",
    "romfs:/res/images/ic_fso_type_audio.png",
	"romfs:/res/images/ic_fso_folder.png",
	"romfs:/res/images/ic_fso_folder_dark.png",
	"romfs:/res/images/ic_fso_type_document.png",
	"romfs:/res/images/ic_fso_default.png",
	"romfs:/res/images/ic_fso_type_image.png",
	"romfs:/res/images/ic_fso_type_text.png",
	"romfs:/res/images/btn_material_light_check_on_normal.png",
	"romfs:/res/images/btn_material_light_check_on_normal_dark.png",
	"romfs:/res/images/btn_material_light_check_off_normal.png",
	"romfs:/res/images/btn_material_light_check_off_normal_dark.png",
	"romfs:/res/images/ic_material_dialog.png",
	"romfs:/res/images/ic_material_options_dialog.png",
    "romfs:/res/images/ic_material_properties_dialog.png",
	"romfs:/res/images/ic_material_dialog_dark.png",
	"romfs:/res/images/ic_material_options_dialog_dark.png",
	"romfs:/res/images/ic_material_properties_dialog_dark.png",
	"romfs:/res/images/bg_header.png",
	"romfs:/res/images/ic_material_light_settings.png",
	"romfs:/res/images/ic_material_light_sdcard.png",
    "romfs:/res/images/ic_material_light_secure.png",
	"romfs:/res/images/ic_material_light_settings_dark.png",
	"romfs:/res/images/ic_material_light_sdcard_dark.png",
	"romfs:/res/images/ic_material_light_secure_dark.png",
	"romfs:/res/images/btn_material_light_radio_off_normal.png",
	"romfs:/res/images/btn_material_light_radio_on_normal.png",
	"romfs:/res/images/btn_material_light_radio_off_normal_dark.png",
	"romfs:/res/images/btn_material_light_radio_on_normal_dark.png",
	"romfs:/res/images/btn_material_light_toggle_on_normal.png",
	"romfs:/res/images/btn_material_light_toggle_on_normal_dark.png",
	"romfs:/res/images/btn_material_light_toggle_off_normal.png",
	"romfs:/res/images/default_artwork.png",
	"romfs:/res/images/default_artwork_blur.png",
	"romfs:/res/images/btn_playback_play.png",
	"romfs:/res/images/btn_playback_pause.png",
	"romfs:/res/images/btn_playback_rewind.png",
	"romfs:/res/images/btn_playback_forward.png",
	"romfs:/res/images/btn_playback_repeat.png",
	"romfs:/res/images/btn_playback_shuffle.png",
	"romfs:/res/images/btn_playback_repeat_overlay.png",
	"romfs:/res/images/btn_playback_shuffle_overlay.png",
	"romfs:/res/images/ic_material_light_navigation_drawer.png",
	"romfs:/res/images/ic_material_light_contextual_action.png",
	"romfs:/res/images/ic_arrow_back_normal.png",
	"romfs:/res/images/ic_material_light_accept.png",
	"romfs:/res/images/ic_material_light_accept_dark.png",
	"romfs:/res/images/ic_material_light_remove.png",
	"romfs:/res/images/ic_material_light_remove_dark.png",
	"romfs:/res/images/battery_20.png",
	"romfs:/res/images/battery_20_charging.png",
	"romfs:/res/images/battery_30.png",
	"romfs:/res/images/battery_30_charging.png",
	"romfs:/res/images/battery_50.png",
	"romfs:/res/images/battery_50_charging.png",
	"romfs:/res/images/battery_60.png",
	"romfs:/res/images/battery_60_charging.png",
	"romfs:/res/images/battery_80.png",
	"romfs:/res/images/battery_80_charging.png",
	"romfs:/res/images/battery_90.png",
	"romfs:/res/images/battery_90_charging.png",
	"romfs:/res/images/battery_full.png",
	"romfs:/res/images/battery_full_charging.png",
	"romfs:/res/images/battery_low.png",
	"romfs:/res/images/battery_unknown.png",
	"romfs:/res/images/ic_material_dialog_fs_locked.png",
};

const std::pair<const char*,u32> fontFiles[] =
{
    {"romfs:/res/Roboto-Regular.ttf", 30},
    {"romfs:/res/Roboto-Regular.ttf", 25},
    {"romfs:/res/Roboto-Regular.ttf", 20},
    {"romfs:/res/Roboto-Regular.ttf", 50},
};

void GUI::LoadTextures(void)
{
    for(uint i = 0 ; i < Tex_count ; ++i)
    {
        LOG("Loading %s...\n", (char*)imgFiles[i]);
        SDL::LoadImage(SDL::Renderer, &(textures[i]), (char*)imgFiles[i]);
    }
}

void GUI::FreeTextures(void)
{
    for(uint i = 0 ; i < Tex_count ; ++i)
        SDL_DestroyTexture(textures[i]);
}

void GUI::LoadFonts()
{
    for(uint i = 0 ; i < Font_count ; ++i)
    {
        LOG("Loading %s at size %d...\n", (char*)fontFiles[i].first, fontFiles[i].second);
        fonts[i] = TTF_OpenFont((char*)fontFiles[i].first, fontFiles[i].second);
        if(!fonts[i])
            LOG("Error loading font %s...\n", fontFiles[i].first);
    }
}

void GUI::FreeFonts()
{
    for(uint i = 0 ; i < Font_count ; ++i)
    {
        TTF_CloseFont(fonts[i]);
    }
}

void GUI::Initialize()
{
    LoadTextures();
    LoadFonts();
    guiFileBrowser->Initialize();
    guiFileBrowser->SetEnabled(true);
}

void GUI::Shutdown()
{
    guiFileBrowser->Shutdown();
    FreeFonts();
   	FreeTextures();
}

void GUI::StatusBarGetBatteryStatus(int x, int y)
{
    char buf[5];
    int width   = 0;
    u32 percent = 0;
    ChargerType state;
    if(GetBatteryStatus(state, percent))
    {
        if (percent < 20)
            SDL::DrawImage(SDL::Renderer, textures[Battery_low], x, 3);
        else if ((percent >= 20) && (percent < 30))
            SDL::DrawImage(SDL::Renderer, textures[(state != 0)?Battery_20_charging:Battery_20], x, 3);
        else if ((percent >= 30) && (percent < 50))
            SDL::DrawImage(SDL::Renderer, textures[(state != 0)?Battery_30_charging:Battery_30], x, 3);
        else if ((percent >= 50) && (percent < 60))
            SDL::DrawImage(SDL::Renderer, textures[(state != 0)?Battery_50_charging:Battery_50], x, 3);
        else if ((percent >= 60) && (percent < 80))
            SDL::DrawImage(SDL::Renderer, textures[(state != 0)?Battery_60_charging:Battery_60], x, 3);
        else if ((percent >= 80) && (percent < 90))
            SDL::DrawImage(SDL::Renderer, textures[(state != 0)?Battery_80_charging:Battery_80], x, 3);
        else if ((percent >= 90) && (percent < 100))
            SDL::DrawImage(SDL::Renderer, textures[(state != 0)?Battery_90_charging:Battery_90], x, 3);
        else if (percent == 100)
            SDL::DrawImage(SDL::Renderer, textures[(state != 0)?Battery_full_charging:Battery_full], x, 3);

        snprintf(buf, 5, "%d%%", percent);
        TTF_SizeText(fonts[Roboto], buf, &width, NULL);
        SDL::DrawText(SDL::Renderer, fonts[Roboto], (x - width - 10), y, WHITE, buf);
    }
    else
    {
        snprintf(buf, 5, "%d%%", percent);
        TTF_SizeText(fonts[Roboto], buf, &width, NULL);
        SDL::DrawText(SDL::Renderer, fonts[Roboto], (x - width - 10), y, WHITE, buf);
        SDL::DrawImage(SDL::Renderer, textures[Battery_unknown], x, 1);
    }
}

void GUI::StatusBarDisplayTime(void)
{
    int width  = 0;
    int height = 0;
    char* curTime = ClockGetCurrentTime();
    TTF_SizeText(fonts[Roboto], curTime, &width, &height);

    StatusBarGetBatteryStatus(1260 - width - 44, (40 - height) / 2);
    SDL::DrawText(SDL::Renderer, fonts[Roboto], 1260 - width, (40 - height) / 2, WHITE, curTime);
}

void GUI::Render(const double timer)
{
    SDL::DrawRect(SDL::Renderer, 0, 0, 1280, 40, STATUS_BAR_COL);	// Status bar
    SDL::DrawRect(SDL::Renderer, 0, 40, 1280, 40, MENU_BAR_COL);	// Directory bar

    // Title
    int height = 0;
    int width  = 0;
    TTF_SizeText(fonts[Roboto], "dOPUS 0.6", &width, &height);
    SDL::DrawText(SDL::Renderer, fonts[Roboto], 12, (40 - height) / 2, CYAN, "dOPUS 0.6");

    // Free space
    {
        char freeSDStr[32];
        char totalSDStr[32];
        char freeNandStr[32];
        char totalNandstr[32];
        u64 freeSDu     = GetFreeSpace(FsStorageId_SdCard);
        u64 totalSDu    = GetTotalSpace(FsStorageId_SdCard);
        u64 freeNandu   = GetFreeSpace(FsStorageId_NandUser);
        u64 totalNandu  = GetTotalSpace(FsStorageId_NandUser);
        GetSizeString(freeSDStr, freeSDu);
        GetSizeString(totalSDStr, totalSDu);
        GetSizeString(freeNandStr, freeNandu);
        GetSizeString(totalNandstr, totalNandu);
        std::string freeSpace =       std::string ("SD ")       + std::string (freeSDStr) +
        std::string ("/")           + std::string(totalSDStr)   + std::string ("      Nand ") +
        std::string(freeNandStr)    + std::string ("/")         + std::string(totalNandstr);
        TTF_SizeText(fonts[Roboto], freeSpace.c_str(), NULL, &height);
        SDL::DrawText(SDL::Renderer, fonts[Roboto], 60 + width, (40 - height) / 2, WHITE, freeSpace.c_str());
    }

    // Status bar
    StatusBarDisplayTime();

    // File browser
    if(guiFileBrowser->IsEnabled())
        guiFileBrowser->Render(timer);
}

void GUI::Update(const double timer, const u64 kDown)
{
    if(guiFileBrowser->IsEnabled())
        guiFileBrowser->Update(timer, kDown);
}

GUI::GUI()
: guiFileBrowser(new GUIFileBrowser(this))
{}

GUI::~GUI()
{
    delete guiFileBrowser;
}
