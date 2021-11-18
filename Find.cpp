#include "FindAll.h"

uint32_t FindAll::ThreadEntry()
{
    std::vector<SearchItem_t> items = GetMatchingItems(mPending.Term);
    if (items.size() == 0)
    {
        m_AshitaCore->GetChatManager()->Write(0, false, Output::Errorf("Could not find any item IDs matching the term: $H%s$R.", mPending.Term).c_str());
        InterlockedExchange(&mPending.State, (uint32_t)SearchState::Idle);
        return 0;
    }
    for (std::vector<QueriableCache*>::iterator iCache = mPending.Caches.begin(); iCache != mPending.Caches.end(); iCache = mPending.Caches.erase(iCache))
    {
        std::vector<SearchResult_t> temp = QueryCache(items, *iCache);
        mPending.Results.insert(mPending.Results.end(), temp.begin(), temp.end());
        delete *iCache;
    }

    if (mPending.Results.size() > 0)
    {
        mPending.pSearch = new SearchInstance(mPending.Results, mPending.Term, mConfig.GetDisplayMax());
    }
    else
    {
        mPending.pSearch = nullptr;
    }

    InterlockedExchange(&mPending.State, (uint32_t)SearchState::Complete);
    return 0;
}
bool FindAll::CheckWildcardMatch(const char* wc, const char* compare)
{
    while (*wc)
    {
        if (wc[0] == '*')
        {
            if (wc[1] == 0)
                return true;
            while (*compare)
            {
                if (CheckWildcardMatch((wc + 1), compare))
                {
                    return true;
                }
                compare++;
            }
            return false;
        }
        if ((wc[0] | 32) != (compare[0] | 32))
            return false;
        wc++;
        compare++;
    }
    return (*compare == 0);
}
SearchItem_t FindAll::CreateSearchItem(uint16_t id)
{
    SearchItem_t item;
    item.ItemId = id;
    item.SlipId = 0;
    item.SlipOffset = 0;
    for (std::vector<StorageSlip_t>::iterator iSlip = mSlips.begin(); iSlip != mSlips.end(); iSlip++)
    {
        for (int offset = 0; offset < 228; offset++)
        {
            if (iSlip->StoredItem[offset] == id)
            {
                item.SlipId = iSlip->ItemId;
                item.SlipOffset = offset;
                return item;
            }
        }
    }
    return item;
}
void FindAll::FindAcrossCharacters(const char* term)
{
    if (InterlockedCompareExchange(&mPending.State, (uint32_t)SearchState::InProgress, (uint32_t)SearchState::Idle) != (uint32_t)(uint32_t)SearchState::Idle)
    {
        m_AshitaCore->GetChatManager()->Write(0, false, Output::Errorf("Currently processing a search: $H%s$R.", mPending.Term).c_str());
        return;
    }
    strcpy_s(mPending.Term, 256, term);
    mPending.Results.clear();
    mPending.Caches.push_back(new QueriableCache(this->pCache));
    QueriableCache::LoadAll(m_AshitaCore, &mPending.Caches);

    this->Start();
}
std::vector<SearchItem_t> FindAll::GetMatchingItems(const char* term)
{
    std::vector<SearchItem_t> matchIds;

    //Match numerical ids..
    bool isNumber = true;
    for (int x = 0; x < strlen(term); x++)
    {
        if (!isdigit(x))
        {
            isNumber = false;
            break;
        }
    }
    if (isNumber)
    {
        int32_t id = atoi(term);
        if ((id >= 0) && (id <= 65535))
        {
            IItem* pResource = m_AshitaCore->GetResourceManager()->GetItemById(id);
            if ((pResource) && (pResource->Name[0]))
            {
                matchIds.push_back(CreateSearchItem(id));
                return matchIds;
            }
        }
    }

    //Match wildcard
    if (strstr(term, "*"))
    {
        for (int x = 0; x < 65536; x++)
        {
            IItem* pResource = m_AshitaCore->GetResourceManager()->GetItemById(x);
            if ((pResource) && (pResource->Name[0]) && (CheckWildcardMatch(term, pResource->Name[0])))
            {
                matchIds.push_back(CreateSearchItem(x));
            }
        }
    }

    //Match standard
    else
    {
        for (int x = 0; x < 65536; x++)
        {
            IItem* pResource = m_AshitaCore->GetResourceManager()->GetItemById(x);
            if ((pResource) && (pResource->Name[0]) && (_stricmp(term, pResource->Name[0]) == 0))
            {
                matchIds.push_back(CreateSearchItem(x));
            }
        }

        //revert to wildcard match if no exact match..
        if (matchIds.size() == 0)
        {
            char wcBuffer[256];
            sprintf_s(wcBuffer, 256, "*%s*", term);

            for (int x = 0; x < 65536; x++)
            {
                IItem* pResource = m_AshitaCore->GetResourceManager()->GetItemById(x);
                if ((pResource) && (pResource->Name[0]) && (CheckWildcardMatch(wcBuffer, pResource->Name[0])))
                {
                    matchIds.push_back(CreateSearchItem(x));
                }
            }
        }
    }
    return matchIds;
}
std::vector<SearchResult_t> FindAll::QueryCache(std::vector<SearchItem_t> itemIds, QueriableCache* pCache)
{
    std::vector<SearchResult_t> results;
    for (std::vector<SearchItem_t>::iterator iSearch = itemIds.begin(); iSearch != itemIds.end(); iSearch++)
    {
        SearchResult_t result;
        strcpy_s(result.Character.Name, 16, pCache->GetCharacterName());
        result.Character.Id = pCache->GetCharacterId();
        result.Id                   = iSearch->ItemId;
        result.Resource             = m_AshitaCore->GetResourceManager()->GetItemById(result.Id);
        result.StorageSlipContainer = -1;
        result.Total                = 0;
        for (int container = 0; container < 13; container++)
        {
            result.Count[container]      = 0;
            for (int index = 0; index < 81; index++)
            {
                Ashita::FFXI::item_t* pItem = pCache->GetContainerItem(container, index);
                if (pItem->Id == iSearch->ItemId)
                {
                    result.Count[container] += pItem->Count;
                    result.Total += pItem->Count;
                }
            }

            if (iSearch->SlipId != 0)
            {
                for (int index = 0; index < 81; index++)
                {
                    Ashita::FFXI::item_t* pItem = pCache->GetContainerItem(container, index);
                    if (pItem->Id == iSearch->SlipId)
                    {
                        if (Ashita::BinaryData::UnpackBitsBE(pItem->Extra, iSearch->SlipOffset, 1))
                        {
                            result.StorageSlipContainer = container;
                            result.Total++;
                        }
                    }
                }
            }
        }
        if (result.Total > 0)
        {
            results.push_back(result);
        }
    }

    return results;
}