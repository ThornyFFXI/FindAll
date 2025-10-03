#ifndef __ASHITA_ThornyOutput_H_INCLUDED__
#define __ASHITA_ThornyOutput_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "Ashita.h"
#include <iostream>
#include <string>

using namespace std;

class OutputHelper
{
private:
    IAshitaCore* pAshitaCore;
    ILogManager* pLogManager;
    static OutputHelper* pOutputHelper;
    std::string mColors[6] = {
        std::string("\x1E\x6A"), //No log
        std::string("\x1E\x44"), //Critical
        std::string("\x1E\x44"), //Error
        std::string("\x1E\x44"), //Warning
        std::string("\x1E\x6A"), //Info
        std::string("\x1E\x6A")  //Debug
    };
    std::string mHighlight = std::string("\x1E\x02");
    char mPluginName[256];

    OutputHelper(IAshitaCore* pAshitaCore, ILogManager* pLogManager, const char* pluginName)
        : pAshitaCore(pAshitaCore)
        , pLogManager(pLogManager)
    {
        strcpy_s(mPluginName, 256, pluginName);
        OutputHelper::pOutputHelper = this;
    }

    std::string Format(Ashita::LogLevel logLevel, std::string input)
    {
        uint32_t level = static_cast<uint32_t>(logLevel);
        std::stringstream outLog;
        std::stringstream outString;
        outString << "\x1E\x51[\x1E\x06" << mPluginName << "\x1E\x51]\x1E\x01 " << mColors[level];
        bool colorCode = false;
        int length     = input.length();
        for (int x = 0; x < length; x++)
        {
            if (colorCode)
            {
                if (input[x] == '$')
                {
                    outString << '$';
                    outLog << '$';
                }
                else if (input[x] == 'H')
                {
                    outString << mHighlight;
                }
                else if (input[x] == 'R')
                {
                    outString << mColors[level];
                }
                else
                {
                    outLog << input[x - 1];
                    outLog << input[x];
                    outString << input[x - 1];
                    outString << input[x];
                }
                colorCode = false;
            }
            else if (input[x] == '$')
            {
                colorCode = true;
            }
            else
            {
                outString << input[x];
                outLog << input[x];
            }
        }
        if (level != 0)
        {
            pLogManager->Log(level, mPluginName, outLog.str().c_str());
        }
        return outString.str();
    }

public:
    void Handle(Ashita::LogLevel logLevel, std::string input)
    {
        std::string outString = Format(logLevel, input);
        pAshitaCore->GetChatManager()->Write(0, false, outString.c_str());
    }

    static void Destroy()
    {
        if (OutputHelper::pOutputHelper != nullptr)
        {
            delete OutputHelper::pOutputHelper;
        }
    }
    static void Initialize(IAshitaCore* pAshitaCore, ILogManager* pLogManager, const char* pluginName)
    {
        if (OutputHelper::pOutputHelper == nullptr)
        {
            OutputHelper::pOutputHelper = new OutputHelper(pAshitaCore, pLogManager, pluginName);
        }
    }

    static void Output(Ashita::LogLevel logLevel, std::string input)
    {
        OutputHelper::pOutputHelper->Handle(logLevel, input);
    }
    static void Outputf(Ashita::LogLevel logLevel, std::string input, ...)
    {
        va_list args;
        va_start(args, input);

        char buffer[1024];
        vsprintf_s(buffer, 1024, input.c_str(), args);
        va_end(args);

        OutputHelper::pOutputHelper->Handle(logLevel, buffer);
    }
};
#endif