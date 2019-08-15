/****************************************************

  ksCommon.h
   Copyright 2019.08.11 konoar

 ****************************************************/

#ifndef __KS_COMMON_H__
#define __KS_COMMON_H__

#define KS_OK   0
#define KS_NG   1

enum ksMsg
{
    KS_MSG_ADD,
    KS_MSG_EXE,
    KS_MSG_END,
    KS_MSG_MAX
};

extern int ksIPCCoreInit            ();
extern int ksIPCCoreFetch           (int *msg, char *param, int size);
extern int ksIPCCoreSendToExecute   (int msg, const char *param = 0);
extern int ksIPCCoreUninit          ();

extern int ksIPCConsoleInit         ();
extern int ksIPCConsoleSendToCore   (int msg, const char *param = 0);
extern int ksIPCConsoleUninit       ();

extern int ksIPCExecuteInit         ();
extern int ksIPCExecuteFetch        (int *msg, char *param, int size);
extern int ksIPCExecuteUninit       ();

#endif // __KS_COMMON_H__
