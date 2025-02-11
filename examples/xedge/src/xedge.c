/*
 *     ____             _________                __                _     
 *    / __ \___  ____ _/ /_  __(_)___ ___  ___  / /   ____  ____ _(_)____
 *   / /_/ / _ \/ __ `/ / / / / / __ `__ \/ _ \/ /   / __ \/ __ `/ / ___/
 *  / _, _/  __/ /_/ / / / / / / / / / / /  __/ /___/ /_/ / /_/ / / /__  
 * /_/ |_|\___/\__,_/_/ /_/ /_/_/ /_/ /_/\___/_____/\____/\__, /_/\___/  
 *                                                       /____/          
 *
 *                  Barracuda Embedded Web-Server 
 ****************************************************************************
 *
 *   $Id: xedge.c 5453 2023-06-02 13:04:40Z wini $
 *
 *   COPYRIGHT:  Real Time Logic, 2008 - 2023
 *               http://www.realtimelogic.com
 *
 *   The copyright to the program herein is the property of
 *   Real Time Logic. The program may be used or copied only
 *   with the written permission from Real Time Logic or
 *   in accordance with the terms and conditions stipulated in
 *   the agreement under which the program has been supplied.
 ****************************************************************************

Xedge Documentation (xedge):
http://realtimelogic.com/ba/doc/?url=/examples/xedge/readme.html

Note: the xedge includes files and resources from the auxiliary
API -- i.e. from the xrc directory. The makefile includes these files in the
build process. The makefile also assembles a number of resource files,
such as Lua scripts, HTML files, etc, and assembles these files into a
ZIP file.

The extra optional API's included are documented in the startup code below.

The xedge's C code shows typical C startup code required for the
Barracuda App Server when used in an RTOS/firmware environment. The C
code includes graceful shutdown, however, the graceful shutdown code
is only included if compiled with USE_DBGMON=1 -- i.e. when the debug
monitor is enabled. Graceful shutdown is typically not required in a
firmware environment, but is required for the debug monitor's hot
restart feature.

The C code below indirectly installs a Lua debugger hook if compiled
with USE_DBGMON=1 and installs a Lua debugger hook directly if
compiled with macro ENABLE_LUA_TRACE. The code enabled with
ENABLE_LUA_TRACE shows how to print each Lua line executed. See
function lHook below.  See the following link for how to use the
debugger: https://makoserver.net/articles/Lua-and-LSP-Debugging

Note: some platforms automatically set USE_DBGMON=1 if the macro is
not defined. See inc/arch/<PLAT>/TargConfig.h for your platform.
*/


/*
  Include all I/O related header files
*/
#include <barracuda.h>

/* Special case when working on Xedge Lua code on a host computer */
#if defined(NO_BAIO_DISK) && defined(BAIO_DISK)
#include <BaDiskIo.h>
#endif

/* When MAXTHREADS is defined, the HttpCmdThreadPool (HTTP thread
 * pool) is not utilized. Instead, the LThreadMgr is used as the
 * thread pool. Additionally, the LThreadMgr is configured to prohibit
 * the creation of additional threads.
 * Details:
 *  https://realtimelogic.com/ba/doc/en/C/reference/html/group__ThreadMgr.html
 */
#ifdef ESP_PLATFORM
#define MAXTHREADS 2
#endif

/* Call this function from the program's main() function or from a
 * dedicated thread.
 */
void barracuda(void);


/* The server's socket dispatcher object.
 */
static SoDisp dispatcher;

/* We keep the Thread Manager global so it can be accessed as:
   extern LThreadMgr ltMgr;
*/
LThreadMgr ltMgr;


/* 
   io=ezip
*/
#if !defined(BAIO_DISK) && !defined(BAIO_ZIP)
/* Default is: BAIO_EZIP */
extern ZipReader* getLspZipReader(void);
#endif

extern void luaopen_AUX(lua_State* L); /* Example Lua binding: led.c */

#if USE_PROTOBUF
extern int luaopen_pb(lua_State* L);
#endif

#if USE_OPCUA
#include <opcua_module.h>
#endif

#if USE_DBGMON
/* This function runs the Lua 'onunload' handlers for all loaded apps
   when the server exits.  onunload is an optional function Lua
   applications loaded by the server can use if the applications
   require graceful termination such as sending a socket close message
   to peers.
 */
static void
onunload(lua_State* L, int onunloadRef)
{
   /* Run the 'onunload' function in the .config Lua script, which in
    * turn runs the optional onunload for all loaded apps.
    */
   lua_rawgeti(L, LUA_REGISTRYINDEX, onunloadRef);
   baAssert(lua_isfunction(L, -1));
   if(lua_pcall(L, 0, 1, 0))
   {
      HttpTrace_printf(0,"Error in 'onunload': %s\n",
                       lua_isstring(L,-1) ? lua_tostring(L, -1) : "?");
   }
   luaL_unref(L, LUA_REGISTRYINDEX, onunloadRef);
   balua_relsocket(L); /* Gracefully close all cosockets, if any */
}
#endif


/* RTOS devices typically require special file system initialization.
 * This variable can be set to a function by platform specific startup
 * code. (REF-1).
 */
#ifndef NO_BAIO_DISK

int (*platformInitDiskIo)(DiskIo*);

/* File system init code for HLOS */
#if defined _WIN32 || defined(BA_POSIX)
#include <stdlib.h>
static int initDiskIo(DiskIo* dio)
{
   static const char appmgr[] = {"xedge"};
   int retVal;
   char* buf;
   IoStat st;
   IoIntf* io = (IoIntf*)dio;
   const char* home = getenv("HOME");
   if( ! home )
      home = getenv("USERPROFILE");
   if( ! home )
   {
      HttpTrace_printf(0, "'HOME' environment variable not set\n");
      return -1;
   }
   DiskIo_setRootDir(dio,home);
   if(io->statFp(io,appmgr,&st) && io->mkDirFp(io,appmgr,0))
   {
      HttpTrace_printf(0, "Cannot create %s/%s\n",home,appmgr);
      return -1;
   }
   buf=baMalloc(strlen(home)+sizeof(appmgr)+1);
   if(!buf) return -1;
   basprintf(buf,"%s/%s",home,appmgr);
   HttpTrace_printf(0, "Xedge root directory: %s\n", buf);
   retVal=DiskIo_setRootDir(dio,buf);
   baFree(buf);
   return retVal;
}
#endif
/* End file system init code */

#endif /* NO_BAIO_DISK */


/* ba.encryptionkey()
   Change key in EncryptionKey.h before compiling.
*/
#include "EncryptionKey.h"
static int
lEncryptionKey(lua_State *L)
{
   lua_pushlstring(L,(char*)ENCRYPTIONKEY,sizeof(ENCRYPTIONKEY));
   return 1;
}

/* Install ba.encryptionkey() */
static void
luaopen_encryptionkey(lua_State *L) 
{
   lua_getglobal(L, "ba");
   lua_pushcfunction(L, lEncryptionKey);
   lua_setfield(L,-2,"encryptionkey"); 
   lua_pop(L,1); /* ba */
}



/* SharkTrustX

   Create your own tokengen.h header file as follows:

   1: Sign up for Real Time Logic's DNS testing service or preferably,
      set up your own DNS service (one of:)
        A: Testing service: https://acme.realtimelogic.com/
        B: Your own service: https://github.com/RealTimeLogic/SharkTrustX
   2: Wait for the email received from the DNS service
   3: Navigate to your DNS zone https://your-domain/login
   4: Login using your credentials
   5: Navigate to https://your-domain/cgen
   6: Download the generated C code, rename to tokengen.h,
      and place generated file in this directory
*/
#define EMBEDDED_ZONE_KEY
#include "tokengen.c"


/* Initialize the HttpServer object by calling HttpServer_constructor.
   The HttpServerConfig object is only needed during initialization.
*/
static void
createServer(HttpServer* server)
{
   HttpServerConfig scfg;
#ifdef MAXTHREADS
   U16 cmdInstances=MAXTHREADS;
   if(cmdInstances > 1)
      cmdInstances-=1;
#else
   /* (A) Configure for 2 threads. See HttpCmdThreadPool. */
   U16 cmdInstances=2;
#endif
   HttpServerConfig_constructor(&scfg);

   HttpServerConfig_setNoOfHttpCommands(&scfg,cmdInstances);

   HttpServerConfig_setNoOfHttpConnections(&scfg,8);

   /* For huge url encoded data, if any. */
   HttpServerConfig_setRequest(&scfg,2*1024, 8*1024);
   /* Large response buffers makes the NetIO much faster when used by
    * the web server.
    */
   HttpServerConfig_setResponseData(&scfg,8*1024);

   /* Create and init the server, by using the above HttpServerConfig.
    */
   HttpServer_constructor(server, &dispatcher, &scfg);
}


/* Lua debugger hook. Use the debug monitor or the basic trace lib.
 */
#if USE_DBGMON
static void
dbgExitRequest(void* na, BaBool rstart)
{
   (void)na;
   (void)rstart;
   SoDisp_setExit(&dispatcher);
}
#elif defined(ENABLE_LUA_TRACE)
/* Prints every line executed by Lua if enabled */
static void lHook(lua_State *L, lua_Debug *ar)
{
   lua_getinfo(L, "S", ar);
   printf("%4d\t%s\n",ar->currentline,ar->short_src);
}
#endif


/* Create and return the IO used by the Lua virtual machine.
   The IO can be configured with compile time options as follows:
   disk   |    net   |    zip   |   ezip
   BAIO_DISK | BAIO_NET | BAIO_ZIP | BAIO_EZIP
*/
#ifdef BAIO_DISK_PATH
#define BAIO_DISK
#define PATH_AUX(x) #x
#define PATH_STRINGIFY(x) PATH_AUX(x)
#define BAIO_DISK_PATH_STR PATH_STRINGIFY(BAIO_DISK_PATH)
#else
#define BAIO_DISK_PATH_STR "../XedgeResources"
#endif
static IoIntf*
createVmIo()
{
#if defined(BAIO_DISK)
   static DiskIo vmIo;
   int ecode;
   DiskIo_constructor(&vmIo);
   if( (ecode=DiskIo_setRootDir(&vmIo, BAIO_DISK_PATH_STR)) != 0 )
   {
      HttpTrace_printf(0, "Cannot set DiskIo " BAIO_DISK_PATH_STR " directory: % s\n",
                       baErr2Str(ecode));
      baFatalE(FE_USER_ERROR_1, 0);
   }
#elif defined(BAIO_NET)
#error Cannot use the NetIo: Not implemented.
/* Disabled Makefile option: make io=net
   Think about the Xedge as a tool and the tool's
   web and Lua app as firmware. The lua code is required for the
   operation of the tool and should be embedded in the executable. The
   code is designed to be used "as is" as a development tool and you
   do not need to study or modify it (unless you want to know how the
   Lua code works).
*/
#else
   static ZipIo vmIo;
   ZipReader* zipReader;
#if defined(BAIO_ZIP)
   static FileZipReader fileZipReader;
   FileZipReader_constructor(&fileZipReader, "lsp.zip");
   zipReader = (ZipReader*)&fileZipReader;  /* cast to base class */
#else
   /* BAIO_EZIP: for Xedge release build. */
   zipReader = getLspZipReader();
#endif
   if( ! CspReader_isValid((CspReader*)zipReader) )
      baFatalE(FE_USER_ERROR_2, 0);
   ZipIo_constructor(&vmIo, zipReader, 2048, 0);
   if(ZipIo_getECode(&vmIo) !=  ZipErr_NoError)
      baFatalE(FE_USER_ERROR_3, 0);
#endif
   return (IoIntf*)(&vmIo);
}


/*
  This is the "main" barracuda function. This function initializes the
  xedge, configures a Lua Virtual Machine and enters a forever
  loop.

  This function is typically called from a thread/task dedicated to
  running the Barracuda App Server socket dispatcher SoDisp.

  Notice that we do not create a Virtual File System in this startup
  code. The Virtual File System is created by the Lua .config script
  in the "lsp" subdirectory. The .config script is executed when we
  call balua_loadconfig from the C code below.
*/
void
barracuda(void)
{
   /* static: less stack; we only have one server instance */
   static ThreadMutex mutex;
   static HttpServer server;
   static BaTimer timer;
#ifndef MAXTHREADS
   static HttpCmdThreadPool pool;
#endif
   static int ecode;
   static NetIo netIo;
   static lua_State* L; /* pointer to a Lua virtual machine */
   static BaLua_param blp; /* configuration parameters */
   static int onunloadRef;
   static int startServerRef;
#ifndef NO_BAIO_DISK
   static DiskIo diskIo;
#endif

/* If restarted by Lua debugger */
#if USE_DBGMON
  L_restart:
#endif

   HttpTrace_setPrio(7); /* block level 8 and 9 */

/* Example code for 'platformInitDiskIo declaration' (REF-1).
   This would typically be in another (startup) file for deep embedded systems.
*/
#if !defined(NO_BAIO_DISK) &&                   \
   (defined(_WIN32) ||                          \
    defined(BA_POSIX))
   platformInitDiskIo=initDiskIo;
#endif

   /* Create the Socket dispatcher (SoDisp), the SoDisp mutex, and the server.
    */
   ThreadMutex_constructor(&mutex);
   SoDisp_constructor(&dispatcher, &mutex);
   createServer(&server);

   /* For embedded systems when developing */
   NetIo_constructor(&netIo, &dispatcher);

   /* The optional timer is useful in LSP code */
   BaTimer_constructor(&timer, &mutex, BA_STACKSZ, 25, ThreadPrioNormal, 0);
   
   /* Create a LSP virtual machine.
    */
   memset(&blp, 0, sizeof(blp));
   blp.vmio = createVmIo();  /* The required IO */
   blp.server = &server;     /* pointer to a HttpServer */
   blp.timer = &timer;       /* Pointer to a BaTimer */
   L = balua_create(&blp);   /* create the Lua state */

   /* Lua debugger hook.
      Use the debugger monitor or the basic trace lib (lHook).
      Must be first.
   */
#if USE_DBGMON
   ba_ldbgmon(L, dbgExitRequest, 0);
#elif defined(ENABLE_LUA_TRACE)
   lua_sethook(L, lHook, LUA_MASKLINE, 0);
#endif


   /* Install optional IO interfaces */
   balua_iointf(L, "net",  (IoIntf*)&netIo);
   balua_http(L); /* Install optional HTTP client library */
   LThreadMgr_constructor(&ltMgr,
                          &server,
                          ThreadPrioNormal,
                          BA_STACKSZ,
#ifdef MAXTHREADS
                          MAXTHREADS,
#else
                          3, /* Number of threads */
#endif
                          L,
#ifdef MAXTHREADS
                          FALSE /* do not allow creating more threads */
#else
                          TRUE /* allow creating more threads */
#endif
      );
                          
   balua_socket(L);  /* Install optional Lua socket library */
   balua_sharkssl(L);  /* Install optional Lua SharkSSL library */
   balua_crypto(L);  /* Install optional crypto library */
   balua_tracelogger(L,&ltMgr); /* Install optional trace logger library */
   balua_luaio(L); /* xrc/lua/lio.c */

/* 
   Some embedded devices may not have a DiskIo.
   The following macro makes it possible to disable the DiskIo.
   Command line: make ..... nodisk=1
*/
#ifdef NO_BAIO_DISK
   HttpTrace_printf(0, "DiskIo not included. Use NetIo!\n");
#else
   DiskIo_constructor(&diskIo);
   /* REF-1 */
   if( platformInitDiskIo && (*platformInitDiskIo)(&diskIo) == 0)
   {
      /* Add optional IO interfaces */
      balua_iointf(L, "disk",  (IoIntf*)&diskIo);
   }
   else
   {
      HttpTrace_printf(0, "Installing DiskIo failed!\n");
   }
#endif

   /* Create user/login tracker */
   balua_usertracker_create(
      L,
      20, /* Tracker node size. */
      4, /* Max number of login attempts. */
      10*60); /* 10 minutes ban time if more than 4 login attempts in a row. */

   balua_tokengen(L); /* See  the "SharkTrustX" comment above */
#if USE_REVCON
   /* Add reverse server connection. This requires SharkTrustX.
    */
   balua_revcon(L);
#endif
#if USE_DLMALLOC
   balua_mallinfo(L); /* Mem info. See dlmalloc.c */
#endif
#if USE_PROTOBUF
   /* Google's protobuf for Lua: https://github.com/starwing/lua-protobuf */
   luaL_requiref(L, "pb", luaopen_pb, FALSE);
   lua_pop(L,1); /* Pop pb obj: statically loaded, not dynamically. */
#endif
#if USE_OPCUA
   luaopen_opcua_ns0_static(L);
#endif
   luaopen_encryptionkey(L); /* code above in this file */
   /* Dispatcher mutex must be locked until the dispatcher starts
    */
   ThreadMutex_set(&mutex);
   ecode=balua_loadconfigExt(L, blp.vmio, 0, 2); /* Load and run .config  */
   if(ecode)
   {
      HttpTrace_printf(0,".config error: %s.\n", lua_tostring(L,-1)); 
      baFatalE(FE_USER_ERROR_2, 0);
   }
   /* .config must return two functions, one for starting server, and
    * one onunload handler
    */
   if(!lua_isfunction(L, -1) || !lua_isfunction(L, -2))
   {
      HttpTrace_printf(0,".config error: no start server or onunload\n");
      baFatalE(FE_USER_ERROR_3, 0);
   }
   /* Keep a reference to function returned by .config */
   onunloadRef=luaL_ref(L,LUA_REGISTRYINDEX);
   startServerRef=luaL_ref(L,LUA_REGISTRYINDEX);

   /* See (A) above */
#ifdef MAXTHREADS
   LThreadMgr_enableHttpPool(&ltMgr, &server);
#else
   HttpCmdThreadPool_constructor(&pool, &server, ThreadPrioNormal, BA_STACKSZ);
#endif

   /* Example Lua bindings, compile with AsynchLua.c or led.c.
      This code opens ESP32 bindings when compiled for ESP32.
    */
   luaopen_AUX(L);

   lua_rawgeti(L, LUA_REGISTRYINDEX, startServerRef);
   baAssert(lua_isfunction(L, -1));
   if(lua_pcall(L, 0, 1, 0))
   {
      HttpTrace_printf(0,"Error in 'startServer': %s\n",
                       lua_isstring(L,-1) ? lua_tostring(L, -1) : "?");
   }
   luaL_unref(L, LUA_REGISTRYINDEX, startServerRef);

   /*
     The dispatcher object waits for incoming HTTP requests. These
     requests are sent to the HttpServer object, where they are delegated to
     a Barracuda resource such as the WebDAV instance.

     Note: the server socket connections are opened by the Lua script
     .config and not by C code.

     Arg -1: Never returns, unless SoDisp_setExit() is called
   */
   ThreadMutex_release(&mutex);
   SoDisp_run(&dispatcher, -1);

   /* Enable gracefull shutdown if debug monitor is included.
    */
#if USE_DBGMON
   /*Dispatcher mutex must be locked when terminating the following objects.*/
   ThreadMutex_set(&mutex);
   /* Graceful termination of Lua apps. See function above. */
   onunload(L, onunloadRef);
   LThreadMgr_destructor(&ltMgr); /* Wait for threads to exit */
#ifndef MAXTHREADS
   HttpCmdThreadPool_destructor(&pool);  /* Wait for threads to exit */
#endif

   /* Must cleanup all sessions before destroying the Lua VM */
   HttpServer_termAllSessions(&server);
   /* Destroy all objects, including server listening objects. */
   balua_close(L);

   IoIntf_destructor(blp.vmio); /* Virtual destr */
#ifndef NO_BAIO_DISK
   DiskIo_destructor(&diskIo);
#endif
   NetIo_destructor(&netIo);
   BaTimer_destructor(&timer);
   HttpServer_destructor(&server);
   SoDisp_destructor(&dispatcher);
   ThreadMutex_release(&mutex);   
   ThreadMutex_destructor(&mutex);
   HttpTrace_printf(0,"\n\nRestarting xedge.\n\n");
   HttpTrace_flush();
   goto L_restart;
#else /*  USE_DBGMON */
   (void)onunloadRef;
#endif
}
