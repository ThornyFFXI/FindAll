#include "InventoryCache.h"
#include <fstream>

InventoryCache::InventoryCache(IAshitaCore* pAshitaCore, FindAllConfig* pConfig)
    : pAshitaCore(pAshitaCore)
    , pConfig(pConfig)
{
    ClearCache(mContainers);
    uint16_t myIndex = pAshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0);
    if (myIndex > 0)
    {
        strcpy_s(mCharacter.Name, 16, pAshitaCore->GetMemoryManager()->GetEntity()->GetName(myIndex));
    }
    else
    {
        strcpy_s(mCharacter.Name, 16, "Unknown");
    }
    mCharacter.Id = 0;
    mSwappable = false;
    mSwapStatus   = SwapStatus::InActive;
    mWritesPending = true;
}
Ashita::FFXI::item_t InventoryCache::GetContainerItem(int container, int index)
{
    //If we have a full backup and it's not a temp item, pull directly from backup.
    if ((container != (int)Ashita::FFXI::Enums::Container::Temporary) && (mSwappable))
    {
        return mContainers[container].Items[index];
    }

    //Otherwise, use client but don't allow failure.
    else
    {
        Ashita::FFXI::item_t* pItem = pAshitaCore->GetMemoryManager()->GetInventory()->GetContainerItem(container, index);
        if (pItem)
        {
            return *pItem;
        }
        else
        {
            Ashita::FFXI::item_t item = {0};
            item.Index                = index;
            return item;
        }
    }
}
void InventoryCache::HandleIncomingPacket(uint16_t id, uint32_t size, const uint8_t* data)
{
    if (id == 0x00A)
    {
        if ((strcmp(mCharacter.Name, (const char*)data + 0x84) != 0) || (*((uint32_t*)(data + 4)) != mCharacter.Id))
        {
            //Final write even if waiting on timer when we zone into a new character.
            if ((pConfig->GetCacheToDisc()) && (mSwappable) && (mWritesPending))
            {
                if (TryWriteFile())
                {
                    mWritesPending = false;
                }
                else
                {
                    OutputHelper::Outputf(Ashita::LogLevel::Error, "Failed to write file: $H%s$R", mFilePath);
                }
            }

            //Update character name and blank everything
            memcpy(mCharacter.Name, data + 0x84, 16);
            mCharacter.Id = *((uint32_t*)(data + 4));
            ClearCache(mContainers);
            mSwappable    = false;
            mWritesPending = true;
            sprintf_s(mFilePath, 256, "%sconfig\\plugins\\findall\\cache\\%s_%u.bin", pAshitaCore->GetInstallPath(), mCharacter.Name, mCharacter.Id);
        }
        else if (mSwappable)
        {
            if (pConfig->GetInstantLoad())
            {
                FlushCacheToMemory();
                pk_FinishInventory packet = {0};
                packet.Flag               = 1;
                pAshitaCore->GetPacketManager()->AddIncomingPacket(0x1D, sizeof(packet), (uint8_t*)&packet);
            }
            mSwapStatus = SwapStatus::Pending;
        }
        ClearCache(mSwap);
    }
    if ((id == 0x01D) && (*(data + 4) == 1))
    {
        if (mCharacter.Id != 0)
        {
            mSwappable = true;
            if (mSwapStatus == SwapStatus::Pending)
            {
                mSwapStatus = SwapStatus::ReadyToSwap;
            }
        }
    }
    if (id == 0x01E)
    {
        pk_UpdateItem1E* pPacket    = (pk_UpdateItem1E*)data;
        Ashita::FFXI::item_t* pItem = &(mContainers[pPacket->Container].Items[pPacket->Index]);
        if ((pPacket->Count != pItem->Count) || (pPacket->Flags != pItem->Flags))
        {
            QueueWrite();
        }
        if (pPacket->Count == 0)
        {
            *pItem = {0};
            pItem->Index = pPacket->Index;
            if (mSwapStatus == SwapStatus::Pending)
            {
                mSwap[pPacket->Container].Items[pPacket->Index]       = {0};
                mSwap[pPacket->Container].Items[pPacket->Index].Index = pPacket->Index;
            }
        }
        else
        {
            pItem->Count = pPacket->Count;
            pItem->Flags = pPacket->Flags;
            if (mSwapStatus == SwapStatus::Pending)
            {
                mSwap[pPacket->Container].Items[pPacket->Index].Count = pItem->Count;
                mSwap[pPacket->Container].Items[pPacket->Index].Flags = pItem->Flags;
            }
        }
    }
    if (id == 0x01F)
    {
        pk_UpdateItem1F* pPacket    = (pk_UpdateItem1F*)data;
        Ashita::FFXI::item_t* pItem = &(mContainers[pPacket->Container].Items[pPacket->Index]);
        if ((pPacket->Count != pItem->Count) || (pPacket->Flags != pItem->Flags) || (pPacket->ItemId != pItem->Id))
        {
            QueueWrite();
        }
        if (pPacket->ItemId != pItem->Id)
        {
            memset(pItem->Extra, 28, 0);
            pItem->Price = {0};
        }
        pItem->Count = pPacket->Count;
        pItem->Flags = pPacket->Flags;
        pItem->Id    = pPacket->ItemId;
        if (mSwapStatus == SwapStatus::Pending)
        {
            Ashita::FFXI::item_t* pSwap = &(mSwap[pPacket->Container].Items[pPacket->Index]);
            if (pSwap->Id != pPacket->ItemId)
            {
                memset(pSwap->Extra, 28, 0);
                pSwap->Price = {0};
            }
            pSwap->Count = pItem->Count;
            pSwap->Flags = pItem->Flags;
            pSwap->Id   = pItem->Id;
        }
    }
    if (id == 0x020)
    {
        pk_CreateItem* pPacket = (pk_CreateItem*)data;
        Ashita::FFXI::item_t item = {0};
        item.Index             = pPacket->Index;
        item.Count                = pPacket->Count;
        memcpy(item.Extra, pPacket->ExtData, 24);
        item.Flags = pPacket->Flags;
        item.Id    = pPacket->Id;
        item.Price                  = pPacket->Bazaar;
        Ashita::FFXI::item_t* pItem = &(mContainers[pPacket->Container].Items[pPacket->Index]);

        if (memcmp(&item, pItem, sizeof(Ashita::FFXI::item_t)))
        {
            *pItem = item;
            QueueWrite();
        }
        if (mSwapStatus == SwapStatus::Pending)
        {
            Ashita::FFXI::item_t* pSwap = &(mSwap[pPacket->Container].Items[pPacket->Index]);
            *pSwap             = item;
        }
    }
}
void InventoryCache::HandleTick()
{
    if (mSwapStatus == SwapStatus::ReadyToSwap)
    {
        memcpy(mContainers, mSwap, sizeof(mContainers));
        if (pConfig->GetInstantLoad())
        {
            FlushCacheToMemory();
        }
        mSwapStatus   = SwapStatus::InActive;
        QueueWrite();
    }
    if (pConfig->GetCacheToDisc())
    {
        if ((mSwapStatus == SwapStatus::InActive) && (mSwappable) && (mWritesPending) && (std::chrono::steady_clock::now() >= mWriteTime))
        {
            if (TryWriteFile())
            {
                mWritesPending = false;
            }
            else
            {
                OutputHelper::Outputf(Ashita::LogLevel::Error, "Failed to write file: $H%s$R", mFilePath);
            }
        }
    }
}

void InventoryCache::ClearCache(Ashita::FFXI::items_t* cache)
{
    for (int container = 0; container < CONTAINER_MAX; container++)
    {
        for (int index = 0; index < 81; index++)
        {
            cache[container].Items[index] = {0};
            cache[container].Items[index].Index = index;
        }
    }
}
void InventoryCache::FlushCacheToMemory()
{
    Ashita::FFXI::items_t* pInventory = pAshitaCore->GetMemoryManager()->GetInventory()->GetRawStructure()->Containers;
    for (int x = 0; x < CONTAINER_MAX; x++)
    {
        if (x != (int)Ashita::FFXI::Enums::Container::Temporary)
        {
            memcpy(pInventory, &(mContainers[x]), sizeof(Ashita::FFXI::items_t));
        }
        pInventory++;
    }
}
void InventoryCache::CreateDirectories(const char* fileName)
{
    //Ensure directories exist, making them if not.
    std::string makeDirectory(fileName);
    size_t nextDirectory = makeDirectory.find("\\");
    nextDirectory        = makeDirectory.find("\\", nextDirectory + 1);
    while (nextDirectory != std::string::npos)
    {
        std::string currentDirectory = makeDirectory.substr(0, nextDirectory + 1);
        if ((!CreateDirectory(currentDirectory.c_str(), NULL)) && (ERROR_ALREADY_EXISTS != GetLastError()))
        {
            OutputHelper::Outputf(Ashita::LogLevel::Error, "Failed to create directory: $H%s$R", currentDirectory.c_str());
            return;
        }
        nextDirectory = makeDirectory.find("\\", nextDirectory + 1);
    }
}
void InventoryCache::QueueWrite()
{
    mWritesPending = true;
    mWriteTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(pConfig->GetWritePeriod());
}
bool InventoryCache::TryWriteFile()
{
    CreateDirectories(mFilePath);
    std::ofstream writer(mFilePath, std::ios::out | std::ios::binary);
    if (!writer.is_open())
        return false;
    try
    {
        writer.write(mCharacter.Name, 16);
        writer.write((char*)&mCharacter.Id, 4);
        for (int x = 0; x < CONTAINER_MAX; x++)
        {
            if (x == (int)Ashita::FFXI::Enums::Container::Temporary)
                continue;

            for (int y = 0; y < 81; y++)
            {
                writer.write((char*)&mContainers[x].Items[y], sizeof(Ashita::FFXI::item_t));
            }
        }
    }
    catch (...)
    {
        writer.close();
        return false;
    }
    writer.close();
    return true;
}

QueriableCache::QueriableCache(InventoryCache* pCache)
{
    strcpy_s(mCharacter.Name, 16, ConfigLoader::pLoader->GetCharacterName());
    mCharacter.Id = ConfigLoader::pLoader->GetCharacterId();
    for (int container = 0; container < CONTAINER_MAX; container++)
    {
        for (int index = 0; index < 81; index++)
        {
            mContainers[container].Items[index] = pCache->GetContainerItem(container, index);
        }
    }
    mLoaded = true;
}
QueriableCache::QueriableCache(const char* fileName)
{
    if (TryLoadFile(fileName))
    {
        for (int index = 0; index < 81; index++)
        {
            mContainers[(int)Ashita::FFXI::Enums::Container::Temporary].Items[index]       = {0};
            mContainers[(int)Ashita::FFXI::Enums::Container::Temporary].Items[index].Index = index;
        }
        mLoaded = true;
    }
    else
    {
        mLoaded = false;
    }       
}
const char* QueriableCache::GetCharacterName()
{
    return mCharacter.Name;
}
uint32_t QueriableCache::GetCharacterId()
{
    return mCharacter.Id;
}
Ashita::FFXI::item_t* QueriableCache::GetContainerItem(int container, int index)
{
    return &mContainers[container].Items[index];
}
bool QueriableCache::IsLoaded()
{
    return mLoaded;
}
void QueriableCache::LoadAll(IAshitaCore* pAshitaCore, std::vector<QueriableCache*>* pCaches)
{
    std::vector<std::string> cachePaths;
    char findString[256];
    sprintf_s(findString, 256, "%sconfig\\plugins\\findall\\cache\\*.bin", pAshitaCore->GetInstallPath());
    WIN32_FIND_DATA data;
    HANDLE hFind;
    if ((hFind = FindFirstFile(findString, &data)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            char buffer[256];
            sprintf_s(buffer, 256, "%sconfig\\plugins\\findall\\cache\\%s", pAshitaCore->GetInstallPath(), data.cFileName);
            cachePaths.push_back(std::string(buffer));
        } while (FindNextFile(hFind, &data) != 0);
        FindClose(hFind);
    }

    char compSelf[256];
    char compBlank[256];
    sprintf_s(compSelf, 256, "%sconfig\\plugins\\findall\\cache\\%s_%u.bin", pAshitaCore->GetInstallPath(), ConfigLoader::pLoader->GetCharacterName(), ConfigLoader::pLoader->GetCharacterId());
    sprintf_s(compBlank, 256, "%sconfig\\plugins\\findall\\cache\\%s_%u.bin", pAshitaCore->GetInstallPath(), ConfigLoader::pLoader->GetCharacterName(), 0);
    for (std::vector<std::string>::iterator iter = cachePaths.begin(); iter != cachePaths.end(); iter++)
    {
        //Skip our own cache because we already have it..
        if ((_stricmp(iter->c_str(), compSelf) == 0) || (_stricmp(iter->c_str(), compBlank) == 0))
            continue;

        QueriableCache* pCache = new QueriableCache(iter->c_str());
        if (!pCache->IsLoaded())
        {
            OutputHelper::Outputf(Ashita::LogLevel::Error, "Failed to load file: $H%s", iter->c_str());
            delete pCache;
        }
        else
        {
            pCaches->push_back(pCache);
        }
    }
}
bool QueriableCache::TryLoadFile(const char* fileName)
{
    int container = 0;
    int index     = 0;
    std::ifstream reader(fileName, std::ios::in | std::ios::binary);
    if (!reader.is_open())
        return false;
    Ashita::FFXI::item_t blank = {0};
    try
    {
        reader.read(mCharacter.Name, 16);
        reader.read((char*)&mCharacter.Id, 4);
        for (container = 0; container < CONTAINER_MAX; container++)
        {
            if (container == (int)Ashita::FFXI::Enums::Container::Temporary)
                continue;

            for (index = 0; index < 81; index++)
            {
                if (reader.peek() == EOF)
                {
                    blank.Index = index;
                    memcpy((char*)&mContainers[container].Items[index], &blank, sizeof(Ashita::FFXI::item_t));
                }
                else
                {
                    reader.read((char*)&mContainers[container].Items[index], sizeof(Ashita::FFXI::item_t));
                    if (mContainers[container].Items[index].Index != index)
                    {
                        reader.close();
                        return false;
                    }
                }
            }
        }
    }
    catch (...)
    {
        reader.close();
        return false;
    }
    reader.close();
    return true;
}