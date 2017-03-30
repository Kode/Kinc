/************************************************************************************

Filename    :   OVR_DebugHelp.h
Content     :   Platform-independent exception handling interface
Created     :   October 6, 2014

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#ifndef OVR_DebugHelp_h
#define OVR_DebugHelp_h

#include "OVR_Types.h"
#include "OVR_String.h"
#include "OVR_Threads.h"
#include "OVR_Atomic.h"
#include "OVR_Nullptr.h"
#include "OVR_System.h"

#include <stdio.h>
#include <time.h>

#if defined(OVR_OS_WIN32) || defined(OVR_OS_WIN64)
    #include "OVR_Win32_IncludeWindows.h"
#elif defined(OVR_OS_APPLE)
    #include <pthread.h>
    #include <mach/thread_status.h>
    #include <mach/mach_types.h>

    extern "C" void* MachHandlerThreadFunctionStatic(void*);
    extern "C" int   catch_mach_exception_raise_state_identity_OVR(mach_port_t, mach_port_t, mach_port_t, exception_type_t, mach_exception_data_type_t*,
                                       mach_msg_type_number_t, int*, thread_state_t, mach_msg_type_number_t, thread_state_t, mach_msg_type_number_t*);
#elif defined(OVR_OS_LINUX)
    #include <pthread.h>
#endif


OVR_DISABLE_MSVC_WARNING(4351) // new behavior: elements of array will be default initialized

namespace OVR { 

    // Thread identifiers
    //typedef void*     ThreadHandle;  // Already defined by OVR Threads. Same as Windows thread handle, Unix pthread_t.
    //typedef void*     ThreadId;      // Already defined by OVR Threads. Used by Windows as DWORD thread id, by Unix as pthread_t. 
    typedef uintptr_t   ThreadSysId;   // System thread identifier. Used by Windows the same as ThreadId (DWORD), thread_act_t on Mac/BSD, lwp id on Linux.

    // Thread constants
    // To do: Move to OVR Threads
    #define OVR_THREADHANDLE_INVALID ((ThreadHandle*)nullptr)
    #define OVR_THREADID_INVALID     ((ThreadId*)nullptr)
    #define OVR_THREADSYSID_INVALID  ((uintptr_t)0)

    OVR::ThreadSysId  ConvertThreadHandleToThreadSysId(OVR::ThreadHandle threadHandle);
    OVR::ThreadHandle ConvertThreadSysIdToThreadHandle(OVR::ThreadSysId threadSysId);   // The returned handle must be freed with FreeThreadHandle.
    void              FreeThreadHandle(OVR::ThreadHandle threadHandle);                 // Frees the handle returned by ConvertThreadSysIdToThreadHandle.
    OVR::ThreadSysId  GetCurrentThreadSysId();

    void GetOSVersionName(char* versionName, size_t versionNameCapacity);

    // CPUContext
    #if defined(OVR_OS_MS)
        typedef CONTEXT CPUContext; 
    #elif defined(OVR_OS_MAC)
        struct CPUContext
        {
            x86_thread_state_t  threadState; // This works for both x86 and x64.
            x86_float_state_t   floatState;
            x86_debug_state_t   debugState;
            x86_avx_state_t     avxState;
            x86_exception_state exceptionState;
            
            CPUContext() { memset(this, 0, sizeof(CPUContext)); }
        };
    #elif defined(OVR_OS_LINUX)
        typedef int CPUContext; // To do.
    #endif


    // Tells if the current process appears to be running under a debugger. Does not attempt to 
    // detect the case of stealth debuggers (malware-related for example).
    bool OVRIsDebuggerPresent();

    // Exits the process with the given exit code.
    #if !defined(OVR_NORETURN)
        #if defined(OVR_CC_MSVC)
            #define OVR_NORETURN __declspec(noreturn)
        #else
            #define OVR_NORETURN __attribute__((noreturn))
        #endif
    #endif
    OVR_NORETURN void ExitProcess(intptr_t processReturnValue);

    // Returns the instruction pointer of the caller for the position right after the call.
    OVR_NO_INLINE void GetInstructionPointer(void*& pInstruction);

    // Returns the stack base and limit addresses for the given thread, or for the current thread if the threadHandle is default.
    // The stack limit is a value less than the stack base on most platforms, as stacks usually grow downward.
    // Some platforms (e.g. Microsoft) have dynamically resizing stacks, in which case the stack limit reflects the current limit.
    void GetThreadStackBounds(void*& pStackBase, void*& pStackLimit, ThreadHandle threadHandle = OVR_THREADHANDLE_INVALID);

    enum MemoryAccess
    {
        kMANone    = 0x00,
        kMARead    = 0x01,
        kMAWrite   = 0x02,
        kMAExecute = 0x04
    };

    // Returns MemoryAccess flags. Returns kMAUnknown for unknown access.
    int  GetMemoryAccess(const void* p);


    /// Used by KillCdeclFunction and RestoreCdeclFunction
    ///
    struct SavedFunction
    {
        void*   Function;
        uint8_t Size;
        uint8_t Data[15];
        void*   FunctionImplementation;  // Points to the original function, if possible to use it. 

        SavedFunction() : Function(nullptr), Size(0), FunctionImplementation(nullptr) {}

        SavedFunction(int){} // Intentionally uninitialized
    };


    /// Overwrites the implementation of a statically linked function with an implementation
    /// that unilaterally returns the given int32_t value. Works regardless of the arguments
    /// passed to that function by the caller. This version is specific to cdecl functions
    /// as opposed to Microsoft stdcall functions. Requires the ability to use VirtualProtect 
    /// to change the code memory to be writable. Returns true if the operation was successful.
    /// 
    /// Since this function overwrites the memory of the existing function implementation,
    /// it reequires the function to have at least enough bytes for this. If functionReturnValue
    /// is zero then pFunction must be at least three bytes in size. If functionReturnValue is 
    /// non-zero then pFunction must be at least six bytes in size.
    ///
    /// Example usage:
    ///    int __cdecl _CrtIsValidHeapPointer(const void* heapPtr);
    /// 
    ///    void main(int, char*[]){
    ///        KillCdeclFunction(_CrtIsValidHeapPointer, TRUE); // Make _CrtIsValidHeapPointer always return true.
    ///    }
    ///
    bool KillCdeclFunction(void* pFunction, int32_t functionReturnValue, SavedFunction* pSavedFunction = nullptr);


    /// This version is for functions that return void. It causes them to immediately return.
    ///
    /// Example usage:
    ///    void __cdecl _CrtCheckMemory();
    ///
    ///    void main(int, char*[]){
    ///        KillCdeclFunction(_CrtCheckMemory);
    ///    }
    ///
    bool KillCdeclFunction(void* pFunction, SavedFunction* pSavedFunction = nullptr);


    /// RedirectCdeclFunction
    ///
    /// Upon success, pSavedFunction is modified to contain a saved copy of the modified bytes.
    /// Upon failure, pSavedFunction is not modified.
    /// RestoreCdeclFunction can be used to restore the bytes saved by pSavedFunction.
    ///
    /// Example usage:
    ///     void* MyMalloc(size_t n)
    ///        { ... }
    ///     RedirectCdeclFunction(malloc, MyMalloc);
    ///
    bool RedirectCdeclFunction(void* pFunction, const void* pDestFunction, OVR::SavedFunction* pSavedFunction = nullptr);


    /// Restores a function which was previously killed by KillCdeclFunction.
    ///
    /// Example usage:
    ///    void main(int, char*[]){
    ///        SavedFunction savedFunction
    ///        KillCdeclFunction(_CrtCheckMemory, &savedFunction);
    ///        [...]
    ///        RestoreCdeclFunction(&savedFunction);
    ///    }
    ///
    bool RestoreCdeclFunction(SavedFunction* pSavedFunction);


    /// Smart class which temporarily kills a function and restores it upon scope completion.
    ///
    /// Example usage:
    ///    void main(int, char*[]){
    ///        TempCdeclFunctionKill tempKill(_CrtIsValidHeapPointer, TRUE);
    ///        [...]
    ///    }
    ///
    struct TempCdeclFunctionKill
    {
        TempCdeclFunctionKill(void* pFunction, int32_t functionReturnValue) : Success(false), FunctionPtr(nullptr), SavedFunctionData()
            { Success = KillCdeclFunction(pFunction, functionReturnValue, &SavedFunctionData); }

        TempCdeclFunctionKill(void* pFunction) : Success(false), FunctionPtr(nullptr), SavedFunctionData()
            { Success = KillCdeclFunction(pFunction, &SavedFunctionData); }

        ~TempCdeclFunctionKill()
            { if(Success) RestoreCdeclFunction(&SavedFunctionData); }

        bool          Success;
        void*         FunctionPtr;
        SavedFunction SavedFunctionData;
    };


    /// Class which implements copying the executable bytes of a function to a newly allocated page.
    /// This is useful for when doing some kinds of function interception and overriding at runtime.
    ///
    /// Example usage:
    ///    void main(int, char*[]){
    ///        CopiedFunction strlenCopy(strlen);
    ///        size_t n = strlen("test"); // Will execute through the newly allocated version of strlen.
    ///    }
    ///
    class CopiedFunction
    {
    public:
        CopiedFunction(const void* pFunction = nullptr, size_t size = 0);
       ~CopiedFunction();

        const void* Copy(const void* pFunction, size_t size);

        void Free();

        const void* GetFunction() const
            { return Function; }

    protected:
        const void* GetRealFunctionLocation(const void* pFunction);

        void* Function;
    };



    // OVR_MAX_PATH
    // Max file path length (for most uses).
    // To do: move this to OVR_File.
    #if !defined(OVR_MAX_PATH)
        #if defined(OVR_OS_MS)         // http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247%28v=vs.85%29.aspx
            #define OVR_MAX_PATH  260  // Windows can use paths longer than this in some cases (network paths, UNC paths).
        #else
            #define OVR_MAX_PATH 1024  // This isn't a strict limit on all Unix-based platforms.
        #endif
    #endif


    // ModuleHandle
    #if defined(OVR_OS_MS)
        typedef void* ModuleHandle;  // from LoadLibrary()
    #elif defined(OVR_OS_APPLE) || defined(OVR_OS_UNIX)
        typedef void* ModuleHandle;  // from dlopen()
    #endif

    #define OVR_MODULEHANDLE_INVALID ((ModuleHandle*)nullptr)

    // Module info constants
    static const ModuleHandle kMIHandleInvalid          = OVR_MODULEHANDLE_INVALID;
    static const uint64_t     kMIAddressInvalid         = 0xffffffffffffffffull;
    static const uint64_t     kMISizeInvalid            = 0xffffffffffffffffull;
    static const int32_t      kMILineNumberInvalid      = -1;
    static const int32_t      kMIFunctionOffsetInvalid  = -1;
    static const uint64_t     kMIBaseAddressInvalid     = 0xffffffffffffffffull;
    static const uint64_t     kMIBaseAddressUnspecified = 0xffffffffffffffffull;

    struct ModuleInfo
    {
        ModuleHandle handle;
        uint64_t     baseAddress;           // The actual runtime base address of the module. May be different from the base address specified in the debug symbol file.
        uint64_t     size;
        char         filePath[OVR_MAX_PATH];
        char         name[32];
        char         type[8];               // Unix-specific. e.g. __TEXT
        char         permissions[8];        // Unix specific. e.g. "drwxr-xr-x"

        ModuleInfo() : handle(kMIHandleInvalid), baseAddress(kMIBaseAddressInvalid), size(0), filePath(), name(){}
    };


    // Refers to symbol info for an instruction address. 
    // Info includes function name, source code file/line, and source code itself.
    struct SymbolInfo
    {
        uint64_t          address;
        uint64_t          size;
        const ModuleInfo* pModuleInfo;
        char              filePath[OVR_MAX_PATH];
        int32_t           fileLineNumber;
        char              function[384];            // This is a fixed size because we need to use it during application exceptions.
        int32_t           functionOffset;
        char              sourceCode[1024];         // This is a string representing the code itself and not a file path to the code.

        SymbolInfo() : address(kMIAddressInvalid), size(kMISizeInvalid), pModuleInfo(nullptr), filePath(), 
                        fileLineNumber(kMILineNumberInvalid), function(), functionOffset(kMIFunctionOffsetInvalid), sourceCode() {}
    };


    // Implements support for reading thread lists, module lists, backtraces, and backtrace symbols.
    class SymbolLookup
    {
    public:
        SymbolLookup();
        ~SymbolLookup();

        // Every successful call to Initialize must be eventually matched by a call to Shutdown.
        // Shutdown should be called if and only if Initialize returns true.
        static bool Initialize();

        static bool IsInitialized();

        static void Shutdown();

        void AddSourceCodeDirectory(const char* pDirectory);

        // Should be disabled when within an exception handler.
        void EnableMemoryAllocation(bool enabled);

        // Refresh our view of the symbols and modules present within the current process.
        bool Refresh();

        // Retrieves the backtrace (call stack) of the given thread. There may be some per-platform restrictions on this.
        // Returns the number written, which will be <= addressArrayCapacity.
        // This may not work on some platforms unless stack frames are enabled.
        // For Microsoft platforms the platformThreadContext is CONTEXT*.
        // For Apple platforms the platformThreadContext is x86_thread_state_t* or arm_thread_state_t*.
        // If threadSysIdHelp is non-zero, it may be used by the implementation to help produce a better backtrace.
        size_t GetBacktrace(void* addressArray[], size_t addressArrayCapacity, size_t skipCount = 0, void* platformThreadContext = nullptr, OVR::ThreadSysId threadSysIdHelp = OVR_THREADSYSID_INVALID);

        // Retrieves the backtrace for the given ThreadHandle.
        // Returns the number written, which will be <= addressArrayCapacity.
        size_t GetBacktraceFromThreadHandle(void* addressArray[], size_t addressArrayCapacity, size_t skipCount = 0, OVR::ThreadHandle threadHandle = OVR_THREADHANDLE_INVALID);

        // Retrieves the backtrace for the given ThreadSysId.
        // Returns the number written, which will be <= addressArrayCapacity.
        size_t GetBacktraceFromThreadSysId(void* addressArray[], size_t addressArrayCapacity, size_t skipCount = 0, OVR::ThreadSysId threadSysId = OVR_THREADSYSID_INVALID);

        // Gets a list of the modules (e.g. DLLs) present in the current process.
        // Writes as many ModuleInfos as possible to pModuleInfoArray.
        // Returns the required count of ModuleInfos, which will be > moduleInfoArrayCapacity if the capacity needs to be larger.
        size_t GetModuleInfoArray(ModuleInfo* pModuleInfoArray, size_t moduleInfoArrayCapacity);

        // Retrieves a list of the current threads. Unless the process is paused the list is volatile.
        // Returns the required capacity, which may be larger than threadArrayCapacity.
        // Either array can be NULL to specify that it's not written to.
        // For Windows the caller needs to CloseHandle the returned ThreadHandles. This can be done by calling DoneThreadList.
        size_t GetThreadList(ThreadHandle* threadHandleArray, ThreadSysId* threadSysIdArray, size_t threadArrayCapacity);

        // Frees any references to thread handles or ids returned by GetThreadList;
        void DoneThreadList(ThreadHandle* threadHandleArray, ThreadSysId* threadSysIdArray, size_t threadArrayCount);

        // Writes a given thread's callstack with symbols to the given output.
        // It may not be safe to call this from an exception handler, as sOutput allocates memory.
        bool ReportThreadCallstack(OVR::String& sOutput, size_t skipCount = 0, ThreadSysId threadSysId = OVR_THREADSYSID_INVALID);

        // Writes all thread's callstacks with symbols to the given output.
        // It may not be safe to call this from an exception handler, as sOutput allocates memory.
        bool ReportThreadCallstacks(OVR::String& sOutput, size_t skipCount = 0);

        // Writes all loaded modules to the given output string.
        // It may not be safe to call this from an exception handler, as sOutput allocates memory.
        bool ReportModuleInformation(OVR::String& sOutput);

        // Retrieves symbol info for the given address. 
        bool LookupSymbol(uint64_t address, SymbolInfo& symbolInfo);
        bool LookupSymbols(uint64_t* addressArray, SymbolInfo* pSymbolInfoArray, size_t arraySize);

        const ModuleInfo* GetModuleInfoForAddress(uint64_t address);  // The returned ModuleInfo points to an internal structure.

    protected:
        bool RefreshModuleList();

    protected:
        bool       AllowMemoryAllocation;   // True by default. If true then we allow allocating memory (and as a result provide less information). This is useful for when in an exception handler.
        bool       ModuleListUpdated;
        ModuleInfo ModuleInfoArray[256];     // Cached list of modules we use. This is a fixed size because we need to use it during application exceptions.
        size_t     ModuleInfoArraySize;
    };



    // ExceptionInfo
    // We need to be careful to avoid data types that can allocate memory while we are 
    // handling an exception, as the memory system may be corrupted at that point in time.
    struct ExceptionInfo
    {
        tm            time;                             // GM time.
        time_t        timeVal;                          // GM time_t (seconds since 1970).
        void*         backtrace[64];
        size_t        backtraceCount;
        ThreadHandle  threadHandle;                     // 
        ThreadSysId   threadSysId;                      // 
        char          threadName[32];                   // Cannot be an allocating String object.
        void*         pExceptionInstructionAddress;
        void*         pExceptionMemoryAddress;
        CPUContext    cpuContext;
        char          exceptionDescription[1024];       // Cannot be an allocating String object.
        SymbolInfo    symbolInfo;                       // SymbolInfo for the exception location.

        #if defined(OVR_OS_MS)
            EXCEPTION_RECORD exceptionRecord;           // This is a Windows SDK struct.
        #elif defined(OVR_OS_APPLE)
            uint64_t         exceptionType;             // e.g. EXC_BAD_INSTRUCTION, EXC_BAD_ACCESS, etc.
            uint32_t         cpuExceptionId;            // The x86/x64 CPU trap id.
            uint32_t         cpuExceptionIdError;       // The x86/x64 CPU trap id extra info.
            int64_t          machExceptionData[4];      // Kernel exception code info.
            int              machExceptionDataCount;    // Count of valid entries.
        #endif

        ExceptionInfo();
    };

    // Implments support for asynchronous exception handling and basic exception report generation.
    // If you are implementing exception handling for a commercial application and want auto-uploading
    // functionality you may want to consider using something like Google Breakpad. This exception handler
    // is for in-application debug/diagnostic services, though it can write a report that has similar
    // information to Breakpad or OS-provided reports such as Apple .crash files.
    //
    // Example usage:
    //     ExceptionHandler exceptionHandler;
    //
    //     int main(int, char**)
    //     {
    //          exceptionHandler.Enable(true);
    //          exceptionHandler.SetExceptionListener(pSomeListener, 0);  // Optional listener hook.
    //     }
    // 
    class ExceptionHandler
    {
    public:
        ExceptionHandler();
       ~ExceptionHandler();

        // Enables or disables handling by installing or uninstalling an exception handler.
        // If you merely want to temporarily pause handling then it may be better to cause PauseHandling,
        // as that's a lighter weight solution.
        bool Enable(bool enable);
        
        // Pauses handling. Exceptions will be caught but immediately passed on to the next handler
        // without taking any action. Pauses are additive and calls to Pause(true) must be eventually
        // matched to Pause(false). This function can be called from multiple threads simultaneously.
        // Returns the new pause count.
        int PauseHandling(bool pause);

        // Some report info can be considered private information of the user, such as the current process list,
        // computer name, IP address or other identifying info, etc. We should not report this information for
        // external users unless they agree to this.
        void EnableReportPrivacy(bool enable);

        struct ExceptionListener
        {
            virtual ~ExceptionListener(){}
            virtual int HandleException(uintptr_t userValue, ExceptionHandler* pExceptionHandler, ExceptionInfo* pExceptionInfo, const char* reportFilePath) = 0;
        };

        void SetExceptionListener(ExceptionListener* pExceptionListener, uintptr_t userValue);

        // What we do after handling the exception.
        enum ExceptionResponse
        {
            kERContinue,    // Continue execution. Will result in the exception being re-generated unless the application has fixed the cause. Similar to Windows EXCEPTION_CONTINUE_EXECUTION.
            kERHandle,      // Causes the OS to handle the exception as it normally would. Similar to Windows EXCEPTION_EXECUTE_HANDLER.
            kERTerminate,   // Exit the application.
            kERThrow,       // Re-throw the exception. Other handlers may catch it. Similar to Windows EXCEPTION_CONTINUE_SEARCH.
            kERDefault      // Usually set to kERTerminate.
        };

        void SetExceptionResponse(ExceptionResponse er)
            { exceptionResponse = er; }

        // Allws you to add an arbitrary description of the current application, which will be added to exception reports.
        void SetAppDescription(const char* appDescription);

        // Specifies how much info the minidump has, but also how large it gets.
        enum MinidumpInfoLevel
        {
            kMILNone,       // Don't generate a .mdmp file.
            kMILSmall,      // Will result in a .mdmp file that's about 10-30 KiB
            kMILMedium,     // Will result in a .mdmp file that's about 5-15 MiB
            kMILLarge       // Will result in a .mdmp file that's about 50-150 MiB.
        };


        void SetMinidumpInfoLevel(MinidumpInfoLevel level)
            { minidumpInfoLevel = level; }

        MinidumpInfoLevel GetMinidumpInfoLevel() const
            { return minidumpInfoLevel; }

        // If the report path has a "%s" in its name, then assume the path is a sprintf format and write it 
        // with the %s specified as a date/time string.
        // The report path can be "default" to signify that you want to use the default user location.
        // Passing NULL for exceptionReportPath or exceptionMinidumpPath causes those files to not be written.
        // By default both the exceptionReportPath and exceptionMinidumpPath are NULL.
        // Example usage:
        //     handler.SetExceptionPaths("/Users/Current/Exceptions/Exception %s.txt");
        void SetExceptionPaths(const char* exceptionReportPath, const char* exceptionMinidumpPath = nullptr);

        // Calls SetExceptionPaths in the appropriate convention for each operating system
        // Windows: %AppData%\Organization\Application
        // Mac: ~/Library/Logs/DiagnosticReports/Organization/Application"
        // Linux: ~/Library/Organization/Application
        // exceptionFormat and minidumpFormat define the file names in the format above
        // with the %s specified as a date/time string.
        void SetPathsFromNames(const char* organizationName, const char* ApplicationName, const char* exceptionFormat = "Exception Report (%s).txt", const char* minidumpFormat = "Exception Minidump (%s).mdmp");

        // Allows you to specify base directories for code paths, which can be used to associate exception addresses to lines 
        // of code as opposed to just file names and line numbers, or function names plus binary offsets.
        void SetCodeBaseDirectoryPaths(const char* codeBaseDirectoryPathArray[], size_t codeBaseDirectoryPathCount);

        // Writes lines into the current report. 
        void WriteReportLine(const char* pLine);
        void WriteReportLineF(const char* format, ...);

        // Retrieves a directory path which ends with a path separator.
        // Returns the required strlen of the path.
        // Guarantees the presence of the directory upon returning true.
        static size_t GetCrashDumpDirectory(char* directoryPath, size_t directoryPathCapacity);


        // Retrieves a directory path for a specific organization and application.
        // Returns the required strlen of the path.
        // Guarantees the presence of the directory upon returning true.
        static void GetCrashDumpDirectoryFromNames(char* path, const char* organizationName, const char* ApplicationName);

        // Given an exception report at a given file path, returns a string suitable for displaying in a message
        // box or similar user interface during the handling of an exception. The returned string must be passed
        // to FreeMessageBoxText when complete.
        static const char* GetExceptionUIText(const char* exceptionReportPath);
        static void FreeExceptionUIText(const char* messageBoxText);

        // Writes a deadlock report similar to an exception report.  Since there is no allocation risk, an exception handler is created for this log.
        static void ReportDeadlock(const char* threadName, const char* organizationName = nullptr, const char* applicationName = nullptr);

    protected:
        // Write one log line to log file, console, syslog, and debug output.
        // If LogFile is null, it will not write to the log file.
        // The buffer must be null-terminated.
        void writeLogLine(const char* buffer, int length);

        void WriteExceptionDescription();
        void WriteReport(const char* reportType);
        void WriteThreadCallstack(ThreadHandle threadHandle, ThreadSysId threadSysId, const char* additionalInfo);
        void WriteMiniDump();

protected:
        // Runtime constants
        bool                enabled;
        std::atomic<int>    pauseCount = { 0 };          // 0 means unpaused. 1+ means paused.
        bool                reportPrivacyEnabled;        // Defaults to true.
        ExceptionResponse   exceptionResponse;           // Defaults to kERHandle
        ExceptionListener*  exceptionListener;
        uintptr_t           exceptionListenerUserValue;
        String              appDescription;
        String              codeBasePathArray[6];        // 6 is arbitrary.
        char                reportFilePath[OVR_MAX_PATH];// May be an encoded path, in that it has "%s" in it or is named "default". See reporFiletPathActual for the runtime actual report path.
        MinidumpInfoLevel   minidumpInfoLevel;
        char                miniDumpFilePath[OVR_MAX_PATH];
        FILE*               LogFile;                        // Can/should we use OVR Files for this?
        char                scratchBuffer[4096];

        // Runtime variables
        bool                     exceptionOccurred;
        std::atomic<uint32_t> handlingBusy = { 0 };
        char                     reportFilePathActual[OVR_MAX_PATH];
        char                     minidumpFilePathActual[OVR_MAX_PATH];
        int                      terminateReturnValue;
        ExceptionInfo            exceptionInfo;
        SymbolLookup             symbolLookup;

        #if defined(OVR_OS_MS)
            void*                        vectoredHandle;
            LPTOP_LEVEL_EXCEPTION_FILTER previousFilter;
            LPEXCEPTION_POINTERS         pExceptionPointers;

            friend LONG WINAPI Win32ExceptionFilter(LPEXCEPTION_POINTERS pExceptionPointers);
            LONG ExceptionFilter(LPEXCEPTION_POINTERS pExceptionPointers);

            //handles exception in a new thread, used for stack overflow
            static unsigned WINAPI ExceptionHandlerThreadExec(void * callingHandler);
        #elif defined(OVR_OS_APPLE)
            struct SavedExceptionPorts
            {
                SavedExceptionPorts() : count(0) { memset(this, 0, sizeof(SavedExceptionPorts)); }

                mach_msg_type_number_t count;
                exception_mask_t       masks[6];
                exception_handler_t    ports[6];
                exception_behavior_t   behaviors[6];
                thread_state_flavor_t  flavors[6];
            };

            friend void* ::MachHandlerThreadFunctionStatic(void*);
            friend int ::catch_mach_exception_raise_state_identity_OVR(mach_port_t, mach_port_t, mach_port_t, exception_type_t,
                                        mach_exception_data_type_t*, mach_msg_type_number_t, int*, thread_state_t,
                                        mach_msg_type_number_t, thread_state_t, mach_msg_type_number_t*);
        
            bool          InitMachExceptionHandler();
            void          ShutdownMachExceptionHandler();
            void*         MachHandlerThreadFunction();
            kern_return_t HandleMachException(mach_port_t port, mach_port_t thread, mach_port_t task, exception_type_t exceptionType,
                                              mach_exception_data_type_t* pExceptionDetail, mach_msg_type_number_t exceptionDetailCount, 
                                              int* pFlavor, thread_state_t pOldState, mach_msg_type_number_t oldStateCount, thread_state_t pNewState,
                                              mach_msg_type_number_t* pNewStateCount);
            kern_return_t ForwardMachException(mach_port_t thread, mach_port_t task, exception_type_t exceptionType,
                                               mach_exception_data_t pExceptionDetail, mach_msg_type_number_t exceptionDetailCount);

            bool                machHandlerInitialized;
            mach_port_t         machExceptionPort;
            SavedExceptionPorts machExceptionPortsSaved;
            volatile bool       machThreadShouldContinue;
            volatile bool       machThreadExecuting;
            pthread_t           machThread;

        #elif defined(OVR_OS_LINUX)
            // To do.
        #endif
    };


    // Identifies basic exception types for the CreateException function.
    enum CreateExceptionType
    {
        kCETAccessViolation,      // Read or write to inaccessable memory.
        kCETAlignment,            // Misaligned read or write.
        kCETDivideByZero,         // Integer divide by zero.
        kCETFPU,                  // Floating point / VPU exception.
        kCETIllegalInstruction,   // Illegal opcode.
        kCETStackCorruption,      // Stack frame was corrupted.
        kCETStackOverflow,        // Stack ran out of space, often due to infinite recursion.
        kCETTrap                  // System/OS trap (system call).
    };


    // Creates an exception of the given type, primarily for testing.
    void CreateException(CreateExceptionType exceptionType);


    //-----------------------------------------------------------------------------
    // GUI Exception Listener
    //
    // When this exception handler is called, it will verify that the application
    // is not being debugged at that instant.  If not, then it will present a GUI
    // to the user containing error information.
    // Initially the exception listener is not silenced.
    class GUIExceptionListener : public ExceptionHandler::ExceptionListener
    {
    public:
        GUIExceptionListener();

        virtual int HandleException(uintptr_t userValue,
                                    ExceptionHandler* pExceptionHandler,
                                    ExceptionInfo* pExceptionInfo,
                                    const char* reportFilePath) OVR_OVERRIDE;

    protected:
        ExceptionHandler Handler;
    };


} // namespace OVR


OVR_RESTORE_MSVC_WARNING()


#endif // OVR_DebugHelp_h
