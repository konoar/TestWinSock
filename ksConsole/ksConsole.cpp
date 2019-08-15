/****************************************************

  ksConsole.cpp
   Copyright 2019.08.11 konoar

 ****************************************************/

#include "ksCommon.h"
#include <Windows.h>
#include "resource.h"

LRESULT __stdcall DlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    int lenw, lenm;
    wchar_t buffw[16];
    char    buffm[33];

    switch (msg) {

    case WM_INITDIALOG:
        SetWindowText(GetDlgItem(hwnd, IDC_ED_DATA), L"Ç±Ç±Ç…ÉfÅ[É^Çì¸óÕ");
        break;

    case WM_COMMAND:
        switch (LOWORD(wp)) {

        case IDC_BT_ADD:
            if (0 < (lenw = GetWindowText(GetDlgItem(hwnd, IDC_ED_DATA), buffw, 16))) {
                lenm = WideCharToMultiByte(932, 0, buffw, lenw, buffm, 32, NULL, NULL);
                buffm[lenm] = '\0';
                if (KS_OK == ksIPCConsoleSendToCore(KS_MSG_ADD, buffm)) {
                    SetWindowText(GetDlgItem(hwnd, IDC_ED_DATA), L"");
                }
            }
            else {
                ksIPCConsoleSendToCore(KS_MSG_ADD);
            }
            SetFocus(GetDlgItem(hwnd, IDC_ED_DATA));
            break;

        case IDC_BT_EXE:
            if (KS_OK == ksIPCConsoleSendToCore(KS_MSG_EXE)) {
                SetFocus(GetDlgItem(hwnd, IDC_ED_DATA));
            }
            break;

        }
        break;

    case WM_CLOSE:
        ksIPCConsoleSendToCore(KS_MSG_END);
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wp, lp);

    }

    return 0;

}

int __stdcall WinMain(
    HINSTANCE hinst, HINSTANCE hprev, LPSTR cmd, int show)
{

    MSG msg;
    int ret;

    if (KS_OK != (ret = ksIPCConsoleInit())) {
        return ret;
    }

    HWND hdlg = CreateDialog(
        hinst,
        MAKEINTRESOURCE(IDD_MAIN),
        NULL,
        (DLGPROC)DlgProc
    );

    if (!hdlg) {
        ksIPCConsoleUninit();
        return KS_NG;
    }

    ShowWindow  (hdlg, SW_SHOW);
    UpdateWindow(hdlg);

    while (GetMessage(&msg, NULL, NULL, NULL)) {
        TranslateMessage(&msg);
        DispatchMessage (&msg);
    }

    if (KS_OK != (ret = ksIPCConsoleUninit())) {
        return ret;
    }

    return KS_OK;

}
