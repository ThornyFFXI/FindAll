#include "FindAll.h"
#include <format>
#include <map>
#include <sstream>
#include <string>
#include <string_view>

#define CONTAINER_MAX 17
const char* ContainerNames[CONTAINER_MAX] =
    {
        "inventory",
        "safe",
        "storage",
        "temporary",
        "locker",
        "satchel",
        "sack",
        "case",
        "wardrobe",
        "safe2",
        "wardrobe2",
        "wardrobe3",
        "wardrobe4",
        "wardrobe5",
        "wardrobe6",
        "wardrobe7",
        "wardrobe8"};

#define COLOR_MAX 6
std::string ColorCodes[COLOR_MAX] = {
    "|cFFFF8080|",
    "|cFF80FF80|",
    "|cFF8080FF|",
    "|cFFFF4080|",
    "|cFF80FF40|",
    "|cFF4080FF|"};

void FindAll::DisplayResults(SearchInstance* pResult)
{
    if (mConfig.GetDisplayMode() == FindAllDisplayMode::ImGui)
    {
        DisplayFontObject(pResult);
    }
    else if (pResult->GetCharacterCount() == 1)
    {
        DisplayChatSingleCharacter(pResult);
    }
    else if (pResult->GetItemCount() == 1)
    {
        DisplayChatMultipleCharacters(pResult);
    }
    else
    {
        DisplayChatMultipleCharactersItems(pResult);
    }
}

void FindAll::DisplayChatSingleCharacter(SearchInstance* pResult)
{
    std::vector<SearchResult_t> results = pResult->GetResultVector();
    if (results.size() == 1)
    {
        SearchResult_t result = results[0];
        IItem* pItem          = m_AshitaCore->GetResourceManager()->GetItemById(result.Id);
        int printCount        = 0;
        for (int x = 0; x < CONTAINER_MAX; x++)
        {
            int count = result.Count[x];
            if (count == 1)
            {
                OutputHelper::Outputf(Ashita::LogLevel::Info, "Found $Ha %s$R in $H%s's %s$R.", pItem->LogNameSingular[0], result.Character.Name, ContainerNames[x]);
                printCount++;
            }
            else if (count > 1)
            {
                OutputHelper::Outputf(Ashita::LogLevel::Info, "Found $H%d %s$R in $H%s's %s$R.", count, pItem->LogNamePlural[0], result.Character.Name, ContainerNames[x]);
                printCount++;
            }
        }
        if (result.StorageSlipContainer != -1)
        {
            OutputHelper::Outputf(Ashita::LogLevel::Info, "Found $Ha %s$R on a storage slip in $H%s's %s$R.", pItem->LogNameSingular[0], result.Character.Name, ContainerNames[result.StorageSlipContainer]);
            printCount++;
        }
        if (printCount > 1)
        {
            OutputHelper::Outputf(Ashita::LogLevel::Info, "Found a total of $H%u %s$R on $H%s$R.", result.Total, pItem->LogNamePlural[0], result.Character.Name);
        }
    }
    else
    {
        for (int x = 0; x < results.size(); x++)
        {
            SearchResult_t result = results[x];
            IItem* pItem          = m_AshitaCore->GetResourceManager()->GetItemById(result.Id);
            if (result.StorageSlipContainer != -1)
            {
                if (result.Total == 1)
                {
                    OutputHelper::Outputf(Ashita::LogLevel::Info, "Found $Ha %s$R on a storage slip in $H%s's %s$R.", pItem->LogNameSingular[0], result.Character.Name, ContainerNames[result.StorageSlipContainer]);
                }
                else
                {
                    OutputHelper::Outputf(Ashita::LogLevel::Info, "Found a total of $H%u %s$R on $H%s$R, including one on a storage slip in $H%s$R.", result.Total, pItem->LogNamePlural[0], result.Character.Name, ContainerNames[result.StorageSlipContainer]);
                }
            }
            else
            {
                if (result.Total == 1)
                {
                    int container = -1;
                    for (int y = 0; y < CONTAINER_MAX; y++)
                    {
                        if (result.Count[y] > 0)
                        {
                            container = y;
                            break;
                        }
                    }
                    if (container != -1)
                    {
                        OutputHelper::Outputf(Ashita::LogLevel::Info, "Found $Ha %s$R in $H%s's %s$R.", pItem->LogNameSingular[0], result.Character.Name, ContainerNames[container]);
                    }
                }
                else
                {
                    int container = -1;
                    for (int y = 0; y < CONTAINER_MAX; y++)
                    {
                        if (result.Count[y] > 0)
                        {
                            if (container == -1)
                            {
                                container = y;
                            }
                            else
                            {
                                container = -1;
                                break;
                            }
                        }
                    }

                    if (container == -1)
                    {
                        OutputHelper::Outputf(Ashita::LogLevel::Info, "Found a total of $H%u %s$R on $H%s$R.", result.Total, pItem->LogNamePlural[0], result.Character.Name);
                    }
                    else
                    {
                        OutputHelper::Outputf(Ashita::LogLevel::Info, "Found $H%u %s$R in $H%s's %s$R.", result.Total, pItem->LogNamePlural[0], result.Character.Name, ContainerNames[container]);
                        OutputHelper::Outputf(Ashita::LogLevel::Info, "Found $Ha %s$R in $H%s's %s$R.", pItem->LogNameSingular[0], result.Character.Name, ContainerNames[container]);
                    }
                }
            }
        }
        OutputHelper::Outputf(Ashita::LogLevel::Info, "Found a total of $H%d matching items$R on $H%s$R.", pResult->GetItemTotal(), results[0].Character.Name);
    }
    delete pResult;
}
void FindAll::DisplayChatMultipleCharacters(SearchInstance* pResult)
{
    std::vector<SearchResult_t> results = pResult->GetResultVector();
    std::sort(results.begin(), results.end(), [](const SearchResult_t& lhs, const SearchResult_t& rhs) {
        int cmp = strcmp(lhs.Resource->Name[0], rhs.Resource->Name[0]);
        if (cmp != 0)
            return (cmp < 0);
        if (lhs.Id != rhs.Id)
            return (lhs.Id < rhs.Id);
        cmp = strcmp(lhs.Character.Name, rhs.Character.Name);
        if (cmp != 0)
            return (cmp < 0);
        return (lhs.Character.Id < rhs.Character.Id);
    });

    IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemById(results[0].Id);
    for (int x = 0; x < results.size(); x++)
    {
        SearchResult_t result = results[x];
        if (result.StorageSlipContainer != -1)
        {
            if (result.Total == 1)
            {
                OutputHelper::Outputf(Ashita::LogLevel::Info, "Found $Ha %s$R on a storage slip in $H%s's %s$R.", pItem->LogNameSingular[0], result.Character.Name, ContainerNames[result.StorageSlipContainer]);
            }
            else
            {
                OutputHelper::Outputf(Ashita::LogLevel::Info, "Found a total of $H%u %s$R on $H%s$R, including one on a storage slip in $H%s$R.", result.Total, pItem->LogNamePlural[0], result.Character.Name, ContainerNames[result.StorageSlipContainer]);
            }
        }
        else
        {
            if (result.Total == 1)
            {
                int container = -1;
                for (int y = 0; y < CONTAINER_MAX; y++)
                {
                    if (result.Count[y] > 0)
                    {
                        container = y;
                        break;
                    }
                }
                if (container != -1)
                {
                    OutputHelper::Outputf(Ashita::LogLevel::Info, "Found $Ha %s$R in $H%s's %s$R.", pItem->LogNameSingular[0], result.Character.Name, ContainerNames[container]);
                }
            }
            else
            {
                int container = -1;
                for (int y = 0; y < CONTAINER_MAX; y++)
                {
                    if (result.Count[y] > 0)
                    {
                        if (container == -1)
                        {
                            container = y;
                        }
                        else
                        {
                            container = -1;
                            break;
                        }
                    }
                }

                if (container == -1)
                {
                    OutputHelper::Outputf(Ashita::LogLevel::Info, "Found a total of $H%u %s$R on $H%s$R.", result.Total, pItem->LogNamePlural[0], result.Character.Name);
                }
                else
                {
                    OutputHelper::Outputf(Ashita::LogLevel::Info, "Found $H%u %s$R in $H%s's %s$R.", result.Total, pItem->LogNamePlural[0], result.Character.Name, ContainerNames[container]);
                    OutputHelper::Outputf(Ashita::LogLevel::Info, "Found $Ha %s$R in $H%s's %s$R.", pItem->LogNameSingular[0], result.Character.Name, ContainerNames[container]);
                }
            }
        }
    }
    if (pResult->GetCharacterCount() > 1)
    {
        OutputHelper::Outputf(Ashita::LogLevel::Info, "Found a total of $H%u %s$R on %d characters.", pResult->GetItemTotal(), pItem->LogNamePlural[0], pResult->GetCharacterCount());
    }
    delete pResult;
}
void FindAll::DisplayChatMultipleCharactersItems(SearchInstance* pResult)
{
    std::vector<SearchResult_t> results = pResult->GetResultVector();
    std::sort(results.begin(), results.end(), [](const SearchResult_t &lhs, const SearchResult_t &rhs) {
        int cmp = strcmp(lhs.Resource->Name[0], rhs.Resource->Name[0]);
        if (cmp != 0)
            return (cmp < 0);
        if (lhs.Id != rhs.Id)
            return (lhs.Id < rhs.Id);
        cmp = strcmp(lhs.Character.Name, rhs.Character.Name);
        if (cmp != 0)
            return (cmp < 0);
        return (lhs.Character.Id < rhs.Character.Id);
    });
    std::map<uint16_t, int> itemTotals;

    for (int x = 0; x < results.size(); x++)
    {
        SearchResult_t result = results[x];
        IItem* pItem          = result.Resource;
        if (result.StorageSlipContainer != -1)
        {
            if (result.Total == 1)
            {
                OutputHelper::Outputf(Ashita::LogLevel::Info, "Found $Ha %s$R on a storage slip in $H%s's %s$R.", pItem->LogNameSingular[0], result.Character.Name, ContainerNames[result.StorageSlipContainer]);
            }
            else
            {
                OutputHelper::Outputf(Ashita::LogLevel::Info, "Found a total of $H%u %s$R on $H%s$R, including one on a storage slip in $H%s$R.", result.Total, pItem->LogNamePlural[0], result.Character.Name, ContainerNames[result.StorageSlipContainer]);
            }
        }
        else
        {
            if (result.Total == 1)
            {
                int container = -1;
                for (int y = 0; y < CONTAINER_MAX; y++)
                {
                    if (result.Count[y] > 0)
                    {
                        container = y;
                        break;
                    }
                }
                if (container != -1)
                {
                    OutputHelper::Outputf(Ashita::LogLevel::Info, "Found $Ha %s$R in $H%s's %s$R.", pItem->LogNameSingular[0], result.Character.Name, ContainerNames[container]);
                }
            }
            else
            {
                int container = -1;
                for (int y = 0; y < CONTAINER_MAX; y++)
                {
                    if (result.Count[y] > 0)
                    {
                        if (container == -1)
                        {
                            container = y;
                        }
                        else
                        {
                            container = -1;
                            break;
                        }
                    }
                }

                if (container == -1)
                {
                    OutputHelper::Outputf(Ashita::LogLevel::Info, "Found a total of $H%u %s$R on $H%s$R.", result.Total, pItem->LogNamePlural[0], result.Character.Name);
                }
                else
                {
                    OutputHelper::Outputf(Ashita::LogLevel::Info, "Found $H%u %s$R in $H%s's %s$R.", result.Total, pItem->LogNamePlural[0], result.Character.Name, ContainerNames[container]);
                    OutputHelper::Outputf(Ashita::LogLevel::Info, "Found $Ha %s$R in $H%s's %s$R.", pItem->LogNameSingular[0], result.Character.Name, ContainerNames[container]);
                }
            }
        }
        auto iTotal = itemTotals.find(result.Id);
        if (iTotal == itemTotals.end())
            itemTotals[result.Id] = result.Total;
        else
            iTotal->second += result.Total;
    }

    //Display items that are on multiple characters..?
    delete pResult;
}
void FindAll::DisplayFontObject(SearchInstance* pResult)
{
    for (std::list<SearchInstance*>::iterator searchIter = mSearches.begin(); searchIter != mSearches.end(); searchIter++)
    {
        if (_stricmp(pResult->GetQuery(), (*searchIter)->GetQuery()) == 0)
        {
            delete *searchIter;
            *searchIter = pResult;
            return;
        }
    }
    mSearches.push_back(pResult);
}