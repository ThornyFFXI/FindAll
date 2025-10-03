#ifndef __ASHITA_ThornyConfigLoader_H_INCLUDED__
#define __ASHITA_ThornyConfigLoader_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "Ashita.h"
#include "Output.h"
#include "thirdparty/rapidxml.hpp"
#include <filesystem>
#include <fstream>

using namespace rapidxml;
using namespace std;

enum class ConfigSaveStyle
{
    DefaultOnly       = 0,
    CharacterSpecific = 1
};

class ConfigBase
{
private:
    char mLoadedFilePath[256];
    char mPluginName[256];

public:
    ConfigBase(const char* pluginName)
    {
        SetFilePath("N/A");
        SetPluginName(pluginName);
    }

    const char* GetFilePath()
    {
        return mLoadedFilePath;
    }
    bool GetIsCharacterFile()
    {
        int length = strlen(mLoadedFilePath);
        return ((strcmp(mLoadedFilePath + (length - 12), "\\default.xml") != 0) && (strcmp(mLoadedFilePath, "N/A") != 0));
    }
    const char* GetPluginName()
    {
        return mPluginName;
    }

    void SetFilePath(const char* path)
    {
        strcpy_s(mLoadedFilePath, 256, path);
    }
    void SetPluginName(const char* pluginName)
    {
        strcpy_s(mPluginName, 256, pluginName);
    }

    virtual ConfigSaveStyle GetDefaultSaveStyle()                            = 0;
    virtual void Reset()                                                     = 0;
    virtual bool LoadConfig(xml_node<>* pRoot, const char* errorBuffer)      = 0;
    virtual bool SaveConfig(std::ofstream* pStream, const char* errorBuffer) = 0;
};

class ConfigLoader
{
private:
    bool mAutoSave;
    ConfigSaveStyle mSaveStyle;
    IAshitaCore* pAshitaCore;
    ConfigBase* pConfig;
    char mCharacterName[32];
    uint32_t mCharacterId;

public:
    static ConfigLoader* pLoader;

    ConfigLoader(IAshitaCore* pAshitaCore, ConfigBase* pConfig)
        : pAshitaCore(pAshitaCore)
        , pConfig(pConfig)
    {
        mCharacterId       = 0;
        uint16_t myIndex   = pAshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0);
        const char* myName = pAshitaCore->GetMemoryManager()->GetEntity()->GetName(myIndex);
        if ((myIndex > 0) && (myName) && (strlen(myName) > 2))
        {
            strcpy_s(mCharacterName, 32, pAshitaCore->GetMemoryManager()->GetEntity()->GetName(myIndex));
            mCharacterId = pAshitaCore->GetMemoryManager()->GetEntity()->GetServerId(myIndex);
        }
        mAutoSave  = false;
        mSaveStyle = pConfig->GetDefaultSaveStyle();
        LoadSettings();
    }

    const char* GetCharacterName()
    {
        return mCharacterName;
    }
    uint32_t GetCharacterId()
    {
        return mCharacterId;
    }

    void Handle0x00A(const uint8_t* data)
    {
        char nameBuffer[17] = {0};
        memcpy(nameBuffer, data + 0x84, 16);
        uint32_t id = *((uint32_t*)(data + 4));
        if ((strcmp(nameBuffer, mCharacterName) != 0) || (mCharacterId != id))
        {
            strcpy_s(mCharacterName, 32, nameBuffer);
            mCharacterId = id;
            if ((strlen(mCharacterName) < 3) || (mCharacterId == 0))
            {
                memset(mCharacterName, 0, 32);
                mCharacterId = 0;
            }
            LoadSettings();
        }
    }
    void LoadSettings()
    {
        CreateDirectories();
        Reset(true);
        char defaultBuffer[256];
        sprintf_s(defaultBuffer, 256, "%sconfig\\plugins\\%s\\default.xml", pAshitaCore->GetInstallPath(), pConfig->GetPluginName());
        char personalBuffer[256];
        sprintf_s(personalBuffer, 256, "%sconfig\\plugins\\%s\\%s_%u.xml", pAshitaCore->GetInstallPath(), pConfig->GetPluginName(), mCharacterName, mCharacterId);

        //Load default config to get save settings.
        pConfig->SetFilePath(defaultBuffer);
        if (std::filesystem::exists(defaultBuffer))
        {
            if (!LoadFile())
            {
                Reset(false);
                return;
            }
        }

        //If a default config doesn't exist, write it.
        else
        {
            WriteFile();
        }

        //If we have a character id, and per character settings are enabled, load them.
        if ((mSaveStyle == ConfigSaveStyle::CharacterSpecific) && (mCharacterId != 0))
        {
            pConfig->SetFilePath(personalBuffer);
            if (std::filesystem::exists(personalBuffer))
            {
                if (!LoadFile())
                {
                    Reset(false);
                    return;
                }
            }

            //If per-character settings don't exist for this character yet, we know the default loaded so we can write that.
            else
            {
                mAutoSave = true;
                WriteFile();
            }
        }
    }
    void SaveSettings()
    {
        if ((this) && (mAutoSave) && (strcmp(pConfig->GetFilePath(), "N/A") != 0))
        {
            WriteFile();
        }
    }

private:
    void CreateDirectories()
    {
        string makeDirectory = string(pAshitaCore->GetInstallPath()) + "config\\plugins\\" + std::string(pConfig->GetPluginName()) + "\\";
        if (std::filesystem::exists(makeDirectory))
            return;

        size_t nextDirectory = makeDirectory.find("\\");
        nextDirectory        = makeDirectory.find("\\", nextDirectory + 1);
        while (nextDirectory != string::npos)
        {
            string currentDirectory = makeDirectory.substr(0, nextDirectory + 1);
            if ((!CreateDirectory(currentDirectory.c_str(), NULL)) && (ERROR_ALREADY_EXISTS != GetLastError()))
            {
                OutputHelper::Outputf(Ashita::LogLevel::Error, "Could not find or create folder: $H%s$R", currentDirectory.c_str());
                return;
            }
            nextDirectory = makeDirectory.find("\\", nextDirectory + 1);
        }
    }

    void Reset(bool silent)
    {
        mAutoSave  = false;
        mSaveStyle = pConfig->GetDefaultSaveStyle();
        pConfig->SetFilePath("N/A");
        pConfig->Reset();
        if (!silent)
        {
            OutputHelper::Output(Ashita::LogLevel::Info, "Default settings restored.");
        }
    }
    bool LoadFile()
    {
        char path[256];
        strcpy_s(path, 256, pConfig->GetFilePath());
        Reset(true);
        std::ifstream inputStream = ifstream(path, ios::in | ios::binary | ios::ate);
        if (!inputStream.is_open())
        {
            OutputHelper::Outputf(Ashita::LogLevel::Error, "Failed to read file: $H%s$R", path);
            return false;
        }

        long size    = inputStream.tellg();
        char* buffer = nullptr;

        try
        {
            buffer = new char[size + 1];
            inputStream.seekg(0, ios::beg);
            inputStream.read(buffer, size);
            buffer[size] = '\0';
            inputStream.close();
        }
        catch (const std::exception e)
        {
            if (buffer != nullptr)
                delete[] buffer;

            OutputHelper::Outputf(Ashita::LogLevel::Error, "Exception while reading $H%s$R: %s$H%s$R", path, e.what());
            return false;
        }
        catch (...)
        {
            if (buffer != nullptr)
                delete[] buffer;

            OutputHelper::Outputf(Ashita::LogLevel::Error, "Unknown exception while reading file: $H%s$R", path);
            return false;
        }

        xml_document<>* pDocument = new xml_document<>();
        try
        {
            pDocument->parse<0>(buffer);
        }
        catch (const rapidxml::parse_error& e)
        {
            int line = static_cast<long>(std::count(buffer, e.where<char>(), '\n') + 1);
            OutputHelper::Outputf(Ashita::LogLevel::Error, "Parse error while evaluating $H%s$R on line $H%d$R.", path, line);
            OutputHelper::Outputf(Ashita::LogLevel::Error, "Error message: %H%s$R", e.what());
            delete pDocument;
            delete[] buffer;
            return false;
        }
        catch (...)
        {
            OutputHelper::Outputf(Ashita::LogLevel::Error, "Parse error while evaluating $H%s$R.  No message specified.", path);
            delete pDocument;
            delete[] buffer;
            return false;
        }

        xml_node<>* pRoot = pDocument->first_node();
        if (_stricmp(pRoot->name(), pConfig->GetPluginName()))
        {
            OutputHelper::Outputf(Ashita::LogLevel::Error, "Parse error while evaluating $H%s$R.  Root node did not match plugin name.", path);
            delete pDocument;
            delete[] buffer;
            return false;
        }

        xml_node<>* pNode = nullptr;
        pNode             = pRoot->first_node("autosave");
        if (pNode)
        {
            if (_stricmp(pNode->value(), "enabled") == 0)
            {
                mAutoSave = true;
            }
            else
            {
                mAutoSave = false;
            }
        }

        if (!pConfig->GetIsCharacterFile())
        {
            pNode = pRoot->first_node("characterspecificsettings");
            if (pNode)
            {
                if (_stricmp(pNode->value(), "enabled") == 0)
                {
                    mSaveStyle = ConfigSaveStyle::CharacterSpecific;
                }
                else
                {
                    mSaveStyle = ConfigSaveStyle::DefaultOnly;
                }
            }
        }

        //Let plugin config class handle load/save
        char errorBuffer[256];
        bool result = pConfig->LoadConfig(pRoot, errorBuffer);
        delete pDocument;
        delete[] buffer;
        if (!result)
        {
            OutputHelper::Outputf(Ashita::LogLevel::Error, "Plugin failed to load settings file: $H%s$R", path);
            OutputHelper::Outputf(Ashita::LogLevel::Error, "Error: %s", errorBuffer);
            return false;
        }
        pConfig->SetFilePath(path);
        return true;
    }

    bool WriteFile()
    {
        //Attempt to create a stream.
        const char* path           = pConfig->GetFilePath();
        bool exists                = std::filesystem::exists(path);
        std::ofstream outputStream = ofstream(path);
        if (!outputStream.is_open())
        {
            OutputHelper::Outputf(Ashita::LogLevel::Error, "Failed to write file: $H%s$R", path);
            return false;
        }

        //Write header

        std::string pluginName = pConfig->GetPluginName();
        for (int x = 0; x < pluginName.length(); x++)
        {
            pluginName[x] = tolower(pluginName[x]);
        }

        outputStream << "<" << pluginName << ">\n\n";
        outputStream << "    <!--When set to enabled, this plugin will be allowed to modify this xml when you change settings ingame.-->\n";
        outputStream << "    <autosave>" << (mAutoSave ? "enabled" : "disabled") << "</autosave>\n\n";
        if (!pConfig->GetIsCharacterFile())
        {
            outputStream << "    <!--When set to enabled, this plugin will create a seperate settings file for each character that loads it.-->\n";
            outputStream << "    <!--When set to anything else, this plugin will only use default.xml for all characters.-->\n";
            outputStream << "    <!--This setting does nothing in any configuration file besides default.xml.-->\n";
            outputStream << "    <characterspecificsettings>" << ((mSaveStyle == ConfigSaveStyle::CharacterSpecific) ? "enabled" : "disabled") << "</characterspecificsettings>\n\n";
        }

        //Let plugin config class handle load/save
        char errorBuffer[256];
        bool result = pConfig->SaveConfig(&outputStream, errorBuffer);
        if (!result)
        {
            OutputHelper::Outputf(Ashita::LogLevel::Error, "Plugin failed to save settings file: $H%s$R", path);
            OutputHelper::Outputf(Ashita::LogLevel::Error, "Error: %s", errorBuffer);
            outputStream.close();
            return false;
        }
        outputStream << "</" << pluginName << ">";
        outputStream.close();
        if (!exists)
        {
            OutputHelper::Outputf(Ashita::LogLevel::Info, "Wrote new settings file: $H%s$R", path);
        }

        return true;
    }
};
#endif