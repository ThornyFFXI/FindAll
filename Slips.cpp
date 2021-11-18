#include "FindAll.h"

void FindAll::LoadSlipData()
{
    char path[256];
    sprintf_s(path, 256, "%sresources\\findall\\slips.xml", m_AshitaCore->GetInstallPath());
    std::ifstream inputStream = ifstream(path, ios::in | ios::binary | ios::ate);
    if (!inputStream.is_open())
    {
        m_AshitaCore->GetChatManager()->Write(0, false, Output::Errorf("Failed to read file: $H%s$R", path).c_str());
        return;
    }

    long size    = inputStream.tellg();
    char* buffer = nullptr;

    try
    {
        buffer = new char[size + 1];
        inputStream.seekg(0, ios::beg);
        inputStream.read(buffer, size);
        buffer[size] = '\0';
        inputStream.close();
    }
    catch (const std::exception e)
    {
        if (buffer != nullptr)
            delete[] buffer;

        m_AshitaCore->GetChatManager()->Write(0, false, Output::Errorf("Exception while reading $H%s$R: %s$H%s$R", path, e.what()).c_str());
        return;
    }
    catch (...)
    {
        if (buffer != nullptr)
            delete[] buffer;

        m_AshitaCore->GetChatManager()->Write(0, false, Output::Errorf("Unknown exception while reading file: $H%s$R", path).c_str());
        return;
    }

    xml_document<>* pDocument = new xml_document<>();
    try
    {
        pDocument->parse<0>(buffer);
    }
    catch (const rapidxml::parse_error& e)
    {
        int line = static_cast<long>(std::count(buffer, e.where<char>(), '\n') + 1);
        m_AshitaCore->GetChatManager()->Write(0, false, Output::Errorf("Parse error while evaluating $H%s$R on line $H%d$R.", path, line).c_str());
        m_AshitaCore->GetChatManager()->Write(0, false, Output::Errorf("Error message: %H%s$R", e.what()).c_str());
        delete pDocument;
        delete[] buffer;
        return;
    }
    catch (...)
    {
        m_AshitaCore->GetChatManager()->Write(0, false, Output::Errorf("Parse error while evaluating $H%s$R.  No message specified.", path).c_str());
        delete pDocument;
        delete[] buffer;
        return;
    }

    xml_node<>* pRoot = pDocument->first_node("slips");
    if (pRoot)
    {
        for (xml_node<>* pNode = pRoot->first_node("slip"); pNode; pNode = pNode->next_sibling("slip"))
        {
            StorageSlip_t slip = {0};
            xml_attribute<>* pAttr = pNode->first_attribute("id");
            if (pAttr)
            {
                slip.ItemId = atoi(pAttr->value());

                for (xml_node<>* pSubNode = pNode->first_node("item"); pSubNode; pSubNode = pSubNode->next_sibling("item"))
                {
                    xml_attribute<>* pOffset = pSubNode->first_attribute("offset");
                    xml_attribute<>* pId     = pSubNode->first_attribute("id");
                    if ((pOffset) && (pId))
                    {
                        slip.StoredItem[atoi(pOffset->value())] = static_cast<uint16_t>(atoi(pId->value()));
                    }                    
                }

                mSlips.push_back(slip);
            }
        }
    }

    delete pDocument;
    delete[] buffer;
}