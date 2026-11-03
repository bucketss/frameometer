#ifndef __MODULECONFIG_H__
#define __MODULECONFIG_H__

#ifndef FRAMEOMETER_VERSION
#define FRAMEOMETER_VERSION "0.0.0"
#endif

#define MODULE_NAME     "frameometer"
#define MODULE_VERSION  FRAMEOMETER_VERSION
#define MODULE_AUTHOR   "arc"
#define MODULE_URL      ""
#define MODULE_LOGTAG   "frameometer"
#define MODULE_LIBRARY  "frameometer"
#define MODULE_LIBCLASS ""

#ifdef __DATE__
#define MODULE_DATE __DATE__
#else
#define MODULE_DATE "Unknown"
#endif

#define USE_METAMOD
#define NO_ALLOC_OVERRIDES

#define FN_AMXX_ATTACH OnAmxxAttach

#ifdef USE_METAMOD
#define FN_StartFrame         StartFrame
#define FN_CmdStart           CmdStart
#define FN_ClientConnect      ClientConnect
#define FN_ClientPutInServer  ClientPutInServer
#define FN_ClientDisconnect   ClientDisconnect
#endif // USE_METAMOD

#endif // __MODULECONFIG_H__
