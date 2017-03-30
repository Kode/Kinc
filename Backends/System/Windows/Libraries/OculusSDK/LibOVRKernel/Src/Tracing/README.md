#Setup

If you want stack walking to work on x64:

    > reg add "HKLM\System\CurrentControlSet\Control\Session Manager\Memory Management" -v DisablePagingExecutive -d 0x1 -t REG_DWORD -f

Add USERS Read & Execute privileges to the folder (or one of its parents) containing the LibOVREvents.man file:

    > icacls . /grant BUILTIN\Users:(OI)(CI)(RX)

To install or reinstall the ETW manifest after building LibOVR run `install.cmd` as Administrator:

    > install

Note: the install script will also attempt to install the manifests for the driver and runtime. Also note that the install
script installs the manifest from the newest version of LibOVR.dll, which might not be the version you are debugging in
Visual Studio (this will only matter if the two versions have specified different events). To be safe make sure your build
is up-to-date.

#Adding trace points

See [./Tracing.h] and the examples in [../OVR_CAPI.cpp].

The following macros can be used to trace call/return and progress through a function:

    TraceCall(frameIndex)
    TraceReturn(frameIndex)
    TraceWaypoint(frameIndex)

Try to place the Call/Return instrumentation as close as possible to the function entry/exit points, and don't forget
to instrument all return paths.

Supply a frame index of 0 if a frame index is not applicable/available.

#Adding new trace events

Use the `ECManGen.exe` utility from the Windows 8.1 SDK to edit the `LibOVREvents.man` manifest.

See [http://msdn.microsoft.com/en-us/library/windows/desktop/dd996930%28v=vs.85%29.aspx]
The `F1` help is also useful.

#Rebuilding the ETW headers and resources

Use the `build.cmd` script to regenerate the `LibOVREvents.h`, `LibOVREvents.rc` and `LibOVREvents*.bin` files.
`clean.cmd` will remove all generated files.

Note that the outputs are checked into the repository so you'll need to `p4 edit` them first.

#Capturing ETW traces

See [../../../Tools/XPerf/README.md]

#Viewing ETW traces with GPUView

See [http://msdn.microsoft.com/en-us/library/windows/desktop/jj585574(v=vs.85).aspx]
