//
// Created by charles on 27/09/23.
//

#include <regex>
#include "BG3.h"
#include "../../Lamp/Control/lampControl.h"
#include "../../third-party/json/json.hpp"

Lamp::Game::lampReturn Lamp::Game::BG3::registerArchive(Lamp::Game::lampString Path) {

    for (Core::Base::lampMod::Mod* it : ModList) {

        std::filesystem::path NewFilePath = Path;
        std::filesystem::path TestingAgainstPath = it->ArchivePath;


        std::string NewFilePathCut = NewFilePath.filename();
        size_t posA = NewFilePathCut.find('-');
        if (posA != std::string::npos) {
            NewFilePathCut.erase(posA);
        }

        std::string TestingAgainstPathCut = TestingAgainstPath.filename();
        size_t posB = TestingAgainstPathCut.find('-');
        if (posB != std::string::npos) {
            TestingAgainstPathCut.erase(posB);
        }


        if(NewFilePathCut == TestingAgainstPathCut){

            it->timeOfUpdate = Lamp::Core::lampControl::getFormattedTimeAndDate();
            it->ArchivePath = Path;
            return Lamp::Core::FS::lampIO::saveModList(Ident().ShortHand,ModList,Games::getInstance().currentProfile);
        }


    }

    Core::Base::lampMod::Mod  * newArchive = new Core::Base::lampMod::Mod{Path,ModType::NaN, false};
    newArchive->timeOfUpdate = Lamp::Core::lampControl::getFormattedTimeAndDate();
    ModList.push_back(newArchive);
    return Lamp::Core::FS::lampIO::saveModList(Ident().ShortHand,ModList,Games::getInstance().currentProfile);
}

Lamp::Game::lampReturn Lamp::Game::BG3::ConfigMenu() {
    ImGui::Separator();
    Lamp::Core::lampControl::lampGameSettingsDisplayHelper("BG3 Steam Directory", keyInfo["installDirPath"],
                                                           "This is usually (steampath)/steamapps/common/Baldurs Gate 3",
                                                            "installDirPath").createImGuiMenu();
    ImGui::Separator();
    Lamp::Core::lampControl::lampGameSettingsDisplayHelper("BG3 AppData Directory", keyInfo["appDataPath"],
                                                           "This is usually (steampath)/steamapps/compatdata/1086940/pfx/drive_c/users/steamuser/AppData/Local/Larian Studios/Baldur's Gate 3",
                                                           "appDataPath").createImGuiMenu();
    return false;
}

Lamp::Game::lampReturn Lamp::Game::BG3::startDeployment() {
    Lamp::Core::lampControl::getInstance().inDeployment = true;
    if(KeyInfo()["installDirPath"] == "" | KeyInfo()["appDataPath"] == "" ) {
        Core::Base::lampLog::getInstance().log("Game Configuration not set.", Core::Base::lampLog::warningLevel::WARNING, true, Core::Base::lampLog::LMP_NOCONFIG);
    }

    Core::Base::LampSequencer::add("BG3 Deployment Queue", [this]() -> lampReturn {
        Lamp::Core::lampControl::getInstance().deploymentStageTitle = "Preparing";
        auto result = preCleanUp();
        if(!result) Core::Base::lampLog::getInstance().log("Pre cleanup has failed.", Core::Base::lampLog::warningLevel::ERROR, true, Core::Base::lampLog::LMP_CLEANUPFAILED);
        return result;
    });

    Core::Base::LampSequencer::add("BG3 Deployment Queue", [this]() -> lampReturn {
        Lamp::Core::lampControl::getInstance().deploymentStageTitle = "Pre Deployment";
        auto result = preDeployment();
        if(!result) Core::Base::lampLog::getInstance().log("Pre Deployment has failed.", Core::Base::lampLog::warningLevel::ERROR, true, Core::Base::lampLog::LMP_PREDEPLOYFAILED);
        return result;
    });

    Core::Base::LampSequencer::add("BG3 Deployment Queue", [this]() -> lampReturn {
        Lamp::Core::lampControl::getInstance().deploymentStageTitle = "Deploying";
        auto result = deployment();
        if(!result) Core::Base::lampLog::getInstance().log("Deployment has failed.", Core::Base::lampLog::warningLevel::ERROR, true, Core::Base::lampLog::LMP_DEOPLYMENTFAILED);
        return result;
    });


    Core::Base::LampSequencer::run("BG3 Deployment Queue");
    Lamp::Core::lampControl::getInstance().inDeployment = false;
    return false;
}

Lamp::Game::lampReturn Lamp::Game::BG3::preCleanUp() {

    std::string workingDir = Lamp::Core::lampConfig::getInstance().DeploymentDataPath + Ident().ReadableName;
    Lamp::Core::lampControl::getInstance().deplopmentTracker = {0,6};

    std::filesystem::path dir(workingDir);
    Core::Base::lampLog::getInstance().log("Cleaning Working Directory : "+workingDir, Core::Base::lampLog::warningLevel::LOG);
    try {
        for (const auto &entry: std::filesystem::directory_iterator(dir)) {
            std::filesystem::remove_all(entry.path());
        }
    }catch(std::exception e){
        return {-1, "Cannot clean working directories"};
    }
    Lamp::Core::lampControl::getInstance().deplopmentTracker.first = 1;
    Core::Base::lampLog::getInstance().log("Cleaning BG3 Mods & Native Mods Folders", Core::Base::lampLog::warningLevel::LOG);
    try {
        if(Lamp::Core::FS::lampIO::emptyFolder(keyInfo["appDataPath"] + "/Mods/")){
            if(!Lamp::Core::FS::lampIO::emptyFolder(keyInfo["installDirPath"] + "/bin/NativeMods", "dll")){
                return {-1, "Unable to empty NativeMods folder."};
            }
        }else{
            return {-1, "Unable to empty Mods folder."};
        }
    }catch(std::exception e){
        return {-1, "Unable to empty folders."};
    }
    Lamp::Core::lampControl::getInstance().deplopmentTracker.first = 2;
    Core::Base::lampLog::getInstance().log("Creating Working Directories", Core::Base::lampLog::warningLevel::LOG);
    try {
        std::filesystem::create_directories(workingDir + "/bin/NativeMods");
        std::filesystem::create_directories(workingDir + "/Data");
        std::filesystem::create_directories(workingDir + "/Mods");
        std::filesystem::create_directories(workingDir + "/PlayerProfiles/Public");
        std::filesystem::create_directories(workingDir + "/ext");
    }catch(std::exception e){
        return {-1, "Unable to create working directories."};
    }
    Lamp::Core::lampControl::getInstance().deplopmentTracker.first = 3;
    Core::Base::lampLog::getInstance().log("Copying modsettings.lsx", Core::Base::lampLog::warningLevel::LOG);
    pugi::xml_document doc;
    if(std::filesystem::copy_file(keyInfo["appDataPath"] + "/PlayerProfiles/Public/modsettings.lsx",
                               workingDir + "/PlayerProfiles/Public/modsettings.lsx",
                               std::filesystem::copy_options::overwrite_existing)){

        pugi::xml_parse_result result = doc.load_file((workingDir + "/PlayerProfiles/Public/modsettings.lsx").c_str());
        if (result.status == pugi::status_ok) {

        } else {
            return {-1, "Invalid XML File."};
        }
    }else{
        return {-1, "Invalid XML File."};
    }
    Lamp::Core::lampControl::getInstance().deplopmentTracker.first = 4;
    Core::Base::lampLog::getInstance().log("Cleaning modsettings.lsx", Core::Base::lampLog::warningLevel::LOG);
    pugi::xml_node modOrderNode = doc.select_node("//node[@id='ModOrder']").node();
    pugi::xml_node modsNode = doc.select_node("//node[@id='Mods']").node();

    if (modOrderNode) {
        pugi::xml_node childrenNode = modOrderNode.child("children");
        if (childrenNode) {
            childrenNode.remove_children();
        }else{
            modOrderNode.append_child("children");
        }
    } else {
        return {-1, "ModOrder section not found."};
    }
    if (modsNode) {
        pugi::xml_node childrenNode = modsNode.child("children");
        if (childrenNode) {
            childrenNode.remove_children();
        }else{
            modsNode.append_child("children");
        }
    } else {
        return {-1, "Mods section not found."};
    }
    Lamp::Core::lampControl::getInstance().deplopmentTracker.first = 5;
    Core::Base::lampLog::getInstance().log("Injecting GustavDev into modsettings.lsx", Core::Base::lampLog::warningLevel::LOG);
    pugi::xml_node modsNode2 = doc.select_node("//node[@id='Mods']").node();
    pugi::xml_node childrenNode = modsNode2.child("children");

    pugi::xml_node newShortDescNode = childrenNode.append_child("node");
    newShortDescNode.append_attribute("id") = "ModuleShortDesc";

    pugi::xml_node folderAttrib = newShortDescNode.append_child("attribute");
    folderAttrib.append_attribute("id") = "Folder";
    folderAttrib.append_attribute("type") = "LSString";
    folderAttrib.append_attribute("value") = "GustavDev";

    pugi::xml_node md5Attrib = newShortDescNode.append_child("attribute");
    md5Attrib.append_attribute("id") = "MD5";
    md5Attrib.append_attribute("type") = "LSString";
    md5Attrib.append_attribute("value") = "";

    pugi::xml_node nameAttrib = newShortDescNode.append_child("attribute");
    nameAttrib.append_attribute("id") = "Name";
    nameAttrib.append_attribute("type") = "LSString";
    nameAttrib.append_attribute("value") = "GustavDev";

    pugi::xml_node uuidAttrib = newShortDescNode.append_child("attribute");
    uuidAttrib.append_attribute("id") = "UUID";
    uuidAttrib.append_attribute("type") = "FixedString";
    uuidAttrib.append_attribute("value") = "28ac9ce2-2aba-8cda-b3b5-6e922f71b6b8";


    pugi::xml_node versionAttrib = newShortDescNode.append_child("attribute");
    versionAttrib.append_attribute("id") = "Version64";
    versionAttrib.append_attribute("value") = "36028797018963968";
    versionAttrib.append_attribute("type") = "int64";

    Core::Base::lampLog::getInstance().log("Saving modsettings.lsx", Core::Base::lampLog::warningLevel::LOG);
    if (!doc.save_file((workingDir + "/PlayerProfiles/Public/modsettings.lsx").c_str())) {
        return {-1, "Failed to save modsettings.lsx"};
    }

    Lamp::Core::lampControl::getInstance().deplopmentTracker.first = 6;
    return {1, "PreCleanup Finished."};
}

Lamp::Game::lampReturn Lamp::Game::BG3::preDeployment() {
    Lamp::Core::lampControl::getInstance().deplopmentTracker = {0,0};
    Core::Base::lampLog::getInstance().log("Extracting Archives", Core::Base::lampLog::warningLevel::LOG);
    auto lambdaFunction = [](const Core::Base::lampMod::Mod* item) {
        if(item->enabled) {
            Lamp::Core::lampControl::getInstance().deplopmentTracker.second++;
        if(Lamp::Core::FS::lampExtract::extract(item)) {

            std::string workingDir = Lamp::Core::lampConfig::getInstance().DeploymentDataPath + Lamp::Games::getInstance().currentGame->Ident().ReadableName;
            std::filesystem::path dirit(workingDir + "/ext/" + std::filesystem::path(item->ArchivePath).filename().stem().string());

            try {
                for (const auto &entry: std::filesystem::directory_iterator(dirit)) {
                    if (entry.is_regular_file()) {
                        std::string fileName = entry.path().filename().string();
                        if (fileName.find('\\') != std::string::npos) {
                            std::string inputString = fileName;
                            size_t found = inputString.find('\\');
                            while (found != std::string::npos) {
                                inputString.replace(found, 1, "/");
                                found = inputString.find('\\', found + 1);
                            }
                            std::filesystem::path temp(inputString);
                            std::filesystem::rename(entry.path(),
                                                    workingDir +  "/ext/" + std::filesystem::path(item->ArchivePath).filename().stem().string() + "/" + temp.filename().string());
                            std::cout << "File with backslash in name: " << fileName << std::endl;
                        }
                    }
                }
            } catch (const std::exception &e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }




            std::string x = item->ArchivePath;
            switch (item->modType) {
                case BG3_ENGINE_INJECTION:
                    Lamp::Core::FS::lampExtract::moveModSpecificFileType(item,"dll","bin/NativeMods");
                    Lamp::Core::FS::lampExtract::moveModSpecificFileType(item,"toml","bin/NativeMods");
                    Lamp::Core::FS::lampExtract::moveModSpecificFileType(item,"ini","bin/NativeMods");
                    break;
                case BG3_MOD:
                    Lamp::Core::FS::lampExtract::moveModSpecificFileType(item,"pak","Mods");

                    Core::Base::LampSequencer::add("BG3 Settings Control", [x]() -> lampReturn {
                        if(!Lamp::Core::Parse::lampBG3PakParser::collectJsonData(x)){
                            return Lamp::Core::Parse::lampBG3PakParser::extractJsonData(x);
                        }else{
                            return true;
                        }
                    });
                    break;
                case BG3_BIN_OVERRIDE:
                    Lamp::Core::FS::lampExtract::moveModSpecificFolder(item,"bin","bin");
                    break;
                case BG3_DATA_OVERRIDE:
                    Lamp::Core::FS::lampExtract::moveModSpecificFolder(item,"Data","Data");
                    break;
                case BG3_MOD_FIXER:
                    Lamp::Core::FS::lampExtract::moveModSpecificFileType(item,"pak","Mods");
                    break;
                default:
                    break;
            }
            Lamp::Core::lampControl::getInstance().deplopmentTracker.first++;
        }
        }
    };
    lampExecutor executor(lambdaFunction);
    executor.execute(ModList);
    Core::Base::LampSequencer::run("BG3 Settings Control");
    return {1,"TESTER"};
}

Lamp::Game::lampReturn Lamp::Game::BG3::deployment() {
    std::string workingDir = Lamp::Core::lampConfig::getInstance().DeploymentDataPath + Ident().ReadableName;
    Lamp::Core::lampControl::getInstance().deplopmentTracker = {0,5};

    try {
        std::filesystem::path sourceDirectory = (std::string) workingDir+"/bin/";
        std::filesystem::path destinationDirectory = (std::string) keyInfo["installDirPath"]+"/bin/";
        Core::Base::lampLog::getInstance().log("Copying Bin");
        Lamp::Core::FS::lampIO::recursiveCopyWithIgnore(sourceDirectory,destinationDirectory,std::vector<std::string>{"NativeMods"});
        Lamp::Core::lampControl::getInstance().deplopmentTracker.first = 1;


        sourceDirectory = (std::string) workingDir+"/bin/NativeMods";
        destinationDirectory = (std::string) (keyInfo["installDirPath"]+"/bin/NativeMods");
        Core::Base::lampLog::getInstance().log("Copying NativeMods");
        Lamp::Core::FS::lampIO::copyExtensionWithConfigIgnore(sourceDirectory, destinationDirectory, ".dll");
        Lamp::Core::lampControl::getInstance().deplopmentTracker.first = 2;


        sourceDirectory = (std::string)workingDir+"/Data/";
        destinationDirectory = (std::string)keyInfo["installDirPath"]+"/Data/";
        Core::Base::lampLog::getInstance().log("Copying Data");
        std::filesystem::copy(sourceDirectory, destinationDirectory, std::filesystem::copy_options::update_existing | std::filesystem::copy_options::recursive);
        Lamp::Core::lampControl::getInstance().deplopmentTracker.first = 3;

        sourceDirectory = (std::string)workingDir+"/Mods";
        destinationDirectory = (std::string)KeyInfo()["appDataPath"]+"/Mods/";
        Core::Base::lampLog::getInstance().log("Copying Mods");
        std::filesystem::copy(sourceDirectory, destinationDirectory, std::filesystem::copy_options::update_existing | std::filesystem::copy_options::recursive);
        Lamp::Core::lampControl::getInstance().deplopmentTracker.first = 4;

        sourceDirectory =  (std::string)workingDir+"/PlayerProfiles";
        destinationDirectory = (std::string)KeyInfo()["appDataPath"]+"/PlayerProfiles/";
        Core::Base::lampLog::getInstance().log("Copying ModProfile");
        std::filesystem::copy(sourceDirectory, destinationDirectory, std::filesystem::copy_options::update_existing | std::filesystem::copy_options::recursive);
        Lamp::Core::lampControl::getInstance().deplopmentTracker.first = 5;

        return {1, "Deployed"};
    } catch (const std::exception& e) {
        return {0, "Deployment Failed."};
    }
}

Lamp::Game::lampReturn Lamp::Game::BG3::postDeploymentTasks() {

}

void Lamp::Game::BG3::listArchives() {
    Lamp::Core::lampControl::lampArchiveDisplayHelper(
            std::list<std::string>{},
            ModList,
            std::vector<std::string>{"Engine Injection", "Standard Mod", "Bin Overwrite","Data Overwrite","No Json Mod", "Select Type"},
            std::list<std::pair<std::string, bool *>>{}
    ).createImguiMenu();
}

std::vector<Lamp::Core::Base::lampMod::Mod *> &Lamp::Game::BG3::getModList() {
    return ModList;
}



