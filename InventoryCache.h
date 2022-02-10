#ifndef __ASHITA_InventoryCache_H_INCLUDED__
#define __ASHITA_InventoryCache_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif 
#include "C:\Ashita 4\plugins\sdk\Ashita.h"
#include "FindAllConfig.h"

//0x1D
struct pk_FinishInventory
{
    uint32_t Header;
    uint32_t Flag;
};

//0x1E
struct pk_UpdateItem1E
{
    uint32_t Header;
    uint32_t Count;
    uint8_t Container;
    uint8_t Index;
    uint8_t Flags;
    uint8_t Padding;
};

//0x1F
struct pk_UpdateItem1F
{
    uint32_t Header;
    uint32_t Count;
    uint16_t ItemId;
    uint8_t Container;
    uint8_t Index;
    uint8_t Flags;
};

//0x20
struct pk_CreateItem
{
    uint32_t Header;
    uint32_t Count;
    uint32_t Bazaar;
    uint16_t Id;
    uint8_t Container;
    uint8_t Index;
    uint8_t Flags;
    uint8_t ExtData[24];
    uint8_t Padding[3];
};

struct CharacterIdentifier_t
{
    char Name[16];
    uint32_t Id;

    bool operator==(const CharacterIdentifier_t& other)
    {
        return ((strcmp(Name, other.Name) == 0) && (Id == other.Id));
    }
};

enum class SwapStatus
{
    InActive = 0,
    Pending = 1,
    ReadyToSwap = 2
};

class InventoryCache
{
private:
    IAshitaCore* pAshitaCore;
    FindAllConfig* pConfig;
    char mFilePath[256];
    CharacterIdentifier_t mCharacter;
    bool mSwappable;
    bool mWritesPending;
    std::chrono::steady_clock::time_point mWriteTime;
    Ashita::FFXI::items_t mContainers[CONTAINER_MAX];

    SwapStatus mSwapStatus;
    Ashita::FFXI::items_t mSwap[CONTAINER_MAX];

public:
    InventoryCache(IAshitaCore* pAshitaCore, FindAllConfig* pConfig);
    Ashita::FFXI::item_t GetContainerItem(int container, int index);
    void HandleIncomingPacket(uint16_t id, uint32_t size, const uint8_t* data);
    void HandleTick();

private:
    void ClearCache(Ashita::FFXI::items_t* cache);
    void FlushCacheToMemory();
    void CreateDirectories(const char* fileName);
    void QueueWrite();
    bool TryWriteFile();
};

class QueriableCache
{
private:
    CharacterIdentifier_t mCharacter;
    Ashita::FFXI::items_t mContainers[CONTAINER_MAX];
    bool mLoaded;
    
public:
    QueriableCache(InventoryCache* pCache);
    QueriableCache(const char* fileName);

    const char* GetCharacterName();
    uint32_t GetCharacterId();
    Ashita::FFXI::item_t* GetContainerItem(int container, int index);
    bool IsLoaded();
    bool TryLoadFile(const char* fileName);
    static void LoadAll(IAshitaCore* pAshitaCore, std::vector<QueriableCache*>* pCaches);
};

#endif