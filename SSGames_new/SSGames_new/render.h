#pragma once
#include "imgui/imgui_internal.h"
#define U8(str) reinterpret_cast<const char*>(str)

namespace menufunctional
{
    static std::string role;
    namespace deletefunc
    {
        static bool open = false;
    }
    namespace timeflags
    {
        static float messageTimer_Done = 0.0f;
        static bool showMessage_Done = false;
    }
    namespace filecheck
    {
        static bool checkheadlines = false;
    }
    namespace animationtab
    {
        const float pos_buttonX = 14.0f;
        const float pos_buttonY = 40.0f;
        static float animpos = 55.0f;
        float speedanim = 4;
    }
    namespace settings
    {
        static bool open = false;
        static bool pintab = true;
        static bool showLanguageOptions = false;
        static int language = 1;
        static bool timeOpen = true;
        static bool separateexcelwindow = false;
    }
    
    namespace info
    {
        static bool open = false;
    }

    namespace menu_main_button
    {
        static bool loading = false;
        float start_time = 0.0f;
        static float Timer_Loading = 0.0f;
        static int tab = 0;
        bool closemenu = false;
    }
    
    namespace login {
        std::string currentUser;
        static bool islogining = false;
    }

    namespace editcolormenu {
        static bool open = false;

        static ImVec4 FrameBg = ImVec4(0.556f, 0.785f, 0.0f, 1.0f); // 0.3f, 0.26f, 0.41f, 0.75f
        static ImVec4 CheckMarker = ImVec4(1.0f, 0.0f, 0.37f, 1.0f); // 1.0f, 0.0f, 0.37f, 1.0f
        static ImVec4 Button = ImVec4(0.14f, 0.87f, 0.214f, 1.0f);   // 0.33f, 0.14f, 0.87f, 1.0f
        static ImVec4 ButtonHovered = ImVec4(0.677f, 0.916f, 0.097f, 1.0f); // 0.26f, 0.12f, 0.65f, 1.0f
        static ImVec4 ButtonActive = ImVec4(0.556f, 0.785f, 0.0f, 1.0f); // 0.28f, 0.14f, 0.5f, 1.0f
        static ImVec4 SliderGrab = ImVec4(0.983f, 0.874f, 0.029f, 1.0f); // 0.24f, 0.02f, 0.5f, 1.0f
        static ImVec4 SliderActive = ImVec4(0.0476f, 0.882f, 0.06f, 1.0f); // 0.31f, 0.24f, 0.39f, 1.0f
        static ImVec4 SliderHovered = ImVec4(0.799f, 0.928f, 0.141f, 1.0f); // 0.25f, 0.2f, 0.31f, 1.0f
        static ImVec4 Text = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // 0.0f, 0.0f, 0.0f, 1.0f
        static ImVec4 TextError = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // 1.0f, 0.0f, 0.0f, 1.0f
        static ImVec4 WindowBg = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // 1.0f, 1.0f, 1.0f, 1.0f new
        static ImVec4 Button_delete = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        static ImVec4 Button_delete_Hovered = ImVec4(1.0f, 0.388f, 0.388f, 1.0f);
        static ImVec4 Button_delete_Active = ImVec4(0.722f, 0.0f, 0.0f, 1.0f);
        static ImVec4 Button_confirmdelete = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        static ImVec4 Button_confirmdelete_Hovered = ImVec4(1.0f, 0.329f, 0.329f, 1.0f);
        static ImVec4 Button_confirmdelete_Active = ImVec4(0.675f, 0.0f, 0.0f, 1.0f);
        static ImVec4 Button_cancle = ImVec4(0.0f, 0.481f, 1.0f, 1.0f);
        static ImVec4 Button_cancle_Hovered = ImVec4(0.072f, 0.189f, 1.0f, 1.0f);
        static ImVec4 Button_cancle_Active = ImVec4(0.0f, 0.709f, 1.0f, 1.0f);

        static float FrameRounding = 4;
        static float GrabRounding = 4;
    }
}

//namespace headername
//{
//    namespace main
//    {
//        const std::string ID = "ID";
//        const std::string Name = "Name";
//        const std::string Price = "Price";
//        const std::string Quantity = "Quantity";
//        const std::string Genre = "Genre";
//        const std::string Developer = "Developer";
//        const std::string Platform = "Platform";
//    }
//}


//struct Game {
//    int ID; 
//    std::string Name; 
//    double Price; 
//    int Quantity; 
//    int IDGenre;           // id genre = genre -> id -> name
//    int IDDeveloper;      // id developer = developer -> id -> name
//    int IDPlatform;      // id platform = platform -> id -> name
//};

//struct Genre {
//    int ID;
//    std::string name;
//};
//
//struct Developer {
//    int ID;
//    std::string name;
//};
//
//struct Platform {
//    int ID;
//    std::string name;
//};

inline void MainStyle() // Установить цвет
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.Colors[ImGuiCol_FrameBg] = menufunctional::editcolormenu::FrameBg;
    style.Colors[ImGuiCol_CheckMark] = menufunctional::editcolormenu::CheckMarker;
    style.Colors[ImGuiCol_Button] = menufunctional::editcolormenu::Button;
    style.Colors[ImGuiCol_ButtonHovered] = menufunctional::editcolormenu::ButtonHovered;
    style.Colors[ImGuiCol_ButtonActive] = menufunctional::editcolormenu::ButtonActive;
    style.Colors[ImGuiCol_SliderGrab] = menufunctional::editcolormenu::SliderGrab;
    style.Colors[ImGuiCol_FrameBgActive] = menufunctional::editcolormenu::SliderActive;
    style.Colors[ImGuiCol_FrameBgHovered] = menufunctional::editcolormenu::SliderHovered;
    style.Colors[ImGuiCol_Text] = menufunctional::editcolormenu::Text;
    style.Colors[ImGuiCol_WindowBg] = menufunctional::editcolormenu::WindowBg;
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.71f, 0.71f, 0.71f,1.0f);

    style.FrameRounding = menufunctional::editcolormenu::FrameRounding;
    style.GrabRounding = menufunctional::editcolormenu::GrabRounding;
}

//void PinWindow(const char* name, int posx, int posy)
//{
//    const auto& window = ImGui::FindWindowByName(name);
//    ImGui::SetNextWindowPos(ImVec2(window->Pos.x - window->Size.x / (ImGui::GetStyle().WindowPadding.x) + posx, window->Pos.y + posy));
//}

void Language() { // изменить язык

    //ImGui::Combo(menufunctional::settings::language == 0 ? "Select Language": "Выберите язык", &menufunctional::settings::language, menufunc::langue::languages, IM_ARRAYSIZE(menufunc::langue::languages));
    if (ImGui::Button(menufunctional::settings::language == 0 ? "Choose a language" : U8(u8"Выбрать язык"))) {
        menufunctional::settings::showLanguageOptions = !menufunctional::settings::showLanguageOptions;
    }

    if (menufunctional::settings::showLanguageOptions) {
        if (menufunctional::settings::timeOpen) {
            ImVec2 mousePos = ImGui::GetMousePos();
            float msposx = mousePos.x;
            float msposy = mousePos.y;
            ImGui::SetNextWindowPos(ImVec2(msposx - 60, msposy + 30), ImGuiCond_Always);
            menufunctional::settings::timeOpen = false;
        }
        ImGui::SetNextWindowSize(ImVec2(150, 30));
        ImGui::Begin("Langue", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
        /*menufunctional::settings::language == 0 ? ImGui::SetCursorPos(ImVec2(5,0)) : ImGui::SetCursorPos(ImVec2(20,0));
        ImGui::Separator();*/
        ImGui::SameLine();
        ImGui::SetCursorPosX(5);
        if (ImGui::Button("English")) {
            menufunctional::settings::language = 0;
            menufunctional::settings::showLanguageOptions = false;
        }
        ImGui::SameLine();
        if (ImGui::Button(U8(u8"Русский"))) {
            menufunctional::settings::language = 1;
            menufunctional::settings::showLanguageOptions = false;
        }
        ImGui::End();
    }
}


void UpdateLoadingState() {// обновления загрузки
   
    if (menufunctional::menu_main_button::loading) {
       
        float elapsed = ImGui::GetTime() - menufunctional::menu_main_button::start_time;

        
        if (elapsed >= 1.0f) {
            menufunctional::menu_main_button::loading = false;
        }
    }
    else {
       
            menufunctional::menu_main_button::loading = false;
            menufunctional::menu_main_button::start_time = ImGui::GetTime();
        
    }
}
