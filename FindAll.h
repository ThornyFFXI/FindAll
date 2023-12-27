#ifndef __ASHITA_FindAll_H_INCLUDED__
#define __ASHITA_FindAll_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "C:\Ashita 4\plugins\sdk\Ashita.h"
#include "FindAllConfig.h"
#include "InventoryCache.h"
#include "SearchInstance.h"

enum class SearchState : uint32_t
{
    Idle = 0,
    InProgress = 1,
    Complete = 2
};

struct PendingSearch_t
{
    std::vector<QueriableCache*> Caches;
    std::vector<SearchResult_t> Results;
    SearchInstance* pSearch;
    std::vector<std::string> Terms;
    volatile uint32_t State;
};

class FindAll : IPlugin, Ashita::Threading::Thread
{
private:
    IAshitaCore* m_AshitaCore;
    ILogManager* m_LogManager;
    IDirect3DDevice8* m_Direct3DDevice;
    uint32_t m_PluginId;

    FindAllConfig mConfig;
    std::list<SearchInstance*> mSearches;
    std::vector<StorageSlip_t> mSlips;

    InventoryCache* pCache;
    PendingSearch_t mPending;


public:
    const char* GetName(void) const override
    {
        return "FindAll";
    }
    const char* GetAuthor(void) const override
    {
        return "Thorny";
    }
    const char* GetDescription(void) const override
    {
        return "Insert description here.";
    }
    const char* GetLink(void) const override
    {
        return "Insert link here.";
    }
    double GetVersion(void) const override
    {
        return 1.16f;
    }
    int32_t GetPriority(void) const override
    {
        return 0;
    }
    uint32_t GetFlags(void) const override
    {
        return (uint32_t)Ashita::PluginFlags::LegacyDirect3D;
    }
	
    bool Initialize(IAshitaCore* core, ILogManager* logger, const uint32_t id) override;
    void Release(void) override;
    bool HandleCommand(int32_t mode, const char* command, bool injected) override;
    bool HandleIncomingPacket(uint16_t id, uint32_t size, const uint8_t* data, uint8_t* modified, uint32_t sizeChunk, const uint8_t* dataChunk, bool injected, bool blocked) override;
    bool Direct3DInitialize(IDirect3DDevice8* device) override;	
    void Direct3DPresent(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion) override;
    
private:
    //Commands.cpp
    void HandleCommandInternal(std::vector<std::string> args, int argCount);

    //Find.cpp
    uint32_t ThreadEntry(void) override;
    bool CheckWildcardMatch(const char* wc, const char* compare);
    SearchItem_t CreateSearchItem(uint16_t id);
    void FindAcrossCharacters(std::vector<std::string> terms);
    void FindLocal(std::vector<std::string> terms);
    std::vector<SearchItem_t> GetMatchingKeyItems(const char* term);
    std::vector<SearchItem_t> GetMatchingItems(const char* term);
    std::vector<SearchResult_t> QueryCache(std::vector<SearchItem_t> itemIds, QueriableCache* pCache);

    //Slips.cpp
    void LoadSlipData();

    //Display.cpp
    void DisplayResults(SearchInstance* pResult);
    void DisplayChatSingleCharacter(SearchInstance* pResult);
    void DisplayChatMultipleCharacters(SearchInstance* pResult);
    void DisplayChatMultipleCharactersItems(SearchInstance* pResult);
    void DisplayFontObject(SearchInstance* pResult);

    //ImguiDisplay.cpp
    void ImguiDisplay();
};
#endif