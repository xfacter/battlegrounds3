/**
 * This file belongs to the 'xlab' game engine.
 * Copyright 2009 xfacter
 * Copyright 2016 wickles
 * This work is licensed under the LGPLv3
 * subject to all terms as reproduced in the included LICENSE file.
 */

#include <pspsdk.h>

#include "xlab.h"

#ifdef X_PSP_KERNEL
PSP_MODULE_INFO("xlabApp", PSP_MODULE_KERNEL, 1, 1);
PSP_MAIN_THREAD_ATTR(0);
//PSP_HEAP_SIZE_MAX();
PSP_HEAP_SIZE_KB(-64);
#else
PSP_MODULE_INFO("xlabApp", PSP_MODULE_USER, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU);
//PSP_HEAP_SIZE_MAX();
PSP_HEAP_SIZE_KB(-64);
#endif

static int x_running = 1;

inline int xRunning()
{
	return x_running;
}

inline void xExit()
{
    x_running = 0;
}

static int xExitCallback(int arg1, int arg2, void *common)
{
	xExit();
	return 0;
}

static int xCallbackThread(SceSize args, void *argp)
{
    int cbid = sceKernelCreateCallback("xExitCallback", xExitCallback, 0);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

extern int xSetupCallbacks()
{
    int thid = sceKernelCreateThread("xCallbackThread", xCallbackThread, 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
    if(thid >= 0) sceKernelStartThread(thid, 0, 0);
    x_running = 1;
    return thid;
}

extern int xMain();

#ifdef X_PSP_KERNEL
static int xUserThread(SceSize args, void* argp)
{
    remove("./xlog.txt");
    X_LOG("Logging started.",0);
    xMain();
    X_LOG("Logging finished.",0);
    sceKernelExitGame();
    return 0;
}

int main(int argc, char *argv[])
{
    #ifdef X_MODULE_WIFI
    pspSdkLoadInetModules();
    pspSdkInetInit();
    #endif
    #ifdef X_MODULE_MP3
    //Reserved
    #endif
    xSetupCallbacks();
    int user_thid = sceKernelCreateThread("xUserThread", xUserThread, 0x18, 0x10000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, 0);
    if (user_thid >= 0) sceKernelStartThread(user_thid, 0, 0);
    sceKernelExitDeleteThread(0);
    return 0;
}
#else
int main(int argc, char **argv)
{
    xSetupCallbacks();
    remove("./xlog.txt");
    X_LOG("Logging started.",0);
    xMain();
    X_LOG("Logging finished.",0);
    sceKernelExitGame();
    return 0;
}
#endif
