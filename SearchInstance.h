#ifndef __ASHITA_SearchInstance_H_INCLUDED__
#define __ASHITA_SearchInstance_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "C:\Ashita 4\plugins\sdk\Ashita.h"
#include "InventoryCache.h"

struct SearchItem_t
{
    uint16_t ItemId;
    uint16_t SlipId;
    uint16_t SlipOffset;
};

struct StorageSlip_t
{
    uint16_t ItemId;
    uint16_t StoredItem[224];
};

struct SearchResult_t
{
    CharacterIdentifier_t Character;
    uint16_t Id;
    IItem* Resource;
    uint32_t Total;
    uint32_t Count[CONTAINER_MAX];
    int16_t StorageSlipContainer;
};
struct ItemStub_t
{
    uint32_t Count;
    IItem* pResource;

    ItemStub_t(IItem* resource, uint32_t count)
        : pResource(resource)
        , Count(count)
    {}

    bool operator==(const ItemStub_t& other)
    {
        return (pResource == other.pResource);
    }
};

class SearchTable
{
    char** ColumnHeaders;
    char** RowHeaders;
    int Columns;
    int Rows;
    int32_t** Data;
public:
    SearchTable(int rows, int columns)
        : Rows(rows)
        , Columns(columns)
    {
        ColumnHeaders = new char*[columns];
        RowHeaders    = new char*[rows];
        Data = new int32_t*[rows];
        for (int x = 0; x < Columns; x++)
        {
            ColumnHeaders[x] = new char[256];
        }
        for (int x = 0; x < Rows; x++)
        {
            RowHeaders[x] = new char[256];
            Data[x]       = new int32_t[columns];
        }
    }
    ~SearchTable()
    {
        for (int x = 0; x < Columns; x++)
        {
            delete[] ColumnHeaders[x];
        }
        for (int x = 0; x < Rows; x++)
        {
            delete[] RowHeaders[x];
            delete[] Data[x];
        }
        delete[] ColumnHeaders;
        delete[] RowHeaders;
        delete[] Data;
    }

    const char* GetColumnHeader(int x)
    {
        if ((x < 0) || (x >= Columns))
            return nullptr;
        return ColumnHeaders[x];

    }
    int GetColumnCount()
    {
        return Columns;
    }
    int GetRowCount()
    {
        return Rows;
    }
    const char* GetRowHeader(int x)
    {
        if ((x < 0) || (x >= Rows))
            return nullptr;
        return RowHeaders[x];
    }

    int32_t GetTableData(int row, int column)
    {
        if ((row < 0) || (row >= Rows) || (column < 0) || (column >= Columns))
        {
            return 0;
        }
        return Data[row][column];
    }

    void SetColumnHeader(int x, const char* data)
    {
        if ((x < 0) || (x >= Columns))
            return;
        strcpy_s(ColumnHeaders[x], 256, data);
    }

    void SetRowHeader(int x, const char* data)
    {
        if ((x < 0) || (x >= Rows))
            return;
        strcpy_s(RowHeaders[x], 256, data);
    }

    void SetTableData(int row, int column, int32_t data)
    {
        if ((row < 0) || (row >= Rows) || (column < 0) || (column >= Columns))
        {
            return;
        }
        Data[row][column] = data;
    }
};

enum class SearchDisplayMode
{
    Unsorted,
    LocationItem,
    LocationCharacter,
    CharacterItem
};

class SearchInstance
{
private:
    std::vector<SearchResult_t> mResultVector;
    SearchDisplayMode mSearchMode;
    SearchTable* pSearchTable;

    std::vector<CharacterIdentifier_t> mCharacters;
    std::vector<IItem*> mItemResources;
    std::vector<int32_t> mLocationIndices;

    int mCharacterCount;
    int mItemTotal;
    int mItemCount;
    int mContainerCount;

    bool mDuplicateItemName;
    bool mDuplicatePlayerName;
    char mQuery[256];

public:
    SearchInstance(std::vector<SearchResult_t> results, const char* query, int maxResults);
    ~SearchInstance();

    void SetSearchMode(SearchDisplayMode mode);
    void BuildSearchTable();
    int GetCharacterCount();
    int GetItemCount();
    int GetContainerCount();
    int GetItemTotal();
    const char* GetQuery();
    std::vector<SearchResult_t> GetResultVector();
    SearchTable* GetResultTable();
};

#endif