#include "FindAll.h"
OutputHelper* OutputHelper::pOutputHelper = nullptr;

__declspec(dllexport) IPlugin* __stdcall expCreatePlugin(const char* args)
{
    UNREFERENCED_PARAMETER(args);

    return (IPlugin*)(new FindAll());
}

__declspec(dllexport) double __stdcall expGetInterfaceVersion(void)
{
    return ASHITA_INTERFACE_VERSION;
}

bool FindAll::Initialize(IAshitaCore* core, ILogManager* logger, const uint32_t id)
{
    m_AshitaCore = core;
    m_LogManager = logger;
    m_PluginId   = id;
    mPending.State = (uint32_t)SearchState::Idle;

    OutputHelper::Initialize(core, logger, "FindAll");
    ConfigLoader::pLoader = new ConfigLoader(core, &mConfig);
    pCache       = new InventoryCache(core, &mConfig);
    LoadSlipData();
    return true;
}

void FindAll::Release(void)
{
    m_AshitaCore->GetFontManager()->Delete("FindAll_Display");
    delete pCache;
    OutputHelper::Destroy();
}

bool FindAll::HandleCommand(int32_t mode, const char* command, bool injected)
{
	UNREFERENCED_PARAMETER(mode);
	UNREFERENCED_PARAMETER(command);
	UNREFERENCED_PARAMETER(injected);
    std::vector<std::string> args;
    int argCount = Ashita::Commands::GetCommandArgs(command, &args);
    
	if ((argCount > 1) && ((_stricmp(args[0].c_str(), "/fa") == 0) || (_stricmp(args[0].c_str(), "/findall") == 0)))
    {
        HandleCommandInternal(args, argCount);
        return true;
    }

	return false;
}

bool FindAll::HandleIncomingPacket(uint16_t id, uint32_t size, const uint8_t* data, uint8_t* modified, uint32_t sizeChunk, const uint8_t* dataChunk, bool injected, bool blocked)
{
    if (!injected)
    {
        if (id == 0x00A)
        {
            ConfigLoader::pLoader->Handle0x00A(data);
        }
        pCache->HandleIncomingPacket(id, size, data);
    }    
	return false;
}

bool FindAll::Direct3DInitialize(IDirect3DDevice8* device)
{
	this->m_Direct3DDevice = device;
	return true;
}

void FindAll::Direct3DPresent(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion)
{
    pCache->HandleTick();

    if (InterlockedCompareExchange(&mPending.State, 0, 2) == 2)
    {
        if (mPending.pSearch == nullptr)
        {
            if (mPending.Terms.size() == 1)
                OutputHelper::Outputf(Ashita::LogLevel::Error, "Could not find any items matching the term: $H%s$R.", mPending.Terms[0].c_str());
            else
                OutputHelper::Outputf(Ashita::LogLevel::Error, "Could not find any items matching any of %d search terms.", mPending.Terms.size());
        }
        else
        {
            DisplayResults(mPending.pSearch);
        }
        this->ResetEnd();
    }


    if (mConfig.GetDisplayMode() == FindAllDisplayMode::ChatLog)
    {
        for (std::list<SearchInstance*>::iterator searchIter = mSearches.begin(); searchIter != mSearches.end(); searchIter = mSearches.erase(searchIter))
        {
            delete *searchIter;
        }
    }
    else if (mSearches.size() > 0)
    {
        ImguiDisplay();
    }
}