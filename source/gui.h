#pragma once

#include <common.h>

using namespace NXFramework;

class GUIComponent
{
public:
    GUIComponent() {}
    virtual ~GUIComponent() {}
    virtual void Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void Render(const double timer) = 0;
    virtual void Update(const double timer, const u64 kDown) = 0;
    virtual bool IsVisible()                 { return isVisible; }
    virtual void SetVisibile(bool visible)   { isVisible = visible; }
protected:
    bool isVisible = false;
};

class GUIFileBrowser;
class GUI
{
public:
    enum Texture
    {
        Icon_app = 0,
        Icon_archive,
        Icon_audio,
        Icon_dir,
        Icon_dir_dark,
        Icon_doc,
        Icon_file,
        Icon_image,
        Icon_text,
        Icon_check,
        Icon_check_dark,
        Icon_uncheck,
        Icon_uncheck_dark,
        Dialog,
        Options_dialog,
        Properties_dialog,
        Dialog_dark,
        Options_dialog_dark,
        Properties_dialog_dark,
        Bg_header,
        Icon_settings,
        Icon_sd,
        Icon_secure,
        Icon_settings_dark,
        Icon_sd_dark,
        Icon_secure_dark,
        Icon_radio_off,
        Icon_radio_on,
        Icon_radio_dark_off,
        Icon_radio_dark_on,
        Icon_toggle_on,
        Icon_toggle_dark_on,
        Icon_toggle_off,
        Default_artwork,
        Default_artwork_blur,
        Btn_play,
        Btn_pause,
        Btn_rewing,
        Btn_forward,
        Btn_repeat,
        Btn_shuffle,
        Btn_repeat_overlay,
        Btn_shuffle_overlay,
        Icon_nav_drawer,
        Icon_actions,
        Icon_back,
        Icon_accept,
        Icon_accept_dark,
        Icon_remove,
        Icon_remove_dark,
        Battery_20,
        Battery_20_charging,
        Battery_30,
        Battery_30_charging,
        Battery_50,
        Battery_50_charging,
        Battery_60,
        Battery_60_charging,
        Battery_80,
        Battery_80_charging,
        Battery_90,
        Battery_90_charging,
        Battery_full,
        Battery_full_charging,
        Battery_low,
        Battery_unknown,
        Icon_lock,
        Tex_count
    };
    enum Font
    {
        Roboto_large = 0,
        Roboto,
        Roboto_small,
        Roboto_osk,
        Font_count
    };

    GUI();
    ~GUI();
    void Initialize();
    void Shutdown();
    void Render(const double timer);
    void Update(const double timer, const u64 kDown);
    SDL_Texture* TextureHandle(const GUI::Texture id) const { return textures[id]; }
    TTF_Font*    FontHandle(const GUI::Font id)       const { return fonts[id];    }

private:
    // Status Bar
    void StatusBarGetBatteryStatus(int x, int y);
    void StatusBarDisplayTime(void);

    // Resources
    void LoadTextures();
    void FreeTextures();
    void LoadFonts();
    void FreeFonts();

    SDL_Texture* textures[Texture::Tex_count];
    TTF_Font*    fonts[Font::Font_count];

    // Components // TODO: array of GUIComponents
    GUIFileBrowser* guiFileBrowser;
};
