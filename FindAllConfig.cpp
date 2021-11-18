#include "FindAllConfig.h"

std::string Output::PluginName      = "FindAll";
ConfigLoader* ConfigLoader::pLoader = nullptr;

FindAllConfig::FindAllConfig()
    : ConfigBase("FindAll")
    , mSettings(FindAllSettings_t())
{
}

ConfigSaveStyle FindAllConfig::GetDefaultSaveStyle()
{
    return ConfigSaveStyle::CharacterSpecific;
}

bool FindAllConfig::GetCacheToDisc()
{
    return mSettings.CacheToDisc;
}
FindAllDisplayMode FindAllConfig::GetDisplayMode()
{
    return mSettings.DisplayMode;
}
int32_t FindAllConfig::GetDisplayMax()
{
    return mSettings.DisplayMax;
}
bool FindAllConfig::GetInstantLoad()
{
    return mSettings.InstantLoad;
}
uint32_t FindAllConfig::GetWritePeriod()
{
    return mSettings.WritePeriod;
}

void FindAllConfig::SetCacheToDisc(bool value)
{
    if (mSettings.CacheToDisc != value)
    {
        mSettings.CacheToDisc = value;
        ConfigLoader::pLoader->SaveSettings();
    }
}
void FindAllConfig::SetDisplayMax(int32_t value)
{
    int32_t newMax = min(20, max(1, value));
    if (newMax != mSettings.DisplayMax)
    {
        mSettings.DisplayMax = newMax;
        ConfigLoader::pLoader->SaveSettings();
    }
}
void FindAllConfig::SetDisplayMode(FindAllDisplayMode mode)
{
    if (mSettings.DisplayMode != mode)
    {
        mSettings.DisplayMode = mode;
        ConfigLoader::pLoader->SaveSettings();
    }
}
void FindAllConfig::SetInstantLoad(bool value)
{
    if (mSettings.InstantLoad != value)
    {
        mSettings.InstantLoad = value;
        ConfigLoader::pLoader->SaveSettings();
    }
}
void FindAllConfig::SetWritePeriod(uint32_t milliseconds)
{
    if (mSettings.WritePeriod != milliseconds)
    {
        mSettings.WritePeriod = milliseconds;
        ConfigLoader::pLoader->SaveSettings();
    }
}

bool FindAllConfig::LoadConfig(xml_node<>* pRoot, const char* errorBuffer)
{
    for (xml_node<>* pNode = pRoot->first_node(); pNode; pNode = pNode->next_sibling())
    {
        if (_stricmp(pNode->name(), "instantload") == 0)
        {
            SetInstantLoad(_stricmp(pNode->value(), "enabled") == 0);
        }
        else if (_stricmp(pNode->name(), "cachetodisc") == 0)
        {
            SetCacheToDisc(_stricmp(pNode->value(), "enabled") == 0);
        }
        else if (_stricmp(pNode->name(), "displaymax") == 0)
        {
            SetDisplayMax(atoi(pNode->value()));
        }
        else if (_stricmp(pNode->name(), "displaymode") == 0)
        {
            if (_stricmp(pNode->value(), "imgui") == 0)
                SetDisplayMode(FindAllDisplayMode::ImGui);
            else
                SetDisplayMode(FindAllDisplayMode::ChatLog);
        }
        else if (_stricmp(pNode->name(), "writedelay") == 0)
        {
            SetWritePeriod(static_cast<uint32_t>(atoll(pNode->value())));
        }
    }
    return true;
}
void FindAllConfig::Reset()
{
    mSettings = FindAllSettings_t();
}
bool FindAllConfig::SaveConfig(ofstream* pStream, const char* errorBuffer)
{
    *pStream << "    <!--When set to enabled, this plugin will memory swap inventory struct in upon zoning, so it is immediately accessible.-->\n";
    *pStream << "    <instantload>" << (GetInstantLoad() ? "enabled" : "disabled") << "</instantload>\n\n";

    *pStream << "    <!--When set to enabled, this plugin will store your items to a local cache so they can be searched even if the character is offline.-->\n";
    *pStream << "    <cachetodisc>" << (GetCacheToDisc() ? "enabled" : "disabled") << "</cachetodisc>\n\n";

    *pStream << "    <!--The maximum number of item IDs to display when in imgui mode.  Must be between 1 and 20.-->\n";
    *pStream << "    <displaymax>" << std::to_string(GetDisplayMax()) << "</displaymax>\n\n";

    *pStream << "    <!--When set to imgui, this plugin will display search results in an imgui window.  When set to chatlog, they will be displayed in chat log.-->\n";
    *pStream << "    <displaymode>" << ((GetDisplayMode() == FindAllDisplayMode::ImGui) ? "imgui" : "chatlog") << "</displaymode>\n\n";

    *pStream << "    <!--The delay, in milliseconds, after an item changes before it is written to cache.  This prevents extensive writes when inventory is frequently changing.-->\n";
    *pStream << "    <writedelay>" << std::to_string(GetWritePeriod()) << "</writedelay>\n\n";
    return true;
}