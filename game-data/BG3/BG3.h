//
// Created by charles on 27/09/23.
//

#ifndef LAMP_BG3_H
#define LAMP_BG3_H
#include "../gameControl.h"
#include "../../Lamp/Filesystem/lampFS.h"
#include "../../Lamp/Parse/lampParse.h"
namespace Lamp::Game {
    typedef Core::Base::lampTypes::lampString lampString;
    typedef Core::Base::lampTypes::lampHexAlpha lampHex;
    typedef Core::Base::lampTypes::lampReturn lampReturn;

    class BG3 : public gameControl {
    public:

        lampReturn registerArchive(lampString Path) override;
        lampReturn ConfigMenu() override;
        lampReturn startDeployment() override;
        lampReturn preCleanUp() override;
        lampReturn preDeployment() override;
        lampReturn deployment() override;
        lampReturn postDeploymentTasks() override;
        void listArchives() override;

        std::vector<Core::Base::lampMod::Mod *> ModList {};

        std::map<std::string,std::string>& KeyInfo() override {
            return keyInfo;
        };

        Core::Base::lampTypes::lampIdent Ident() override {
            return {"Baldur's Gate 3","BG3"};
        };

        std::vector<Core::Base::lampMod::Mod *>& getModList() override;

        void launch() override {
            for (const auto& pair : keyInfo) {
                const std::string& key = pair.first;
                keyInfo[key] = (std::string) Lamp::Core::FS::lampIO::loadKeyData(key,Ident().ShortHand).returnReason;
                if(key == "ProfileList"){
                    if(pair.second == "" || pair.second == "Default") {
                        keyInfo[key] = "Default";
                        Lamp::Core::FS::lampIO::saveKeyData(key, keyInfo[key], Ident().ShortHand);
                    }
                }
                if(key == "CurrentProfile"){
                    if(pair.second == "" || pair.second == "Default") {
                        keyInfo[key] = "Default";
                        Lamp::Core::FS::lampIO::saveKeyData(key, keyInfo[key], Ident().ShortHand);
                    }
                }
            }
            ModList = Lamp::Core::FS::lampIO::loadModList(Ident().ShortHand, keyInfo["CurrentProfile"]);
        }

    private:
        enum ModType{
            BG3_ENGINE_INJECTION = 0,
            BG3_MOD,
            BG3_BIN_OVERRIDE,
            BG3_DATA_OVERRIDE,
            BG3_MOD_FIXER,
            NaN
        };

        std::map<std::string,std::string> keyInfo{
                {"installDirPath",""},
                {"appDataPath",""},
                {"ProfileList","Default"},
                {"CurrentProfile", "Default"}
        };
    };
}

#endif //LAMP_BG3_H
