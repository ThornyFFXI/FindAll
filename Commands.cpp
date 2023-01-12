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
            OutputHelper::Outputf(Ashita::LogLevel::Info, "InstantLoad $H%s$R.", mConfig.GetInstantLoad() ? "Enabled" : "Disabled");
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
            OutputHelper::Outputf(Ashita::LogLevel::Info, "CacheToDisc $H%s$R.", mConfig.GetCacheToDisc() ? "Enabled" : "Disabled");
            return;
        }

        if ARG (2, "displaymax")
        {
            if (argCount > 3)
            {
                mConfig.SetDisplayMax(atoi(args[3].c_str()));
                OutputHelper::Outputf(Ashita::LogLevel::Info, "DisplayMax set to $H%d$R.", mConfig.GetDisplayMax());
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
                OutputHelper::Outputf(Ashita::LogLevel::Info, "DisplayMode set to $HImGui$R.");
            else
                OutputHelper::Outputf(Ashita::LogLevel::Info, "DisplayMode set to $HChat Log$R.");
            return;
        }

        if ARG(2, "writedelay")
        {
            if (argCount > 3)
            {
                mConfig.SetWritePeriod(static_cast<uint32_t>(atoll(args[3].c_str())));
                OutputHelper::Outputf(Ashita::LogLevel::Info, "WriteDelay set to $H%u$R.", mConfig.GetWritePeriod());
            }
            return;
        }
    }

    else if ARG(1, "clear")
    {
        for (std::list<SearchInstance*>::iterator searchIter = mSearches.begin(); searchIter != mSearches.end(); searchIter = mSearches.erase(searchIter))
        {
            delete *searchIter;
        }
        return;
    }

    else if ARG(1, "search")
    {
        if (argCount > 2)
        {
            std::vector<std::string> terms;
            for (int x = 2; x < argCount; x++)
            {
                terms.push_back(std::string(args[x]));
            }
            FindAcrossCharacters(terms);
        }
    }

    else if ARG (1, "local")
    {
        if (argCount > 2)
        {
            std::vector<std::string> terms;
            for (int x = 2; x < argCount; x++)
            {
                terms.push_back(std::string(args[x]));
            }
            FindLocal(terms);
        }
    }

    else if (argCount > 1)
    {
        std::vector<std::string> terms;
        for (int x = 1; x < argCount; x++)
        {
            terms.push_back(std::string(args[x]));
        }
        FindAcrossCharacters(terms);
    }
}