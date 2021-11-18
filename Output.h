#ifndef __ASHITA_ThornyOutput_H_INCLUDED__
#define __ASHITA_ThornyOutput_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "C:\Ashita 4\plugins\sdk\Ashita.h"
#include <iostream>
#include <string>

using namespace std;

#define ERROR_COLOR std::string("\x1E\x44")
#define HIGHLIGHT_COLOR std::string("\x1E\x02")
#define MESSAGE_COLOR std::string("\x1E\x6A")

class Output
{
public:
    static std::string PluginName;
    static std::string Format(std::string input, std::string basicColor)
    {
        std::string outString      = "\x1E\x51[\x1E\x06" + PluginName + "\x1E\x51]\x1E\x01 " + basicColor;
        bool colorCode = false;
        int length                 = input.length();
        for (int x = 0; x < length; x++)
        {
            if (colorCode)
            {
                if (input[x] == '$')
                {
                    outString += '$';
                }
                else if (input[x] == 'H')
                {
                    outString += HIGHLIGHT_COLOR;
                }
                else if (input[x] == 'R')
                {
                    outString += basicColor;
                }
                else
                {
                    outString += input[x - 1];
                    outString += input[x];
                }
                colorCode = false;
            }
            else if (input[x] == '$')
            {
                colorCode = true;
            }
            else
            {
                outString += input[x];
            }
        }
        return outString;
    }
    static std::string Error(std::string input)
    {
        return Format(input, ERROR_COLOR);
    }
    static std::string Errorf(std::string input, ...)
    {
        std::string outString = Format(input, ERROR_COLOR);

        va_list args;
        va_start(args, input);

        char buffer[1024];
        vsprintf_s(buffer, 1024, outString.c_str(), args);
        va_end(args);

        return buffer;
    }
    static std::string Message(std::string input)
    {
        return Format(input, MESSAGE_COLOR);
    }
    static std::string Messagef(std::string input, ...)
    {
        std::string outString = Format(input, MESSAGE_COLOR);

        va_list args;
        va_start(args, input);

        char buffer[1024];
        vsprintf_s(buffer, 1024, outString.c_str(), args);
        va_end(args);

        return buffer;
    }
};
#endif