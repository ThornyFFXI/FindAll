#include "FindAll.h"

void FindAll::ImguiDisplay()
{
    IGuiManager* imgui = m_AshitaCore->GetGuiManager();
    char objectName[256] = {0};

    imgui->Begin("FindAll", 0, ImGuiWindowFlags_AlwaysAutoResize);
    
    if (imgui->BeginTabBar("FindAllTabBar"))
    {
        for (std::list<SearchInstance*>::iterator searchIter = mSearches.begin(); searchIter != mSearches.end();)
        {
            SearchInstance* pSearch = *searchIter;
            SearchTable* pTable     = pSearch->GetResultTable();
            int rowCount            = pTable->GetRowCount();
            int columnCount         = pTable->GetColumnCount();

            sprintf_s(objectName, 256, "Query: %s", pSearch->GetQuery());
            if (imgui->BeginTabItem(objectName, 0, ImGuiTabItemFlags_NoCloseWithMiddleMouseButton))
            {
                if ((imgui->IsItemHovered()) && (imgui->IsMouseDoubleClicked(0)))
                {
                    searchIter = mSearches.erase(searchIter);
                    imgui->EndTabItem();
                    continue;
                }

                sprintf_s(objectName, 256, "FindAllTable_%s", pSearch->GetQuery());
                if (imgui->BeginTable(objectName, columnCount, ImGuiTableFlags_SizingStretchSame))
                {
                    for (int x = 0; x < columnCount; x++)
                    {
                        imgui->TableSetupColumn(pTable->GetColumnHeader(x), ImGuiTableColumnFlags_NoSort, 0, x);
                    }
                    imgui->TableHeadersRow();
                    for (int row = 0; row < rowCount; row++)
                    {
                        if (imgui->TableNextColumn())
                        {
                            imgui->Text(pTable->GetRowHeader(row));
                            for (int column = 1; column < columnCount; column++)
                            {
                                if (imgui->TableNextColumn())
                                {
                                    imgui->Text("%d", pTable->GetTableData(row, column));
                                }
                            }
                        }
                    }
                    imgui->EndTable();
                }
                imgui->EndTabItem();
            }
            searchIter++;
        }
        imgui->EndTabBar();
    }
    imgui->End();
}