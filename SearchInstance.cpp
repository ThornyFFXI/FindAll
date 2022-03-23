#include "SearchInstance.h"
#define CONTAINER_MAX 17
const char* gContainerNames[CONTAINER_MAX] =
    {
        "Inventory",
        "Safe",
        "Storage",
        "Temporary",
        "Locker",
        "Satchel",
        "Sack",
        "Case",
        "Wardrobe",
        "Safe2",
        "Wardrobe2",
        "Wardrobe3",
        "Wardrobe4",
        "Wardrobe5",
        "Wardrobe6",
        "Wardrobe7",
        "Wardrobe8"};

SearchInstance::SearchInstance(std::vector<SearchResult_t> results, std::vector<std::string> searchTerms, int maxResults)
    : mCharacterCount(0)
    , mContainerCount(0)
    , mDuplicateItemName(false)
    , mDuplicatePlayerName(false)
    , mItemTotal(0)
    , mItemCount(0)
    , mResultVector(results)
    , mSearchTerms(searchTerms)
    , mSearchMode(SearchDisplayMode::Unsorted)
    , pSearchTable(nullptr)
{
    //used to trim the most common items
    std::vector<ItemStub_t> itemStubs;
    //used to identify duplicate character names
    std::vector<std::string> duplicateCharacters;

    //Preprocessing to fill prior vectors
    for (auto resultIter = results.begin(); resultIter != results.end(); resultIter++)
    {
        IItem* pResource = resultIter->Resource;
        ItemStub_t stub(resultIter->Resource, resultIter->Total);
        auto pStub       = std::find(itemStubs.begin(), itemStubs.end(), stub);
        if (pStub == itemStubs.end())
        {
            itemStubs.push_back(stub);
        }
        else
        {
            pStub->Count += resultIter->Total;
        }

        if (std::find(mCharacters.begin(), mCharacters.end(), resultIter->Character) == mCharacters.end())
        {
            mCharacters.push_back(resultIter->Character);
            mCharacterCount++;
            std::string name(resultIter->Character.Name);
            if (std::find(duplicateCharacters.begin(), duplicateCharacters.end(), name) == duplicateCharacters.end())
            {
                duplicateCharacters.push_back(name);
            }
            else
            {
                mDuplicatePlayerName = true;
            }
        }
        for (int x = 0; x < CONTAINER_MAX; x++)
        {
            if ((resultIter->Count[x] > 0) || (resultIter->StorageSlipContainer == x))
            {
                if (std::find(mLocationIndices.begin(), mLocationIndices.end(), x) == mLocationIndices.end())
                {
                    mLocationIndices.push_back(x);
                    mContainerCount++;
                }
            }
        }
        mItemTotal += resultIter->Total;
    }

    //Trim items if too many..
    if (itemStubs.size() > maxResults)
    {
        std::sort(itemStubs.begin(), itemStubs.end(), [](ItemStub_t const& lhs, ItemStub_t const& rhs) {
            return (lhs.Count > rhs.Count);
        });
        while (itemStubs.size() > maxResults)
            itemStubs.pop_back();
    }

    //Copy remaining items to resource table to be sorted, determine if we have any duplicate item names
    std::vector<std::string> duplicateItems;
    for (auto iStub = itemStubs.begin(); iStub != itemStubs.end(); iStub++)
    {
        std::string name(iStub->pResource->Name[0]);
        if (std::find(duplicateItems.begin(), duplicateItems.end(), name) == duplicateItems.end())
        {
            duplicateItems.push_back(name);
        }
        else
        {
            mDuplicateItemName = true;
        }

        mItemResources.push_back(iStub->pResource);
        mItemCount++;
    }
   
    //Sort characters, items, locations for easier display.
    std::sort(mCharacters.begin(), mCharacters.end(), [](CharacterIdentifier_t const& lhs, CharacterIdentifier_t const& rhs) {
        int cmp = strcmp(lhs.Name, rhs.Name);
        if (cmp != 0)
            return (cmp < 0);
        return (lhs.Id < rhs.Id);
    });
    std::sort(mItemResources.begin(), mItemResources.end(), [](IItem* const& lhs, IItem* const& rhs) {
        int cmp = strcmp(lhs->Name[0], rhs->Name[0]);
        if (cmp != 0)
            return (cmp < 0);
        return (lhs->Id < rhs->Id);
    });
    std::sort(mLocationIndices.begin(), mLocationIndices.end());

    if (mCharacterCount == 1)
    {
        SetSearchMode(SearchDisplayMode::LocationItem);
    }
    else if (mItemCount == 1)
    {
        SetSearchMode(SearchDisplayMode::LocationCharacter);
    }
    else
    {
        SetSearchMode(SearchDisplayMode::CharacterItem);
    }
}
SearchInstance::~SearchInstance()
{
    if (pSearchTable)
    {
        delete pSearchTable;
    }
}

void SearchInstance::SetSearchMode(SearchDisplayMode mode)
{
    if (mSearchMode == mode)
        return;

    mSearchMode = mode;
    BuildSearchTable();
}
void SearchInstance::BuildSearchTable()
{
    if (pSearchTable)
    {
        delete pSearchTable;
    }

    if (mSearchMode == SearchDisplayMode::LocationItem)
    {
        pSearchTable = new SearchTable(mContainerCount + 1, mItemCount + 2);

        char buffer[256];
        if (mCharacters.size() == 1)
        {
            sprintf_s(buffer, 256, "%s", mCharacters.begin()->Name);
        }
        else
        {
            sprintf_s(buffer, 256, "%d Chars", mCharacters.size());
        }
        pSearchTable->SetColumnHeader(0, buffer);

        int row = 0;    
        for (auto rowIter = mLocationIndices.begin(); rowIter != mLocationIndices.end(); rowIter++)
        {
            int rowTotal = 0;
            pSearchTable->SetRowHeader(row, gContainerNames[*rowIter]);

            std::list<SearchResult_t*> rowMatches;
            for (auto resultIter = mResultVector.begin(); resultIter != mResultVector.end(); resultIter++)
            {
                if ((resultIter->Count[*rowIter] > 0) || (resultIter->StorageSlipContainer == *rowIter))
                {
                    rowMatches.push_back(&(*resultIter));
                }
            }

            int column = 1;
            for (auto columnIter = mItemResources.begin(); columnIter != mItemResources.end(); columnIter++)
            {
                if (mDuplicateItemName)
                    sprintf_s(buffer, 256, "%s[%u]", (*columnIter)->Name[0], (*columnIter)->Id);
                else
                    strcpy_s(buffer, 256, (*columnIter)->Name[0]);
                pSearchTable->SetColumnHeader(column, buffer);

                int count = 0;
                for (auto resultIter = rowMatches.begin(); resultIter != rowMatches.end(); resultIter++)
                {
                    if ((*resultIter)->Id == ((*columnIter)->Id))
                    {
                        count += (*resultIter)->Count[*rowIter];
                        if ((*resultIter)->StorageSlipContainer == *rowIter)
                            count++;
                    }
                }
                pSearchTable->SetTableData(row, column, count);
                rowTotal += count;
                column++;
            }

            pSearchTable->SetTableData(row, column, rowTotal);
            row++;
        }
    }

    else if (mSearchMode == SearchDisplayMode::LocationCharacter)
    {
        pSearchTable = new SearchTable(mContainerCount + 1, mCharacterCount + 2);

        char buffer[256];
        if (mItemResources.size() != 1)
        {
            sprintf_s(buffer, 256, "%d ItemIds", mItemResources.size());
        }
        else
        {
            sprintf_s(buffer, 256, "%s", (*mItemResources.begin())->Name[0]);
        }
        pSearchTable->SetColumnHeader(0, buffer);

        int row = 0;
        for (auto rowIter = mLocationIndices.begin(); rowIter != mLocationIndices.end(); rowIter++)
        {
            int rowTotal = 0;
            pSearchTable->SetRowHeader(row, gContainerNames[*rowIter]);

            std::list<SearchResult_t*> rowMatches;
            for (auto resultIter = mResultVector.begin(); resultIter != mResultVector.end(); resultIter++)
            {
                if ((resultIter->Count[*rowIter] > 0) || (resultIter->StorageSlipContainer == *rowIter))
                {
                    rowMatches.push_back(&(*resultIter));
                }
            }

            int column = 1;
            for (auto columnIter = mCharacters.begin(); columnIter != mCharacters.end(); columnIter++)
            {
                if (mDuplicatePlayerName)
                    sprintf_s(buffer, 256, "%s[%u]", columnIter->Name, columnIter->Id);
                else
                    strcpy_s(buffer, 256, columnIter->Name);
                pSearchTable->SetColumnHeader(column, buffer);

                int count = 0;
                for (auto resultIter = rowMatches.begin(); resultIter != rowMatches.end(); resultIter++)
                {
                    if ((*resultIter)->Character == *columnIter)
                    {
                        count += (*resultIter)->Count[*rowIter];
                        if ((*resultIter)->StorageSlipContainer == *rowIter)
                            count++;
                    }
                }
                pSearchTable->SetTableData(row, column, count);
                rowTotal += count;
                column++;
            }

            pSearchTable->SetTableData(row, column, rowTotal);
            row++;
        }
    }

    else if (mSearchMode == SearchDisplayMode::CharacterItem)
    {
        pSearchTable = new SearchTable(mCharacterCount + 1, mItemCount + 2);

        char buffer[256];
        pSearchTable->SetColumnHeader(0, "");

        int row = 0;
        for (auto rowIter = mCharacters.begin(); rowIter != mCharacters.end(); rowIter++)
        {
            int rowTotal = 0;
            if (mDuplicatePlayerName)
                sprintf_s(buffer, 256, "%s[%u]", rowIter->Name, rowIter->Id);
            else
                strcpy_s(buffer, 256, rowIter->Name);
            pSearchTable->SetRowHeader(row, buffer);


            std::list<SearchResult_t*> rowMatches;
            for (auto resultIter = mResultVector.begin(); resultIter != mResultVector.end(); resultIter++)
            {
                if (resultIter->Character == *rowIter)
                {
                    rowMatches.push_back(&(*resultIter));
                }
            }

            int column = 1;
            for (auto columnIter = mItemResources.begin(); columnIter != mItemResources.end(); columnIter++)
            {
                if (mDuplicateItemName)
                    sprintf_s(buffer, 256, "%s[%u]", (*columnIter)->Name[0], (*columnIter)->Id);
                else
                    strcpy_s(buffer, 256, (*columnIter)->Name[0]);
                pSearchTable->SetColumnHeader(column, buffer);

                int count = 0;
                for (auto resultIter = rowMatches.begin(); resultIter != rowMatches.end(); resultIter++)
                {
                    if ((*resultIter)->Id == ((*columnIter)->Id))
                    {
                        count += (*resultIter)->Total;
                    }
                }
                pSearchTable->SetTableData(row, column, count);
                rowTotal += count;
                column++;
            }
            
            pSearchTable->SetTableData(row, column, rowTotal);
            row++;
        }
    }

    pSearchTable->SetColumnHeader(pSearchTable->GetColumnCount() - 1, "Total");
    pSearchTable->SetRowHeader(pSearchTable->GetRowCount() - 1, "Total");
    //Sum columns..
    for (int column = 1; column < pSearchTable->GetColumnCount(); column++)
    {
        int total = 0;
        for (int row = 0; (row + 1) < pSearchTable->GetRowCount(); row++)
        {
            total += pSearchTable->GetTableData(row, column);
        }
        pSearchTable->SetTableData(pSearchTable->GetRowCount() - 1, column, total);
    }
}

int SearchInstance::GetCharacterCount()
{
    return mCharacterCount;
}
int SearchInstance::GetItemCount()
{
    return mItemCount;
}
int SearchInstance::GetItemTotal()
{
    return mItemTotal;
}
int SearchInstance::GetContainerCount()
{
    return mContainerCount;
}
std::vector<std::string> SearchInstance::GetQuery()
{
    return mSearchTerms;
}

std::vector<SearchResult_t> SearchInstance::GetResultVector()
{
    return mResultVector;
}
SearchTable* SearchInstance::GetResultTable()
{
    return pSearchTable;
}