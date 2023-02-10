#include "FindAllConfig.h"

ConfigLoader* ConfigLoader::pLoader = nullptr;

FindAllConfig::FindAllConfig()
    : ConfigBase("FindAll")
    , mSettings(FindAllSettings_t())
{
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
bool FindAllConfig::GetKeyItemPrefix()
{
    return mSettings.RequireKeyItemPrefix;
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
void FindAllConfig::SetKeyItemPrefix(bool required)
{
    if (mSettings.RequireKeyItemPrefix != required)
    {
        mSettings.RequireKeyItemPrefix = required;
        ConfigLoader::pLoader->SaveSettings();
    }
}
void FindAllConfig::SetWritePeriod(uint32_t milliseconds)
{
    uint32_t newPeriod = min(300000, milliseconds);
    if (mSettings.WritePeriod != newPeriod)
    {
        mSettings.WritePeriod = newPeriod;
        ConfigLoader::pLoader->SaveSettings();
    }
}

ConfigSaveStyle FindAllConfig::GetDefaultSaveStyle()
{
    return ConfigSaveStyle::CharacterSpecific;
}

bool FindAllConfig::LoadConfig(xml_node<>* pRoot, const char* errorBuffer)
{
    mSettings = FindAllSettings_t();
    for (xml_node<>* pNode = pRoot->first_node(); pNode; pNode = pNode->next_sibling())
    {
        if (_stricmp(pNode->name(), "instantload") == 0)
        {
            mSettings.InstantLoad = (_stricmp(pNode->value(), "enabled") == 0);
        }
        else if (_stricmp(pNode->name(), "cachetodisc") == 0)
        {
            mSettings.CacheToDisc = (_stricmp(pNode->value(), "enabled") == 0);
        }
        else if (_stricmp(pNode->name(), "displaymax") == 0)
        {
            mSettings.DisplayMax = atoi(pNode->value());
        }
        else if (_stricmp(pNode->name(), "displaymode") == 0)
        {
            if (_stricmp(pNode->value(), "imgui") == 0)
                mSettings.DisplayMode = FindAllDisplayMode::ImGui;
            else
                mSettings.DisplayMode = FindAllDisplayMode::ChatLog;
        }
        else if (_stricmp(pNode->name(), "requirekeyitemprefix") == 0)
        {
            mSettings.RequireKeyItemPrefix = (_stricmp(pNode->value(), "enabled") == 0);
        }
        else if (_stricmp(pNode->name(), "writedelay") == 0)
        {
            mSettings.WritePeriod = static_cast<uint32_t>(atoll(pNode->value()));
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

    *pStream << "    <!--When set to enabled, you must type ki: prior to a search term to include key items in the search.-->\n";
    *pStream << "    <requirekeyitemprefix>" << (GetKeyItemPrefix() ? "enabled" : "disabled") << "</requirekeyitemprefix>\n\n";

    *pStream << "    <!--The delay, in milliseconds, after an item changes before it is written to cache.  This prevents extensive writes when inventory is frequently changing.-->\n";
    *pStream << "    <writedelay>" << std::to_string(GetWritePeriod()) << "</writedelay>\n\n";
    return true;
}