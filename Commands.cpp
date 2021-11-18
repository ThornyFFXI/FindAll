#include "FindAll.h"

#define ARG(a, b) ((argCount > a) && (_stricmp(args[a].c_str(), b) == 0))

void FindAll::HandleCommandInternal(std::vector<std::string> args, int argCount)
{
    if ARG(1, "config")
    {
        if ARG(2, "instantload")
        {
            if (ARG(3, "on") || ARG(3, "enabled"))
            {
                mConfig.SetInstantLoad(true);
            }
            else if (ARG(3, "off") || ARG(3, "disabled"))
            {
                mConfig.SetInstantLoad(false);
            }
            else
            {
                mConfig.SetInstantLoad(!mConfig.GetInstantLoad());
            }
            m_AshitaCore->GetChatManager()->Write(0, false, Output::Messagef("InstantLoad $H%s$R.", mConfig.GetInstantLoad() ? "Enabled" : "Disabled").c_str());
            return;
        }

        if ARG(2, "cachetodisc")
        {
            if (ARG(3, "on") || ARG(3, "enabled"))
            {
                mConfig.SetCacheToDisc(true);
            }
            else if (ARG(3, "off") || ARG(3, "disabled"))
            {
                mConfig.SetCacheToDisc(false);
            }
            else
            {
                mConfig.SetCacheToDisc(!mConfig.GetCacheToDisc());
            }
            m_AshitaCore->GetChatManager()->Write(0, false, Output::Messagef("CacheToDisc $H%s$R.", mConfig.GetCacheToDisc() ? "Enabled" : "Disabled").c_str());
            return;
        }

        if ARG (2, "displaymax")
        {
            if (argCount > 3)
            {
                mConfig.SetDisplayMax(atoi(args[3].c_str()));
                m_AshitaCore->GetChatManager()->Write(0, false, Output::Messagef("DisplayMax set to $H%d$R.", mConfig.GetDisplayMax()).c_str());
            }
            return;
        }

        if ARG(2, "displaymode")
        {
            if ARG(3, "chatlog")
            {
                mConfig.SetDisplayMode(FindAllDisplayMode::ChatLog);
            }
            else if ARG (3, "imgui")
            {
                mConfig.SetDisplayMode(FindAllDisplayMode::ImGui);
            }
            else if (mConfig.GetDisplayMode() == FindAllDisplayMode::ImGui)
            {
                mConfig.SetDisplayMode(FindAllDisplayMode::ChatLog);
            }
            else
            {
                mConfig.SetDisplayMode(FindAllDisplayMode::ImGui);
            }

            if (mConfig.GetDisplayMode() == FindAllDisplayMode::ImGui)
                m_AshitaCore->GetChatManager()->Write(0, false, Output::Message("DisplayMode set to $HImGui$R.").c_str());
            else
                m_AshitaCore->GetChatManager()->Write(0, false, Output::Message("DisplayMode set to $HChat Log$R.").c_str());
            return;
        }

        if ARG(2, "writedelay")
        {
            if (argCount > 3)
            {
                mConfig.SetWritePeriod(static_cast<uint32_t>(atoll(args[3].c_str())));
                m_AshitaCore->GetChatManager()->Write(0, false, Output::Messagef("WriteDelay set to $H%u$R.", mConfig.GetWritePeriod()).c_str());
            }
            return;
        }
    }

    if ARG(1, "clear")
    {
        for (std::list<SearchInstance*>::iterator searchIter = mSearches.begin(); searchIter != mSearches.end(); searchIter = mSearches.erase(searchIter))
        {
            delete *searchIter;
        }
        return;
    }

    if ARG (1, "search")
    {
        if (argCount > 2)
        {
            FindAcrossCharacters(args[2].c_str());
        }
    }

}