#ifndef __ASHITA_FindAllConfig_H_INCLUDED__
#define __ASHITA_FindAllConfig_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif
#define CONTAINER_MAX 17
#include "ConfigLoader.h"

enum class FindAllDisplayMode
{
    ChatLog =  0,
    ImGui = 1
};

struct FindAllSettings_t
{
    bool CacheToDisc;
    int32_t DisplayMax;
    FindAllDisplayMode DisplayMode;
    bool InstantLoad;
    uint32_t WritePeriod;

    FindAllSettings_t()
        : CacheToDisc(true)
        , DisplayMax(8)
        , DisplayMode(FindAllDisplayMode::ImGui)
        , InstantLoad(false)
        , WritePeriod(20000)
    {}
};

class FindAllConfig : public ConfigBase
{
private:
    FindAllSettings_t mSettings;

public:
    FindAllConfig();

    bool GetCacheToDisc();
    int32_t GetDisplayMax();
    FindAllDisplayMode GetDisplayMode();
    bool GetInstantLoad();
    uint32_t GetWritePeriod();

    void SetCacheToDisc(bool value);
    void SetDisplayMax(int32_t value);
    void SetDisplayMode(FindAllDisplayMode mode);
    void SetInstantLoad(bool value);
    void SetWritePeriod(uint32_t milliseconds);
    
    ConfigSaveStyle GetDefaultSaveStyle() override;
    bool LoadConfig(xml_node<>* pRoot, const char* errorBuffer) override;
    void Reset() override;
    bool SaveConfig(std::ofstream* pStream, const char* errorBuffer) override;
};

#endif