# BAS Amalgamated
### Barracuda App Server Amalgamated

<img align="left" width="200" height="200" alt="Barracuda App Server Logo" src="https://realtimelogic.com/ba/doc/logo.svg">

The Barracuda App Server is an embeddable C code library with support for the following platforms: Embedded Linux, Zephyr, FreeRTOS+TCP or lwIP, VxWorks, QNX, INTEGRITY, Windows & Win CE, Azure RTOS, Nucleus RTOS, embOS, RTXC, SMX, MQX, RTOS-32, Mac, and UNIX/POSIX.

Check out the tutorial [Embedded Web Server vs. Embedded Application Server](https://realtimelogic.com/products/web-server-vs-application-server/) if you are new to application server technology.

![Barracuda App Server Amalgamated](https://realtimelogic.com/GZ/images/BarracudaAppServerBLK.svg)

The Barracuda App Server (BAS) runs on anything from tiny FPGA's to online cloud servers. Refer to the [Barracuda App Server's Product Page](https://realtimelogic.com/products/barracuda-application-server/) for details.

[Lua Server Pages](https://realtimelogic.com/products/lua-server-pages/) (LSP for short) is a [Lua](https://www.lua.org/about.html) API that includes a compact and efficient Lua web framework, non-blocking asynchronous sockets, and many (I)IoT protocols.

## Components

BAS Amalgamated is a compact version of BAS, extracted from the BAS SDK. BAS Amalgamated includes all APIs found in the BAS SDK, but with a limited set of examples and tools. BAS Amalgamated includes the following source code components.  

* BAS.c: Platform independent code (amalgamation of many files)
* ThreadLib.c: Kernel porting layer
* SoDisp.c: TCP/IP porting layer (the socket dispatcher)
* BaFile.c: File system porting layer (optional)
* BWS.c: The [Barracuda Embedded Web Server](https://realtimelogic.com/products/barracuda-web-server/) is also included and the [C++ WebSocket example project](examples/C-WebSockets/README.md) is configured to link with BWS.c. The [SharkSSL ESP32 IDE](https://realtimelogic.com/downloads/sharkssl/ESP32/) includes BWS and the WebSocket example.

## Who Can Use BAS Amalgamated

BAS Amalgamated runs efficiently on a Cortex M4 running @100Mhz and up; however, most microcontrollers will need external memory. See the Memory section in the [Porting Barracuda to an Embedded System](https://realtimelogic.com/ba/doc/?url=introduction.html#porting) for details.

## Before Downloading the Source Code

The Barracuda App Server is a feature-rich product. To ease into its diverse options, you might find it helpful to start by downloading one of the [pre-compiled binaries with included Lua tutorials](https://realtimelogic.com/downloads/bas/), which are complemented with handy Lua tutorials, prior to accessing the source code.

## How To Compile

To compile BAS, include BAS.c, src/arch/XXX/ThreadLib.c, src/arch/NET/XXX/SoDisp.c, and optionally src/DiskIo/XXX/BaFile.c, where XXX is the required porting layer. In addition, the compiler's include path must include inc/arch/XXX, inc/arch/NET/XXX, and optionally inc/DiskIo/XXX

BAS is extremely flexible and can, for example, be used as follows:
* Compiled and integrated with RTOS powered monolithic firmware
* Compiled and run as a process on a High Level Operating System (HLOS) such as embedded Linux
* Compiled and integrated with a standard (desktop) application

BAS Amalgamated is very easy to compile and several command line compilation examples are shown below. You can add the code to any IDE and Makefile system.

## BAS Amalgamated Examples

BAS Amalgamated includes three examples from the BAS SDK: the [Mako Server](#mako-server-hlos), [Xedge](#xedge-rtos), and the [C++ WebSocket Server Example](examples/C-WebSockets/README.md).

* The Mako Server is designed for HLOS, and Xedge is designed for RTOS. Both examples provide a [Lua foundation](https://realtimelogic.com/products/lua-server-pages/) enabling rapid interactive development of web, IoT, and business logic (high level logic).
* The C++ WebSocket Server Example shows how to implement everything using C code (no Lua). With this example, you are effectively using the [Barracuda Embedded Web Server](https://realtimelogic.com/products/barracuda-web-server/) component.

### Mako Server (HLOS)

The [Mako Server Example](https://realtimelogic.com/ba/doc/?url=Mako.html) can be compiled for Windows, (embedded) Linux, QNX, and [VxWorks](#vxworks).

#### Windows:

Use the Visual Studio project file found in: BAS/examples/MakoServer/VcMake; however, make sure to download SQLite or disable the SQLite build in the project file. 

#### Linux:

See the tutorial [Embedded Linux Web Interface Design](https://makoserver.net/articles/Expedite-Your-Embedded-Linux-Web-Interface-Design) for details; however, the server can be downloaded and compiled for Linux as follows:

```
wget -O - https://raw.githubusercontent.com/RealTimeLogic/BAS/main/LinuxBuild.sh | bash
```
Download and cross-compile the code for (embedded) Linux as follows:

```
export CC=/opt/gcc-linaro-7.1.1-2017.08-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
wget -O - https://raw.githubusercontent.com/RealTimeLogic/BAS/main/LinuxBuild.sh | bash
Change the above CC environment variable to your cross-compiler.
```

#### Details:

The following example builds the Mako Server without SQLite for HLOS. You can copy and paste the command into a Linux shell. Note that you can also download pre-compiled Mako Server versions for many operating systems on the [Mako Server Web Site](https://makoserver.net/).

```
gcc -o examples/MakoServer/mako -fmerge-all-constants -O3 -Os\
    -DUSE_EMBEDDED_ZIP=0 -DBA_FILESIZE64 -DLUA_USE_LINUX -DMAKO -DUSE_SQL=0\
    -Iinc -Iinc/arch/Posix -Iinc/arch/NET/Posix\
    src/BAS.c\
    src/arch/Posix/ThreadLib.c src/arch/NET/generic/SoDisp.c src/DiskIo/posix/BaFile.c\
    examples/MakoServer/src/MakoMain.c\
    -lpthread -lm -ldl
```

The above command works for any HLOS with a GCC compiler (and derivatives), including cross compiling for embedded Linux, compiling for QNX, and compiling for VxWorks 7. Simply replace 'gcc' with the applicable compiler. Note that VxWorks is a cross between HLOS and RTOS; thus, you may consider using Xedge instead (details below). BAS supports VxWorks 5 and up.

See the [Mako Server's Build Documentation](https://realtimelogic.com/ba/doc/?url=MakoServer/readme.html) for details on macros and other important information, but note that the amalgamated version in the standard SDK works differently.

After running the above compilation command and after [building mako.zip using the mako script](https://github.com/RealTimeLogic/BAS-Resources/tree/main/build), start the server as follows:
```
$examples/MakoServer/mako

Mako Server. Version 3.7
BAS lib 4920. Build date: Dec  2 2021
Copyright (c) Real Time Logic.

Mounting /tmp/BAS/examples/MakoServer/mako.zip
Server listening on IPv6 port 9357
Server listening on IPv4 port 9357
Loading certificate MakoServer
SharkSSL server listening on IPv6 port 9443
SharkSSL server listening on IPv4 port 9443
```

**Compiling for Windows**

The following example compiles and links the server as a 64 bit Windows application using the Visual Studio command line compiler (cl). Note that we include two additional C files. These two C files make it possible to run the server as a Windows service. Note that the [pre-compiled Mako Server for Windows](https://makoserver.net/download/windows/) is compiled as a 32 bit application.

```
cl /O2^
 /DUSE_EMBEDDED_ZIP=0 /DBA_FILESIZE64 /DMAKO /DUSE_SQL=0^
 /Iinc /Iinc/arch/Windows /Iinc/arch/NET/Windows^
 src/BAS.c^
 src/arch/Windows/ThreadLib.c src/arch/NET/generic/SoDisp.c src/DiskIo/windows/BaFile.c^
 examples/MakoServer/src/MakoMain.c^
 examples/MakoServer/src/Windows/MakoWinMain.c examples/MakoServer/src/Windows/servutil.c^
 ws2_32.lib kernel32.lib User32.lib Gdi32.lib advapi32.lib ole32.lib oleaut32.lib shell32.lib^
 /link /machine:X64 /OUT:examples/MakoServer/mako.exe
```

The above command requires setting up the [command line for 64 bit compilation](https://docs.microsoft.com/en-us/cpp/build/how-to-enable-a-64-bit-visual-cpp-toolset-on-the-command-line?view=msvc-170). The above can be copied and pasted "as is" into a command window.

**Including SQLite in the Build**

The Mako Server can optionally be linked with the SQLite database engine. The following command shows how to include SQLite and activate the [Lua SQLite bindings](https://realtimelogic.com/ba/doc/?url=lua/luasql.html). However, before running the compilation command, [download SQLite](https://www.sqlite.org/download.html) and copy sqlite3.c and sqlite3.h to the BAS/src directory.

```
gcc -o examples/MakoServer/mako -fmerge-all-constants -O3 -Os\
    -DUSE_EMBEDDED_ZIP=0 -DBA_FILESIZE64 -DLUA_USE_LINUX -DMAKO\
    -Iinc -Iinc/arch/Posix -Iinc/arch/NET/Posix\
    src/BAS.c\
    src/arch/Posix/ThreadLib.c src/arch/NET/generic/SoDisp.c src/DiskIo/posix/BaFile.c\
    examples/MakoServer/src/MakoMain.c\
    src/ls_sqlite3.c src/luasql.c src/sqlite3.c\
    -lpthread -lm -ldl
```

### Xedge (RTOS)

The Xedge turns the Barracuda App Server into an interactive development tool. See the [Online Xedge Documentation](https://realtimelogic.com/ba/doc/?url=xedge/readme.html) for details.

![Xedge](https://realtimelogic.com/images/xedge/v1/Xedge.png)

Xedge is designed for RTOS devices, but can also be compiled for non embedded as is shown in the following compile examples.


Xedge is typically run from a dedicated RTOS thread, but for HLOS we can create a startup file as follows:

```
echo "extern void barracuda(void); int main() {barracuda(); return 0;}" > main.c
```

The following example shows how to compile Xedge for HLOS and include file system support.

```
gcc -o examples/xedge/xedge -Iinc -Iinc/arch/Posix -Iinc/arch/NET/Posix\
    src/BAS.c\
    src/arch/Posix/ThreadLib.c src/arch/NET/generic/SoDisp.c src/DiskIo/posix/BaFile.c\
    main.c examples/xedge/src/xedge.c examples/xedge/src/led.c\
    examples/xedge/obj/XedgeZip.c -lpthread -lm
```

The file **XedgeZip.c** embodies Xedge's resources, transformed into a C file. You can uncover this file's resources and detailed build instructions in the [BAS-Resources Repository](https://github.com/RealTimeLogic/BAS-Resources).

If you run the server after compiling it, you will see no printouts. The server tries to open port 80 and if that fails, it tries to open port 9357. Embedded systems with a console can enable the [trace library](https://realtimelogic.com/ba/doc/en/C/reference/html/structHttpTrace.html) by providing a callback for the data being printed. You can also view the trace in a browser by navigating to http://ip-addr/rtl/tracelogger/. See the [TraceLogger Documentation](https://realtimelogic.com/ba/doc/?url=auxlua.html#tracelogger) for details and our online tutorial server for a demo: [https://tutorial.realtimelogic.com/rtl/tracelogger/](https://tutorial.realtimelogic.com/rtl/tracelogger/)

The Xedge does not require a file system, and the following example shows how to compile Xedge without including BaFile.c

```
gcc -o examples/xedge/xedge -DNO_BAIO_DISK -Iinc -Iinc/arch/Posix -Iinc/arch/NET/Posix\
    src/BAS.c
    src/arch/Posix/ThreadLib.c src/arch/NET/generic/SoDisp.c
    main.c examples/xedge/src/xedge.c examples/xedge/src/led.c\
    examples/xedge/obj/XedgeZip.c -lpthread -lm
```

**Note:** You must build XedgeZip.c using the [Xedge build scripts](https://github.com/RealTimeLogic/BAS-Resources/tree/main/build) and copy the file to the above shown directory prior to running the above command.

The macro NO_BAIO_DISK is used by Xedge's C startup code xedge.c.

**Cross Compiling Xedge for Embedded Systems**

Include the files as instructed above in your IDE or Makefile. Most embedded systems require an efficient allocator, which is included. See, for example, the [FreeRTOS Readme File](src/arch/FreeRTOS/README.txt) for how to set up all required components. Most embedded RTOSs require the same setup.

A recommendation is to initially try the server on an [ESP32 using FreeRTOS and lwIP](https://realtimelogic.com/ba/ESP32/source/GettingStarted.html) even if you plan on using another RTOS and/or device. The ESP32 is very easy to set up and is an excellent RTOS learning platform.

## Enabling Additional Features

BAS Amalgamated (BAS.c) includes features that are by default not compiled. These features can be enabled by the following macros. The macros can be enabled on any platform, including RTOS, unless stated otherwise.

* USE_DBGMON=1: Include [Lua Debugger Support](https://makoserver.net/articles/Lua-and-LSP-Debugging).
* USE_REVCON=1: Enable the reverse connection if you plan on using the connection bridge feature in [SharkTrustX](https://realtimelogic.com/products/SharkTrustX/). Note that both Xedge and the Mako Server include the [Let's Encrypt plugins acmebot and acmedns](https://realtimelogic.com/ba/doc/?url=Mako.html#acmebot).
* USE_OPCUA=1: The OPC-UA stack is implemented in Lua and can be found in mako.zip/.lua/opcua. The OPC-UA stack requires a C module. Enable this flag when compiling BAS if you plan on using OPC-UA.
* USE_FORKPTY=1: Enable the [advanced process management API](https://realtimelogic.com/ba/doc/?url=auxlua.html#forkptylib), which is available for Linux and QNX. This API is required if you plan on using the [CGI plugin](https://github.com/RealTimeLogic/LSP-Examples/tree/master/CGI) or the [web shell](https://makoserver.net/articles/Linux-Web-Shell).
* USE_REDIRECTOR=1: Enable the [Reverse Proxy](https://realtimelogic.com/ba/doc/?url=auxlua.html#reverseproxy)
* USE_UBJSON=1: Enable [Universal Binary JSON](https://realtimelogic.com/ba/doc/?url=auxlua.html#ubjson)

The following macros are required if you plan on using the Let's Encrypt plugin. The macros are pre-set for the following ports: POSIX (Linux/QNX), Windows, VxWorks, and INtime.

* SHARKSSL_ENABLE_ASN1_KEY_CREATION=1
* SHARKSSL_ENABLE_RSAKEY_CREATE=1
* SHARKSSL_ENABLE_ECCKEY_CREATE=1
* SHARKSSL_ENABLE_CSR_CREATION=1
* SHARKSSL_ENABLE_CSR_SIGNING=1

The following Mako Server specific macro enables loading [external Lua modules](https://makoserver.net/documentation/c-modules/). When not using a pre-built Mako Server and when using the source code you can instead choose to integrate additional [Lua bindings](https://realtimelogic.info/swig/) directly with your build.

* USE_LUAINTF

All porting layers, except POSIX and Windows, have the following pre-set:

* LUA_NUMBER_INTEGER=1

The above construction makes Lua use integer only and excludes floating point numbers. To re-enable floating point, compile the code with:

* LUA_NUMBER_INTEGER=0

See inc/arch/XXX/luaconf.h for details.

# Porting Layers

| OS+TCP | include | source |
| ----------- | ----------- | ----------- |
| Digi | inc/arch/Digi | src/arch/Digi/ThreadLib.c src/arch/NET/generic/SoDisp.c |
| embOS | inc/arch/NET/embOSIP inc/arch/embOS | src/arch/embOS/ThreadLib.c src/arch/NET/embOSIP/SoDisp.c |
| FreeRTOS+lwIP | inc/arch/NET/LwIP inc/arch/FreeRTOS | src/arch/FreeRTOS/ThreadLib.c src/arch/NET/LwIP/SoDisp.c |
| FreeRTOS+TCP | inc/arch/FreeRTOS | src/arch/FreeRTOS/ThreadLib.c src/arch/FreeRTOS/SoDisp.c |
| INTEGRITY | inc/arch/NET/Posix inc/arch/INTEGRITY | src/arch/INTEGRITY/ThreadLib.c src/arch/NET/generic/SoDisp.c |
| INtime | inc/arch/NET/INtime inc/arch/INtime | src/arch/INtime/ThreadLib.c src/arch/NET/generic/SoDisp.c |
| Linux+epoll | inc/arch/NET/epoll inc/arch/Posix | src/arch/Posix/ThreadLib.c src/arch/NET/epoll/SoDisp.c |
| MQX | inc/arch/NET/MQX inc/arch/MQX | src/arch/MQX/ThreadLib.c src/arch/NET/MQX/SoDisp.c |
| NuttX | inc/arch/NET/Posix inc/arch/Posix | src/arch/Posix/ThreadLib.c src/arch/NET/generic/SoDisp.c |
| Nucleus | inc/arch/NET/Nucleus inc/arch/Nucleus | src/arch/Nucleus/ThreadLib.c src/arch/NET/Nucleus/SoDisp.c |
| Posix (Linux, Mac, QNX) | inc/arch/NET/Posix inc/arch/Posix | src/arch/Posix/ThreadLib.c src/arch/NET/generic/SoDisp.c |
| Quadros | inc/arch/Quadros | src/arch/Quadros/ThreadLib.c src/arch/NET/generic/SoDisp.c |
| SMX | inc/arch/NET/SMX-NET inc/arch/SMX | src/arch/SMX/ThreadLib.c src/arch/NET/generic/SoDisp.c |
| Azure RTOS | inc/arch/ThreadX | src/arch/ThreadX/ThreadLib.c src/arch/ThreadX/SoDisp.c |
| VxWorks | inc/arch/VxWorks | src/arch/VxWorks/ThreadLib.c src/arch/NET/generic/SoDisp.c |
| Windows | inc/arch/NET/Windows inc/arch/Windows | src/arch/Windows/ThreadLib.c src/arch/NET/generic/SoDisp.c |
| Windows CE | inc/arch/NET/CE inc/arch/Windows | src/arch/Windows/ThreadLib.c src/arch/NET/generic/SoDisp.c |
| Zephyr | inc/arch/Zephyr | src/arch/Zephyr/ThreadLib.c src/arch/NET/generic/SoDisp.c |

The generic directory 'inc' must also be in the include path.

# Additional Build Examples

## VxWorks

The following example shows how to compile Mako Server for VxWorks 7. Use [Xedge](#xedge-rtos) if you are using an older VxWorks version.

```
wr-cc -o examples/MakoServer/mako -static -fmerge-all-constants -O3 -Os\
    -DUSE_EMBEDDED_ZIP=0 -DBA_FILESIZE64 -DBA_HAS_ANSI_IO -DMAKO -DUSE_SQL=0\
    -DLUA_NUMBER_INTEGER=0\
    -Iinc -Iinc/arch/VxWorks -Iinc/arch/NET/Posix\
    src/BAS.c\
    src/arch/VxWorks/ThreadLib.c src/arch/NET/generic/SoDisp.c src/DiskIo/posix/BaFile.c\
    examples/MakoServer/src/MakoMain.c\
    -lnet
```

Note that we are not including SQLite support. You can include SQLite by using the SQLite version provided by Wind River.

## Azure RTOS using IAR

The following example shows how to compile the generic BAS library for Azure RTOS using IAR for ARM. We have the following directories tx (ThreadX), nx (NetX), and BAS.

```
iccarm -e -c^
  -Itx -Inx^
  -IBAS/inc -IBAS/inc/arch/ThreadX^
  BAS/src/BAS.c
```

Using the above compilation settings, you would also need to include src/arch/ThreadX/ThreadLib.c, src/arch/ThreadX/SoDisp.c, and optionally src/DiskIo/FileX/BaFile.c.

## FreeRTOS

#### i.MX RT1020
[Download](https://realtimelogic.com/downloads/bas/?target=RT1020) a ready to compile and run project for [i.MX RT1020](https://www.nxp.com/design/development-boards/i-mx-evaluation-and-development-boards/i-mx-rt1020-evaluation-kit:MIMXRT1020-EVK).

#### ESP32

[Xedge32](https://realtimelogic.com/ba/ESP32/) is an Xedge version for the ESP32 and ESP32-S3 microcontrollers.

# Barracuda App Server Tutorials

* [What is an Embedded Application Server?](https://realtimelogic.com/articles/What-is-an-Embedded-Application-Server)
* [Using Lua for IoT and Web Application Development](https://realtimelogic.com/articles/Lua-FastTracks-Embedded-Web-Application-Development)
* [Online Interactive Lua and LSP Tutorials](https://tutorial.realtimelogic.com/)
* [Modern Approach to Embedding a Web Server in a Device](https://realtimelogic.com/articles/Modern-Approach-to-Embedding-a-Web-Server-in-a-Device)
* [The Ultimate Guide to Embedded Web Server Technology](https://realtimelogic.com/articles/Embedded-Web-Server-Tutorials)
* [A Large List of Mako/BAS Tutorials](https://makoserver.net/tutorials/)
* [Ready To Run Examples (GitHub)](https://github.com/RealTimeLogic/LSP-Examples)
 
### DZone BAS and Mako Server Tutorials
* [AWS IoT MQTT Tutorial Using Lua](https://dzone.com/articles/aws-iot-mqtt-tutorial-using-lua)
* [Easy IoT and Device Management for Non-C Coders](https://dzone.com/articles/easy-iot-amp-device-management-for-non-c-coders)
* [IoT as an Alternative to VPN for Secure Remote Device Access](https://dzone.com/articles/iot-as-an-alternative-to-vpn-for-secure-remote-dev)
* [Cheaper Alternatives to IoT Cloud Services](https://dzone.com/articles/cheaper-alternatives-to-iot-cloud-services)
* [Have We Forgotten the Ancient Lessons About Building Defense Systems?](https://dzone.com/articles/have-we-forgotten-the-ancient-lessons)
* [Exploiting MQTT Using Lua](https://dzone.com/articles/exploiting-mqtt-using-lua)
* [An Arduino ESP8266 Christmas Light Controller](https://dzone.com/articles/arduino-esp8266-iot-light-controller)
