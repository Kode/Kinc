/************************************************************************************

Filename    :   Logging.h
Content     :   Logging system
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

#pragma once

#pragma warning(push)
#pragma warning(disable: 4530) // C++ exception handler used, but unwind semantics are not enabled

#include <string>
#include <array>
#include <sstream>
#include <vector>
#include <queue>
#include <memory>
#include <set>
#include <map>
#include <time.h>

#pragma warning(push)

#include "Logging_Tools.h"

#if defined(__clang__)
    #pragma clang diagnostic ignored "-Wformat-nonliteral"
    #pragma clang diagnostic ignored "-Wformat-security"  // Otherwise the printf usage below generates a warning.
#endif


namespace ovrlog {

struct LogStringBuffer;
class Channel;
class Configurator;

typedef uint32_t Log_Level_t;
typedef uint32_t Write_Option_t;

//-----------------------------------------------------------------------------
// Log Level
//
// Log message priority is indicated by its level.  The level can inform how
// prominently it is displayed on the console window or whether or not a
// message is displayed at all.

enum class Level : Log_Level_t
{

    // Trace message.  This is a message that can potentially happen once per
    // camera/HMD frame and are probably being reviewed after they are recorded
    // since they will scroll by too fast otherwise.
    Trace,

    // Debug message.  This is a verbose log level that can be selectively
    // turned on by the user in the case of perceived problems to help
    // root-cause the problems.  This log level is intended to be used for
    // messages that happen less often than once per camera/HMD frame,
    // such that they can be human readable.
    Debug,

    // Info messages, which should be the default message type for infrequent
    // messages used during subsystem initialization and/or shutdown.  This
    // log level is fairly visible so should be used sparingly.  Expect users to
    // have these turned on, so avoid printing anything here that would obscure
    // Warning and Error messages.
    Info,

    // Warning message, which is also displayed almost everywhere.  For most
    // purposes it is as visible as an Error message, so it should also be used
    // very selectively.  The main difference from Error level is informational
    // as it is just as visible.
    Warning,

    // Highest level of logging.  If any logging is going on it will include this
    // message.  For this reason, please avoid using the Error level unless the
    // message should be displayed absolutely everywhere.
    Error,

    // Number of error levels
    Count, // Used to static assert where updates must happen in code.
};


//-----------------------------------------------------------------------------
// Line of Code
//
// C++11 Trait that can be used to insert the file and line number into a log
// message.  Often times it is a good idea to put these at the start of a log
// message so that the user can click on them to have the code editor IDE jump
// to that line of code.

// Log Line of Code FileLineInfo object.
#if defined(LOGGING_DEBUG)
    #define LOGGING_FILE_LINE_STRING_ __FILE__ "(" LOGGING_STRINGIZE(__LINE__) ")"
    #define LOGGING_LOC LOGGING_FILE_LINE_STRING_
#else
    #define LOGGING_LOC "(no LOC)"
#endif

//-----------------------------------------------------------------------------
// Name
//
// A fixed length name which avoids allocation and is safe to share across DLL
// boundaries.

class Name;

class Name
{
public:
    // Longest string not including '\0' termination
    static const size_t MaxLength = 63;

    Name(const char* init)
    {
        // Maximum portability vs. ::strncpy_s
        for (size_t i = 0; i < MaxLength; ++i)
        {
            if ((name[i] = init[i]) == '\0')
                return;
        }
        name[MaxLength] = '\0';
    }
    Name(std::string init) : Name(init.c_str())
    {
    }
    const char* Get() const
    {
        return name.data();
    }
    int cmp(const Name& rhs)
    {
        // Maximum portability vs. std::strncmp
        int diff = 0;
        for (size_t i = 0; i < MaxLength; ++i)
        {
            // this < rhs => (-), this > rhs => (+)
            diff = int(name[i]) - int(rhs.name[i]);
            // If strings are equal it is sufficent to terminate on either '\0'
            if ((diff != 0) || (name[i] == '\0')) // => (rhs.name[i] == '\0')
                break;
        }
        return diff;
    }
    bool operator==(const Name& rhs) { return cmp(rhs) == 0; }
    bool operator!=(const Name& rhs) { return cmp(rhs) != 0; }

private:
    // '\0'-terminated 
    std::array<char, MaxLength + 1> name;
};

//-----------------------------------------------------------------------------
// LogStringBuffer
//
// Thread-local buffer for constructing a log message.

struct LogStringBuffer
{
    const Name SubsystemName;
    const Level MessageLogLevel;

    // Buffer containing string as it is constructed
    std::stringstream Stream;
    // TBD: We can optimize this better than std::string
    // TBD: We can remember the last log string size to avoid extra allocations.

    // Flag indicating that the message is being relogged.
    // This is useful to prevent double-logging messages.
    bool Relogged;

    // Ctor
    LogStringBuffer(const char* subsystem, Level level) :
        SubsystemName(subsystem),
        MessageLogLevel(level),
        Stream(),
        Relogged(false)
    {
    }
};


//-----------------------------------------------------------------------------
// LogStringize Override
//
// This is the function that user code can override to control how special types
// are serialized into the log messages.

// Delete logging a shared_ptr
template<typename T>
LOGGING_INLINE void LogStringize(LogStringBuffer& buffer, const std::shared_ptr<T>& thing) {
  (void)buffer;
  (void)thing;
#if !defined(__clang__)
  static_assert(false, "Don't log a shared_ptr, log *ptr (or ptr.get() for the raw pointer value).");
#endif
}

// Delete logging a unique_ptr
template<typename T, typename Deleter>
LOGGING_INLINE void LogStringize(LogStringBuffer& buffer, const std::unique_ptr<T, Deleter>& thing) {
  (void)buffer;
  (void)thing;
#if !defined(__clang__)
  static_assert(false, "Don't log a unique_ptr, log *ptr (or ptr.get() for the raw pointer value).");
#endif
}

template<typename T>
LOGGING_INLINE void LogStringize(LogStringBuffer& buffer, const T& first)
{
    buffer.Stream << first;
}

// Overrides for various types we want to handle specially:

template<>
LOGGING_INLINE void LogStringize(LogStringBuffer& buffer, const bool& first)
{
    buffer.Stream << (first ? "true" : "false");
}

template<>
void LogStringize(LogStringBuffer& buffer, wchar_t const * const & first);

template<int N>
LOGGING_INLINE void LogStringize(LogStringBuffer& buffer, const wchar_t(&first)[N])
{
    const wchar_t* str = first;
    LogStringize(buffer, str);
}

template<>
LOGGING_INLINE void LogStringize(LogStringBuffer& buffer, const std::wstring& first)
{
    const wchar_t* str = first.c_str();
    LogStringize(buffer, str);
}


//-----------------------------------------------------------------------------
// Log Output Worker Thread
//
// Worker thread that produces the output.
// Call AddPlugin() to register an output plugin.

// User-defined output plugin
class OutputPlugin
{
public:
    virtual ~OutputPlugin() {}

    // Return a unique string naming this output plugin.
    virtual const char* GetUniquePluginName() = 0;

    // Write data to output.
    virtual void Write(Level level, const char* subsystem, const char* header, const char* utf8msg) = 0;
};


//-----------------------------------------------------------------------------
// Used by channel to specify how to write
enum class WriteOption : Write_Option_t
{
    // Default log write
    Default,

    // Dangerously ignore the queue limit
    DangerouslyIgnoreQueueLimit
};


// Iterate through the list of channels before the CRT has initialized
#pragma pack(push,1)
struct ChannelNode 
{
    const char* SubsystemName; // This is always a pointer to a Channel's SubsystemName.Get()
    Log_Level_t* Level;
    bool* UserOverrodeMinimumOutputLevel;
    ChannelNode* Next;
    ChannelNode* Prev;
};
#pragma pack(pop)


#ifndef OVR_EXPORTED_FUNCTION
    #if defined(_WIN32)
        #define OVR_EXPORTED_FUNCTION __declspec(dllexport)
    #else
        #define OVR_EXPORTED_FUNCTION __attribute__((visibility("default")))
    #endif
#endif


// Export the function to access OutputWorker::Write(). This is used by the Channel class
// to allow writing with OutputWorker possibly in a separate module
extern "C"
{
    OVR_EXPORTED_FUNCTION extern void OutputWorkerOutputFunctionC(const char* subsystemName, Log_Level_t messageLogLevel, const char* stream, bool relogged, Write_Option_t option);
    typedef void(*OutputWorkerOutputFunctionType)(const char* subsystemName, Log_Level_t messageLogLevel, const char* stream, bool relogged, Write_Option_t option);

    OVR_EXPORTED_FUNCTION extern void ConfiguratorOnChannelLevelChangeC(const char* channelName, Log_Level_t minimumOutputLevel);
    typedef void(*ConfiguratorOnChannelLevelChangeType)(const char* channelName, Log_Level_t minimumOutputLevel);

    OVR_EXPORTED_FUNCTION extern void ConfiguratorRegisterC(ChannelNode* channelNode);
    typedef void(*ConfiguratorRegisterType)(ChannelNode* channelNode);

    OVR_EXPORTED_FUNCTION extern void ConfiguratorUnregisterC(ChannelNode* channelNode);
    typedef void(*ConfiguratorUnregisterType)(ChannelNode* channelNode);
}

// Shutdown the logging system and release memory
void ShutdownLogging();

// Restart the logging system
void RestartLogging();

// Log Output Worker Thread
class OutputWorker
{
    OutputWorker(); // Use GetInstance() to get the singleton instance.

public:
    static OutputWorker* GetInstance();

    ~OutputWorker();

    void InstallDefaultOutputPlugins();

    // Start/stop logging output (started automatically)
    void Start();
    void Stop();

    // Blocks until all log messages before this function call are completed.
    void Flush();

    // Write a log buffer to the output
    void Write(const char* subsystemName, Level messageLogLevel, const char* stream, bool relogged, WriteOption option);

    // Plugin management
    void AddPlugin(std::shared_ptr<OutputPlugin> plugin);
    void RemovePlugin(std::shared_ptr<OutputPlugin> plugin);
    std::shared_ptr<OutputPlugin> GetPlugin(const char* const pluginName);

    // Disable all output
    void DisableAllPlugins();

    // Get the lock used for the channels.
    Lock* GetChannelsLock();

    // Our time type
    #if defined(WIN32)
        typedef SYSTEMTIME LogTime;
    #else
        typedef time_t LogTime;  // To do: Make this a C++ time with better resolution than time_t.
    #endif
    
    static LogTime GetCurrentLogTime();
    
private:
    // Is the logger running in a debugger?
    bool IsInDebugger;

    // It's here so so we know it is valid in the scope of ~OutputWorker
    Lock ChannelsLock;

    // Plugins
    Lock PluginsLock;
    std::set< std::shared_ptr<OutputPlugin> > Plugins;

    // Worker Log Buffer
    struct QueuedLogMessage
    {
        const Name        SubsystemName;
        const Level       MessageLogLevel;
        std::string       Buffer;
        LogTime           Time;
        QueuedLogMessage* Next;
        OvrLogHandle      FlushEvent;

        QueuedLogMessage(const char* subsystemName, Level messageLogLevel, const char* stream, const LogTime& time);
    };

    // Maximum number of logs that we allow in the queue at a time.
    // If we go beyond this limit, we keep a count of additional logs that were lost.
    static const int WorkQueueLimit = 1000;

    AutoHandle        WorkerWakeEvent;      // Event letting the worker thread know the queue is not empty
    Lock              WorkQueueLock;        // Lock guarding the work queue
    QueuedLogMessage* WorkQueueHead;        // Head of linked list of work that is queued
    QueuedLogMessage* WorkQueueTail;        // Tail of linked list of work that is queued
    int               WorkQueueSize;        // Size of the linked list of queued work
    int               WorkQueueOverrun;     // Number of log messages that exceeded the limit
    // The work queue size is used to avoid overwhelming the logging thread, since it takes 1-2 milliseconds
    // to log out each message it can easily fall behind a large amount of logs.  Lost log messages are added
    // to the WorkQueueOverrun count so that they can be reported as "X logs were lost".

    inline void WorkQueueAdd(QueuedLogMessage* msg)
    {
        if (WorkQueueTail)
        {
            WorkQueueTail->Next = msg;
        }
        else
        {
            WorkQueueHead = msg;
        }
        WorkQueueTail = msg;
        ++WorkQueueSize;
    }

    #if defined(_WIN32)
        #define OVR_THREAD_FUNCTION_TYPE DWORD WINAPI
    #else
        #define OVR_THREAD_FUNCTION_TYPE uint32_t
    #endif
    
    static OVR_THREAD_FUNCTION_TYPE WorkerThreadEntrypoint_(void* worker);
    
    void WorkerThreadEntrypoint();

    Lock StartStopLock;
    Terminator WorkerTerminator;
    AutoHandle LoggingThread;

    // Append level and subsystem name to timestamp buffer
    // The buffer should point to the ending null terminator of
    // the timstamp string.
    static void AppendHeader(char* buffer, size_t bufferBytes,
                             Level level, const char* subsystemName);

    void ProcessQueuedMessages();

    void FlushDbgViewLogImmediately(const char* subsystemName, Level messageLogLevel, const char* stream);
};

//-----------------------------------------------------------------------------
// ErrorSilencer
//
// This will demote errors to warnings in the log until it goes out of scope.
// Helper class that allows error silencing to be done several function calls
// up the stack and checked down the stack.
class ErrorSilencer
{
public:
    // Returns a bitfield of SilenceOptions that are currently in effect
    static int GetSilenceOptions();

    // Start silencing errors.
    ErrorSilencer(int options = DemoteErrorsToWarnings);

    enum SilenceOptions
    {
        // Error logs will be demoted to the warning log level
        DemoteErrorsToWarnings = 1,

        // All Log* methods will be silenced
        CompletelySilenceLogs = 2,

        // OVR::MakeError will not assert when errors are set
        PreventErrorAsserts = 4,

        // All logs at a level > Debug will be set to Debug level
        DemoteToDebug = 8
    };

    // Stop silencing errors.
    ~ErrorSilencer();

private:
    // Start silencing errors.  This is done automatically be the constructor.
    void Silence();

    // Stop silencing errors.  This is done automatically be the deconstructor.
    void Unsilence();

    int Options = 0;
};


//-----------------------------------------------------------------------------
// Channel
//
// One named logging channel.

class Channel
{
public:
    Channel(const char* nameString);
    Channel(const Channel& other);
    ~Channel();

    const Name SubsystemName;
    // Deprecated, use SubsystemName.Get() instead
    const char* GetName() const
    {
        return SubsystemName.Get();
    }

    // Add an extra prefix to all log messages generated by the channel.
    // This function is *not* thread-safe.  Logging from another thread while changing
    // the prefix can cause crashes.
    std::string GetPrefix() const;
    void SetPrefix(const std::string& prefix);

    // Set the minimum output level permitted from this channel.
    void SetMinimumOutputLevel(Level newLevel);

    // Set the output level temporarily for this session without remembering that setting.
    void SetMinimumOutputLevelNoSave(Level newLevel);

    Level GetMinimumOutputLevel() const;

    LOGGING_INLINE bool Active(Level level) const
    {
        return MinimumOutputLevel <= (uint32_t)level;
    }

    template<typename... Args>
    LOGGING_INLINE void Log(Level level, Args&&... args) const
    {
        if (Active(level))
        {
            doLog(level, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    LOGGING_INLINE void LogError(Args&&... args) const
    {
        if (Active(Level::Error))
        {
            doLog(Level::Error, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    LOGGING_INLINE void LogWarning(Args&&... args) const
    {
        if (Active(Level::Warning))
        {
            doLog(Level::Warning, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    LOGGING_INLINE void LogInfo(Args&&... args) const
    {
        if (Active(Level::Info))
        {
            doLog(Level::Info, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    LOGGING_INLINE void LogDebug(Args&&... args) const
    {
        if (Active(Level::Debug))
        {
            doLog(Level::Debug, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    LOGGING_INLINE void LogTrace(Args&&... args) const
    {
        if (Active(Level::Trace))
        {
            doLog(Level::Trace, std::forward<Args>(args)...);
        }
    }

    // printf style log functions
    template<typename... Args>
    LOGGING_INLINE void LogF(Level level, Args&&... args) const
    {
        if (Active(level))
        {
            doLogF(level, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    LOGGING_INLINE void LogErrorF(Args&&... args) const
    {
        if (Active(Level::Error))
        {
            doLogF(Level::Error, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    LOGGING_INLINE void LogWarningF(Args&&... args) const
    {
        if (Active(Level::Warning))
        {
            doLogF(Level::Warning, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    LOGGING_INLINE void LogInfoF(Args&&... args) const
    {
        if (Active(Level::Info))
        {
            doLogF(Level::Info, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    LOGGING_INLINE void LogDebugF(Args&&... args) const
    {
        if (Active(Level::Debug))
        {
            doLogF(Level::Debug, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    LOGGING_INLINE void LogTraceF(Args&&... args) const
    {
        if (Active(Level::Trace))
        {
            doLogF(Level::Trace, std::forward<Args>(args)...);
        }
    }

    // DANGER DANGER DANGER
    // This function forces a log message to be recorded even if the log queue is full.
    // This is dangerous because the caller can run far ahead of the output writer thread
    // and cause a large amount of memory to be allocated and logging tasks can take many
    // minutes to flush afterwards.  It should only be used when the data is critical.
    template<typename... Args>
    LOGGING_INLINE void DangerousForceLog(Level level, Args&&... args) const
    {
        if (Active(level))
        {
            int silenceOptions = ErrorSilencer::GetSilenceOptions();
            if (silenceOptions & ErrorSilencer::CompletelySilenceLogs)
            {
                return;
            }

            if (level > Level::Debug && (silenceOptions & ErrorSilencer::DemoteToDebug))
            {
                // Demote to debug
                level = Level::Debug;
            }
            else if (level == Level::Error && (silenceOptions & ErrorSilencer::DemoteErrorsToWarnings))
            {
                // Demote to warning
                level = Level::Warning;
            }

            LogStringBuffer buffer(SubsystemName.Get(), level);

            writeLogBuffer(buffer, Prefix, args...);

            // Submit buffer to logging subsystem
            const std::string& tmp = buffer.Stream.str();
            OutputWorkerOutputFunction(buffer.SubsystemName.Get(), (Log_Level_t)buffer.MessageLogLevel, tmp.c_str(), buffer.Relogged, (Write_Option_t)WriteOption::DangerouslyIgnoreQueueLimit);
        }
    }
    // DANGER DANGER DANGER

private:
    //-------------------------------------------------------------------------
    // Internal Implementation

    Channel() = delete;
    Channel(Channel&& other) = delete;
    Channel& operator=(const Channel& other) = delete;
    Channel& operator=(Channel&& other) = delete;

    friend class Configurator;

    // Used to iterate through a linked list of Channel objects
    // A linked list is used to avoid CRT new / delete during startup as this is called from the constructor
    ChannelNode Node;
    void registerNode();

    // Level at which this channel will log.
    Log_Level_t MinimumOutputLevel;

    // Optional prefix
    std::string Prefix;

    // So changing Prefix is threadsafe
    mutable Lock PrefixLock;

    bool UserOverrodeMinimumOutputLevel;

    // Target of doLog function
    static OutputWorkerOutputFunctionType OutputWorkerOutputFunction;

    // Target of OnChannelLevelChange
    static ConfiguratorOnChannelLevelChangeType ConfiguratorOnChannelLevelChange;

    // Target of Register
    static ConfiguratorRegisterType ConfiguratorRegister;

    // Target of Unregister
    static ConfiguratorUnregisterType ConfiguratorUnregister;

    void GetFunctionPointers();

    template<typename T>
    LOGGING_INLINE void writeLogBuffer(LogStringBuffer& buffer, T&& arg) const
    {
        LogStringize(buffer, arg);
    }

    template<typename T, typename... Args>
    LOGGING_INLINE void writeLogBuffer(LogStringBuffer& buffer, T&& arg, Args&&... args) const
    {
        writeLogBuffer(buffer, arg);
        writeLogBuffer(buffer, args...);
    }

    // Unroll arguments
    template<typename... Args>
    LOGGING_INLINE void doLog(Level level, Args&&... args) const
    {
        int silenceOptions = ErrorSilencer::GetSilenceOptions();
        if (silenceOptions & ErrorSilencer::CompletelySilenceLogs)
        {
            return;
        }

        if (level > Level::Debug && (silenceOptions & ErrorSilencer::DemoteToDebug))
        {
            // Demote to debug
            level = Level::Debug;
        }
        else if (level == Level::Error && (silenceOptions & ErrorSilencer::DemoteErrorsToWarnings))
        {
            // Demote to warning
            level = Level::Warning;
        }

        LogStringBuffer buffer(SubsystemName.Get(), level);

        writeLogBuffer(buffer, Prefix, args...);

        // Submit buffer to logging subsystem
        const std::string& tmp = buffer.Stream.str();
        OutputWorkerOutputFunction(buffer.SubsystemName.Get(), (Log_Level_t)buffer.MessageLogLevel, tmp.c_str(), buffer.Relogged, (Write_Option_t)WriteOption::Default);
    }

    // Returns the buffer capacity required to printf the given format+arguments.
    // Returns -1 if the format is invalid.
    static int GetPrintfLengthV(const char* format, va_list argList)
    {
        int size;

    #if defined(_MSC_VER) // Microsoft doesn't support C99-Standard vsnprintf, so need to use _vscprintf.
        size = _vscprintf(format, argList); // Returns the required strlen, or -1 if format error.
    #else
        size = vsnprintf(nullptr, 0, format, argList); // Returns the required strlen, or negative if format error.
    #endif

        if (size > 0) // If we can 0-terminate the output...
            ++size; // Add one to account for terminating null.
        else
            size = -1;

        return size;
    }


    static int GetPrintfLength(const char* format, ...)
    {
        va_list argList;
        va_start(argList, format);
        int size = GetPrintfLengthV(format, argList);
        va_end(argList);
        return size;
    }


    template<typename... Args>
    LOGGING_INLINE void doLogF(Level level, Args&&... args) const
    {
        int silenceOptions = ErrorSilencer::GetSilenceOptions();
        if (silenceOptions & ErrorSilencer::CompletelySilenceLogs)
        {
            return;
        }

        if (level > Level::Debug && (silenceOptions & ErrorSilencer::DemoteToDebug))
        {
            // Demote to debug
            level = Level::Debug;
        }
        else if (level == Level::Error && (silenceOptions & ErrorSilencer::DemoteErrorsToWarnings))
        {
            // Demote to warning
            level = Level::Warning;
        }

        LogStringBuffer buffer(SubsystemName.Get(), level);

        char  logCharsLocal[1024];
        char* logChars = logCharsLocal;
        char* logCharsAllocated = nullptr;

#if defined(_MSC_VER)
        int result = _snprintf_s(logCharsLocal, sizeof(logCharsLocal), _TRUNCATE, args...);
#else
        int result = snprintf(logCharsLocal, sizeof(logCharsLocal), args...);
#endif

        if ((result < 0) || ((size_t)result >= sizeof(logCharsLocal)))
        {
            int requiredSize = GetPrintfLength(args...);

            if ((requiredSize < 0) || (requiredSize > (1024 * 1024)))
            {
                LOGGING_DEBUG_BREAK(); // This call should be converted to the new log system.
                return;
            }

            logCharsAllocated = new char[requiredSize];
            logChars = logCharsAllocated;

#if defined(_MSC_VER)
            _snprintf_s(logChars, (size_t)requiredSize, _TRUNCATE, args...);
#else
            snprintf(logChars, (size_t)requiredSize, args...);
#endif
        }

        writeLogBuffer(buffer, Prefix, logChars);

        // Submit buffer to logging subsystem
        const std::string& tmp = buffer.Stream.str();
        OutputWorkerOutputFunction(buffer.SubsystemName.Get(), (Log_Level_t)buffer.MessageLogLevel, tmp.c_str(), buffer.Relogged, (Write_Option_t)WriteOption::Default);

        delete[] logCharsAllocated;
    }
};


//-----------------------------------------------------------------------------
// Log Configurator
//
// Centralized object that can configure and enumerate all the channels.

class ConfiguratorPlugin
{
public:
    ConfiguratorPlugin();
    virtual ~ConfiguratorPlugin();

    // Modify the channel level if it is set, otherwise leave it as-is.
    virtual void RestoreChannelLevel(const char* name, Level& level) = 0;

    // Sets the channel level
    virtual void SaveChannelLevel(const char* name, Level level) = 0;
};

class Configurator
{
    friend class Channel;
    friend class OutputWorker;
    Configurator(); // Call GetInstance() to get the singleton instance.

public:
    // Get singleton instance for logging configurator
    static Configurator* GetInstance();

    ~Configurator();

    void SetGlobalMinimumLogLevel(Level level);

    inline void SilenceLogging()
    {
        // Set the minimum logging level higher than any actual message.
        SetGlobalMinimumLogLevel(Level::Count);
    }

    void SetPlugin(std::shared_ptr<ConfiguratorPlugin> plugin);

    // Get all channels - note channels do not necessarily have unique names
    void GetChannels(std::vector< std::pair<std::string, Level> > &channels);

    // Set all channels with channelName to level
    void SetChannel(std::string channelName, Level level);

    // Internal: Invoked through callbacks
    void OnChannelLevelChange(const char* channelName, Log_Level_t level);

    // Internal: Load log level for a channel from disk, set all channels with this name to this level
    void RestoreChannelLogLevel(const char* channelName);

    // Internal: Load log level for a channel from disk, set this channel to this level
    void RestoreChannelLogLevel(ChannelNode* channelNode);

    // Internal: Iterate through all channels and store them
    void RestoreAllChannelLogLevels();
private:

    void RestoreAllChannelLogLevelsNoLock();

    uint32_t GlobalMinimumLogLevel;
    std::shared_ptr<ConfiguratorPlugin> Plugin;

    void SetChannelNoLock(std::string channelName, Level level, bool overrideUser);
};


// Convenience function: ovrlog::Flush();
inline void Flush()
{
  OutputWorker::GetInstance()->Flush();
}

} // namespace ovrlog
