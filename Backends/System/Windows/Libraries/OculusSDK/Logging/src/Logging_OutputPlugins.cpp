/************************************************************************************

Filename    :   Logging_OutputPlugins.cpp
Content     :   Logging output plugins
Created     :   Oct 26, 2015
Authors     :   Chris Taylor

Copyright   :   Copyright 2015-2016 Oculus VR, LLC All Rights reserved.

Licensed under the Oculus VR Rift SDK License Version 3.3 (the "License");
you may not use the Oculus VR Rift SDK except in compliance with the License,
which is provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

You may obtain a copy of the License at

http://www.oculusvr.com/licenses/LICENSE-3.3

Unless required by applicable law or agreed to in writing, the Oculus VR SDK
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#include "Logging_OutputPlugins.h"
#include "Logging_Tools.h"

#include <iostream>
#include <time.h>

namespace ovrlog {


//-----------------------------------------------------------------------------
// Console

OutputConsole::OutputConsole()
{
}

OutputConsole::~OutputConsole()
{
}

const char* OutputConsole::GetUniquePluginName()
{
    return "DefaultOutputConsole";
}

void OutputConsole::Write(Level level, const char* /*subsystem*/, const char* header, const char* utf8msg)
{
    #if defined(_WIN32)
        HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
        
        // Save current console attributes
        CONSOLE_SCREEN_BUFFER_INFO bufInfo = { 0 };
        BOOL oldAttrValid = ::GetConsoleScreenBufferInfo(hConsole, &bufInfo);
        WORD attr = 0;

        switch (level)
        {
        case Level::Trace:
            attr |= FOREGROUND_BLUE | FOREGROUND_RED;
            break;
        case Level::Debug:
            attr |= FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
            break;
        case Level::Info:
            attr |= FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
            break;
        case Level::Warning:
            attr |= FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
            break;
        case Level::Error:
            attr |= FOREGROUND_RED | FOREGROUND_INTENSITY;
            break;
        default:
            break;
        }
        static_assert(Level::Count == static_cast<Level>(5), "Needs updating");

        ::SetConsoleTextAttribute(hConsole, attr & ~FOREGROUND_INTENSITY);

        std::cout << header;

        if ((attr & FOREGROUND_INTENSITY) != 0)
        {
            ::SetConsoleTextAttribute(hConsole, attr);
        }

        std::cout << utf8msg << std::endl;

        // Restore original attributes, if saved
        if ( TRUE == oldAttrValid )
        {
            ::SetConsoleTextAttribute(hConsole, bufInfo.wAttributes);
        }
    #else

        FILE* output = stdout;

        switch(level)
        {
        case Level::Trace:
          break;
        case Level::Debug:
          break;
        case Level::Info:
          break;
        case Level::Warning:
          output = stderr;
          fprintf(output, "\e[38;5;255m\e[48;5;208m");
          break;
        case Level::Error:
          output = stderr;
          fprintf(output, "\e[38;5;255m\e[48;5;196m");
          break;
        default:
          break;
        }

        fprintf(output, "%s", header);
        fprintf(output, "\e[0m ");

        fprintf(output, "%s\n", utf8msg);
    #endif
}


//-----------------------------------------------------------------------------
// System Application Event Log

#ifndef OVR_SYSLOG_NAME
    #define OVR_SYSLOG_NAME L"OculusVR"
#endif // OVR_SYSLOG_NAME

OutputEventLog::OutputEventLog()
  : hEventSource(nullptr), MinReportEventLevel(Level::Error)
{
    #if defined(_WIN32)
        hEventSource = ::RegisterEventSourceW(
            nullptr, // No server name
            OVR_SYSLOG_NAME); // Syslog event source name
    #else
        // To do.
    #endif
    
    if (!hEventSource)
    {
        // Unable to register event source
        LOGGING_DEBUG_BREAK();
    }
}

OutputEventLog::~OutputEventLog()
{
}

const char* OutputEventLog::GetUniquePluginName()
{
    return "DefaultOutputEventLog";
}

void OutputEventLog::Write(Level level, const char* subsystem, const char* header, const char* utf8msg)
{
    (void)subsystem; // unused

    if (level < MinReportEventLevel)
    {
        return;
    }

    if (!hEventSource)
    {
        return;
    }

    #if defined(_WIN32)
        WORD mType = 0;

        switch (level)
        {
        case Level::Trace:
            mType = EVENTLOG_INFORMATION_TYPE;
            return; // Do not log at this level.
        case Level::Debug:
            mType = EVENTLOG_INFORMATION_TYPE;
            return; // Do not log at this level.
        case Level::Info:
            mType = EVENTLOG_INFORMATION_TYPE;
            return; // Do not log at this level.

        case Level::Warning:
            mType = EVENTLOG_WARNING_TYPE;
            break; // Log at this level.
        case Level::Error:
            mType = EVENTLOG_ERROR_TYPE;
            break; // Log at this level.

        default:
            break;
        }
        static_assert(Level::Count == static_cast<Level>(5), "Needs updating");

        const size_t MAX_REPORT_EVENT_A_LEN = 31839;

        std::vector<const char*> cstrVtr;
        cstrVtr.push_back(header);
        std::vector<std::string> splitStrings;
        size_t longStringLen = strlen(utf8msg);
        if (longStringLen >= MAX_REPORT_EVENT_A_LEN)
        {
            std::string longStr(utf8msg);
            for (size_t x = 0; x < longStringLen; x += MAX_REPORT_EVENT_A_LEN)
            {
                size_t remaining = longStringLen - x;
                std::string thisSubStr = longStr.substr(x, (remaining > MAX_REPORT_EVENT_A_LEN) ? MAX_REPORT_EVENT_A_LEN : remaining);
                splitStrings.push_back(thisSubStr);
            }

            for (size_t i = 0; i < splitStrings.size(); i++)
            {
                cstrVtr.push_back(splitStrings[i].c_str());
            }
        }
        else
        {
            cstrVtr.push_back(utf8msg);
        }


        if (!::ReportEventA(
            hEventSource, // Event source
            mType, // Event log level
            0, // Default category
            0, // Default event id
            nullptr, // No security identifier
            (WORD) cstrVtr.size(), // Number of strings
            0, // No bytes of event-specific binary data attached
            &cstrVtr[0], // String array
            nullptr)) // No binary data attached
        {
            // Unable to write event log
            LOGGING_DEBUG_BREAK();
        }
    #else
        (void)header;
        (void)utf8msg;
        // To do.
    #endif
}


//-----------------------------------------------------------------------------
// DbgView

OutputDbgView::OutputDbgView()
{
}

OutputDbgView::~OutputDbgView()
{
}

const char* OutputDbgView::GetUniquePluginName()
{
    return "DefaultOutputDbgView";
}

void OutputDbgView::Write(Level level, const char* subsystem, const char* header, const char* utf8msg)
{
    (void)subsystem; // unused
    (void)level; // unused

    // Build up a single string to send to OutputDebugStringA so it
    // all appears on the same line in DbgView.
    std::stringstream ss;
    ss << header << utf8msg << "\n";

    #if defined(_WIN32)
        ::OutputDebugStringA(ss.str().c_str());
    #else
        fputs(ss.str().c_str(), stderr);
    #endif
}


} // namespace ovrlog

#ifdef OVR_STRINGIZE
#error "This code must remain independent of LibOVR"
#endif
