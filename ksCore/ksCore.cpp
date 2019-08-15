/****************************************************

  ksCore.cpp
   Copyright 2019.08.11 konoar

 ****************************************************/

#include "ksCommon.h"
#include <Windows.h>

#define KS_DATA_MAX 8

enum
{
    KS_EVENT_EXE,
    KS_EVENT_END,
    KS_EVENT_MAX
};

static struct ksData
{

    HANDLE events[KS_EVENT_MAX];

    int count = 0;

    struct ksRecord {

        char    param[256];

    } record[KS_DATA_MAX];

} d;

static HANDLE CallExe(const wchar_t* filename)
{

    wchar_t buff[MAX_PATH];

    GetFullPathName(filename, MAX_PATH, buff, NULL);

    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    ZeroMemory(&pi, sizeof(pi));

    if (0 == CreateProcess(
        buff, NULL, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {

        return INVALID_HANDLE_VALUE;

    }

    return pi.hProcess;

}

DWORD __stdcall Run1(LPVOID param)
{

    int msg, abort = 1;
    char buff[256];

    while (abort) {
        if (KS_OK == ksIPCCoreFetch(&msg, buff, sizeof(buff))) {

            switch (msg) {

            case KS_MSG_ADD:
                if (d.count < KS_DATA_MAX) {
                    strcpy_s(d.record[d.count++].param, buff);
                }
                break;

            case KS_MSG_EXE:
                SetEvent(d.events[KS_EVENT_EXE]);
                break;

            case KS_MSG_END:
                SetEvent(d.events[KS_EVENT_END]);
                abort = 0;
                break;

            }

        }
    }

    return KS_OK;

}

DWORD __stdcall Run2(LPVOID param)
{

    int abort = 1;

    while (abort) {
        switch (WaitForMultipleObjects(KS_EVENT_MAX, d.events, FALSE, INFINITE)) {

        case WAIT_OBJECT_0 + KS_EVENT_EXE:
            if (d.count) {
                for (int idx = 0; idx < d.count; idx++) {
                    ksIPCCoreSendToExecute(KS_MSG_EXE, d.record[idx].param);
                }
                d.count = 0;
            }
            else {
                // コンソールがTOPMOSTのためメッセージを表示しないように変更
                // MessageBox(NULL, L"no data.", L"Msg", MB_OK);
            }
            break;

        case WAIT_OBJECT_0 + KS_EVENT_END:
            ksIPCCoreSendToExecute(KS_MSG_END);
            abort = 0;
            break;

        }
    }

    return KS_OK;

}

int __stdcall WinMain(HINSTANCE hinst, HINSTANCE hprev, LPSTR cmd, int show)
{

    int ret;

    if (KS_OK != (ret = ksIPCCoreInit())) {
        return ret;
    }

    d.events[KS_EVENT_EXE] = CreateEvent(NULL, FALSE, FALSE, L"EXE");
    d.events[KS_EVENT_END] = CreateEvent(NULL, FALSE, FALSE, L"END");

    HANDLE hdls[] = {

        CreateThread(NULL, 0, Run1, 0, 0, NULL),
        CreateThread(NULL, 0, Run2, 0, 0, NULL),

        CallExe(L"ksExecute.exe"),
        CallExe(L"ksConsole.exe")

    };

    WaitForMultipleObjects(sizeof(hdls) / sizeof(HANDLE), hdls, TRUE, INFINITE);

    if (KS_OK != (ret = ksIPCCoreUninit())) {
        return ret;
    }

    return KS_OK;

}
