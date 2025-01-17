//
// Created by charles on 08/10/23.
//

#ifndef LAMP_LAMPCOLOUR_H
#define LAMP_LAMPCOLOUR_H

#include "../../third-party/imgui/imgui.h"
#include "../Base/lampBase.h"
#include "../Filesystem/lampFS.h"
#include "../../third-party/imgui/imgui_internal.h"

namespace Lamp {
    namespace Core {

        class lampColour {
        public:

            static lampColour& getInstance()
            {
                static lampColour instance;
                return instance;
            }

            lampColour(lampColour const&) = delete;
            void operator=(lampColour const&)  = delete;

            std::vector<std::string> defaultColours = {
                    "ffffff-ff",
                    "0a0d12-ff",
                    "141414-ff",
                    "260101-ff",
                    "071216-ff",
                    "590202-ff",
                    "a61103-ff",
                    "260101-ff",
                    "4296f9-4f",
                    "4296f9-cc",
                    "590202-ff",
                    "4296f9-ff",
                    "a61103-ff",
                    "1966bf-c6"
            };

            float floatMap[15][4] = {
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { 1.0f, 0.0f, 0.0f, 0.0f }
            };

            std::map<ImGuiCol_,std::string> styleMap = {
                    {ImGuiCol_Text,"Colour_Text"},
                    {ImGuiCol_WindowBg,"Colour_WindowBackground"},
                    {ImGuiCol_PopupBg, "Colour_ToolTipAndTypes"},
                    {ImGuiCol_FrameBg,"Colour_InputBG"},
                    {ImGuiCol_MenuBarBg, "Colour_MenuBar"},
                    {ImGuiCol_Button, "Colour_Button"},
                    {ImGuiCol_ButtonHovered,"Colour_Hover"},
                    {ImGuiCol_ButtonActive,"Colour_Pressed"},
                    {ImGuiCol_Header,"Colour_MenuItems"},
                    {ImGuiCol_HeaderHovered,"Colour_HeadHover"},
                    {ImGuiCol_HeaderActive,"Colour_HeadPressed"},
                    {ImGuiCol_Separator,"Colour_Separator"},
                    {ImGuiCol_SeparatorHovered,"Colour_SeparatorHover"}
            };


            void getConfigColours(){
                int x = 0;
                for (const auto& pair : styleMap) {
                    ImGuiCol_ key = pair.first;
                    const std::string& value = pair.second;

                    std::string loaded = Lamp::Core::FS::lampIO::loadKeyData(value, "LAMP CONFIG");
                    if(loaded == ""){
                        Lamp::Core::Base::lampTypes::lampHexAlpha colour = Lamp::Core::Base::lampTypes::lampHexAlpha(ImGui::GetStyle().Colors[key]);
                        Lamp::Core::FS::lampIO::saveKeyData(value, ((std::string)colour), "LAMP CONFIG");
                    }else{
                        ImGui::GetStyle().Colors[key] = Lamp::Core::Base::lampTypes::lampHexAlpha(loaded);
                    }


                    lampColour::getInstance().floatMap[x][0] = ImGui::GetStyle().Colors[key].x;
                    lampColour::getInstance().floatMap[x][1] = ImGui::GetStyle().Colors[key].y;
                    lampColour::getInstance().floatMap[x][2] = ImGui::GetStyle().Colors[key].z;
                    lampColour::getInstance().floatMap[x][3] = ImGui::GetStyle().Colors[key].w;


                    x++;
                }

                std::string xloaded = Lamp::Core::FS::lampIO::loadKeyData("Colour_SearchHighlight", "LAMP CONFIG");
                if(xloaded == ""){
                    Lamp::Core::Base::lampTypes::lampHexAlpha colour("a61103-ff");
                    Lamp::Core::FS::lampIO::saveKeyData("Colour_SearchHighlight", ((std::string)colour), "LAMP CONFIG");
                }else{
                    Lamp::Core::lampControl::getInstance().Colour_SearchHighlight = Lamp::Core::Base::lampTypes::lampHexAlpha(xloaded);
                }


                lampColour::getInstance().floatMap[x][0] = ((ImVec4)Lamp::Core::lampControl::getInstance().Colour_SearchHighlight).x;
                lampColour::getInstance().floatMap[x][1] = ((ImVec4)Lamp::Core::lampControl::getInstance().Colour_SearchHighlight).y;
                lampColour::getInstance().floatMap[x][2] = ((ImVec4)Lamp::Core::lampControl::getInstance().Colour_SearchHighlight).z;
                lampColour::getInstance().floatMap[x][3] = ((ImVec4)Lamp::Core::lampControl::getInstance().Colour_SearchHighlight).w;


            }

            void setColourTemp(ImGuiCol_ StylePoint, Lamp::Core::Base::lampTypes::lampHexAlpha colour){
                ImGui::GetStyle().Colors[StylePoint] = colour;
            }

            void setColour(ImGuiCol_ StylePoint, Lamp::Core::Base::lampTypes::lampHexAlpha colour){
                ImGui::GetStyle().Colors[StylePoint] = colour;
                Lamp::Core::FS::lampIO::saveKeyData(styleMap[StylePoint], ((std::string)colour), "LAMP CONFIG");
            }

            static bool displayColourMenu(){

                int x = 0;
                for (const auto& pair : lampColour::getInstance().styleMap) {
                    ImGuiCol_ key = pair.first;
                    const std::string &value = pair.second;
                    ImGui::ColorEdit4((pair.second+"##"+std::to_string(x)).c_str(),lampColour::getInstance().floatMap[x]);
                    lampColour::getInstance().setColourTemp(key,ImVec4(lampColour::getInstance().floatMap[x][0],lampColour::getInstance().floatMap[x][1],lampColour::getInstance().floatMap[x][2],lampColour::getInstance().floatMap[x][3]));
                    x++;
                }
                ImGui::ColorEdit4(("Colour_SearchHighlight##"+std::to_string(x)).c_str(),lampColour::getInstance().floatMap[x]);
                Lamp::Core::lampControl::getInstance().Colour_SearchHighlight = ImVec4(lampColour::getInstance().floatMap[x][0],lampColour::getInstance().floatMap[x][1],lampColour::getInstance().floatMap[x][2],lampColour::getInstance().floatMap[x][3]);




                if(ImGui::Button("Save")){
                    ImGui::End();

                    int x = 0;
                    for (const auto& pair : lampColour::getInstance().styleMap) {
                        ImGuiCol_ key = pair.first;
                        lampColour::getInstance().setColour(key,ImVec4(lampColour::getInstance().floatMap[x][0],lampColour::getInstance().floatMap[x][1],lampColour::getInstance().floatMap[x][2],lampColour::getInstance().floatMap[x][3]));
                        x++;
                    }

                    Lamp::Core::FS::lampIO::saveKeyData("Colour_SearchHighlight", ((std::string)Lamp::Core::lampControl::getInstance().Colour_SearchHighlight), "LAMP CONFIG");

                    return true;
                }
                ImGui::SameLine();
                if(ImGui::Button("Back")){
                        ImGui::End();
                        return true;
                }
                ImGui::SameLine();
                if(ImGui::Button("Reset")){
                    Lamp::Core::lampControl::getInstance().Colour_SearchHighlight = Lamp::Core::Base::lampTypes::lampHexAlpha("a61103-ff");

                    for (int i = 0; i < lampColour::getInstance().defaultColours.size(); ++i) {
                        ImVec4 color = Lamp::Core::Base::lampTypes::lampHexAlpha(lampColour::getInstance().defaultColours[i]);
                        lampColour::getInstance().floatMap[i][0] = color.x;
                        lampColour::getInstance().floatMap[i][1] = color.y;
                        lampColour::getInstance().floatMap[i][2] = color.z;
                        lampColour::getInstance().floatMap[i][3] = color.w;
                    }


                }

                ImGui::End();
                return false;
            }

        private:
            lampColour(){}
        };
    } // Lamp
} // Core

#endif //LAMP_LAMPCOLOUR_H
