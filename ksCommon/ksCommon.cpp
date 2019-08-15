/****************************************************

  ksCommon.cpp
   Copyright 2019.08.11 konoar

 ****************************************************/

#include "ksCommon.h"

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define KS_COMMON_LOCALHOST     "127.0.0.1"
#define KS_COMMON_PORT_CORE     "10002"
#define KS_COMMON_PORT_EXEC     "10003"
#define KS_COMMON_BUFF_MAX      256

static const char *KS_MSG[KS_MSG_MAX] =
{
    "Add",
    "Exe",
    "End"
};

static WSADATA wsa;
static SOCKET ls = INVALID_SOCKET;

int ksIPCCoreInit()
{

    struct addrinfo hints, *info;

    if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
        return KS_NG;
    }

    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family     = AF_INET;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_protocol   = IPPROTO_TCP;
    hints.ai_flags      = AI_PASSIVE;

    if (0 != getaddrinfo(NULL, KS_COMMON_PORT_CORE, &hints, &info)) {
        return KS_NG;
    }

    if (INVALID_SOCKET == (ls = socket(
        info->ai_family, info->ai_socktype, info->ai_protocol))) {
        freeaddrinfo(info);
        return KS_NG;
    }

    if (SOCKET_ERROR == bind(ls, info->ai_addr, (int)info->ai_addrlen)) {
        freeaddrinfo(info);
        closesocket(ls);
        return KS_NG;
    }

    freeaddrinfo(info);

    if (SOCKET_ERROR == listen(ls, SOMAXCONN)) {
        closesocket(ls);
        return KS_NG;
    }

    return KS_OK;

}

int ksIPCCoreFetch(int *msg, char *param, int size)
{

    SOCKET s = INVALID_SOCKET;
    char buff[KS_COMMON_BUFF_MAX];
    int ret, cnt = 0;

    if (INVALID_SOCKET == ls) {
        return KS_NG;
    }

    if (INVALID_SOCKET == (s = accept(ls, NULL, NULL))) {
        return KS_NG;
    }

    for (;;) {

        if (0 == (ret = recv(s, buff + cnt, KS_COMMON_BUFF_MAX - cnt, 0))) {
            break;
        }

        cnt += ret;

        if (cnt >= KS_COMMON_BUFF_MAX - 1) {
            cnt = KS_COMMON_BUFF_MAX;
            break;
        }

    }

    if (SOCKET_ERROR == shutdown(s, SD_SEND)) {
        closesocket(s);
        return KS_NG;
    }

    closesocket(s);

    buff[cnt] = '\0';

    if (buff == strstr(buff, KS_MSG[KS_MSG_ADD])) {
        *msg = KS_MSG_ADD;
        if (param) {
            strcpy_s(param, size, buff + 3);
        }
    }

    if (buff == strstr(buff, KS_MSG[KS_MSG_EXE])) {
        *msg = KS_MSG_EXE;
    }

    if (buff == strstr(buff, KS_MSG[KS_MSG_END])) {
        *msg = KS_MSG_END;
    }

    return KS_OK;

}

int ksIPCCoreSendToExecute(int msg, const char *param)
{

    SOCKET s = INVALID_SOCKET;
    struct addrinfo hints, *info, *a;
    char buffs[256], buffr[256];

    switch (msg) {

    case KS_MSG_ADD:
    case KS_MSG_EXE:
    case KS_MSG_END:
        strcpy_s(buffs, KS_MSG[msg]);
        if (param) {
            strcat_s(buffs, param);
        }
        break;

    default:
        return KS_NG;

    }

    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_protocol   = IPPROTO_TCP;

    if (0 != getaddrinfo(KS_COMMON_LOCALHOST, KS_COMMON_PORT_EXEC, &hints, &info)) {
        return KS_NG;
    }

    for (a = info; a != NULL; a = a->ai_next) {

        if (INVALID_SOCKET == (s = socket(a->ai_family, a->ai_socktype, a->ai_protocol))) {
            freeaddrinfo(info);
            return KS_NG;
        }

        if (SOCKET_ERROR == connect(s, a->ai_addr, (int)a->ai_addrlen)) {
            closesocket(s);
            s = INVALID_SOCKET;
            continue;
        }

        break;

    }

    freeaddrinfo(info);

    if (INVALID_SOCKET == s) {
        return KS_NG;
    }

    if (SOCKET_ERROR == send(s, buffs, strlen(buffs), 0)) {
        closesocket(s);
        return KS_NG;
    }

    if (SOCKET_ERROR == shutdown(s, SD_SEND)) {
        closesocket(s);
        return KS_NG;
    }

    while (0 > recv(s, buffr, 256, 0));

    closesocket(s);

    return KS_OK;

}

int ksIPCCoreUninit()
{

    closesocket(ls);
    ls = INVALID_SOCKET;

    if (SOCKET_ERROR == WSACleanup()) {
        return KS_NG;
    }

    return KS_OK;

}

int ksIPCConsoleInit()
{

    if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
        return KS_NG;
    }

    return KS_OK;

}

int ksIPCConsoleSendToCore(int msg, const char *param)
{

    SOCKET s = INVALID_SOCKET;
    struct addrinfo hints, *info, *a;
    char buffs[256], buffr[256];

    switch (msg) {

    case KS_MSG_ADD:
    case KS_MSG_EXE:
    case KS_MSG_END:
        strcpy_s(buffs, KS_MSG[msg]);
        if (param) {
            strcat_s(buffs, param);
        }
        break;

    default:
        return KS_NG;

    }

    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_protocol   = IPPROTO_TCP;

    if (0 != getaddrinfo(KS_COMMON_LOCALHOST, KS_COMMON_PORT_CORE, &hints, &info)) {
        return KS_NG;
    }

    for (a = info; a != NULL; a = a->ai_next) {

        if (INVALID_SOCKET == (s = socket(a->ai_family, a->ai_socktype, a->ai_protocol))) {
            freeaddrinfo(info);
            return KS_NG;
        }

        if (SOCKET_ERROR == connect(s, a->ai_addr, (int)a->ai_addrlen)) {
            closesocket(s);
            s = INVALID_SOCKET;
            continue;
        }

        break;

    }

    freeaddrinfo(info);

    if (INVALID_SOCKET == s) {
        return KS_NG;
    }

    if (SOCKET_ERROR == send(s, buffs, strlen(buffs), 0)) {
        closesocket(s);
        return KS_NG;
    }

    if (SOCKET_ERROR == shutdown(s, SD_SEND)) {
        closesocket(s);
        return KS_NG;
    }

    while (0 > recv(s, buffr, 256, 0));

    closesocket(s);

    return KS_OK;

}

int ksIPCConsoleUninit()
{

    if (SOCKET_ERROR == WSACleanup()) {
        return KS_NG;
    }

    return KS_OK;

}

int ksIPCExecuteInit()
{

    struct addrinfo hints, *info;

    if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
        return KS_NG;
    }

    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family     = AF_INET;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_protocol   = IPPROTO_TCP;
    hints.ai_flags      = AI_PASSIVE;

    if (0 != getaddrinfo(NULL, KS_COMMON_PORT_EXEC, &hints, &info)) {
        return KS_NG;
    }

    if (INVALID_SOCKET == (ls = socket(
        info->ai_family, info->ai_socktype, info->ai_protocol))) {
        freeaddrinfo(info);
        return KS_NG;
    }

    if (SOCKET_ERROR == bind(ls, info->ai_addr, (int)info->ai_addrlen)) {
        freeaddrinfo(info);
        closesocket(ls);
        return KS_NG;
    }

    freeaddrinfo(info);

    if (SOCKET_ERROR == listen(ls, SOMAXCONN)) {
        closesocket(ls);
        return KS_NG;
    }

    return KS_OK;

}

int ksIPCExecuteFetch(int *msg, char *param, int size)
{

    SOCKET s = INVALID_SOCKET;
    char buff[KS_COMMON_BUFF_MAX];
    int ret, cnt = 0;

    if (INVALID_SOCKET == ls) {
        return KS_NG;
    }

    if (INVALID_SOCKET == (s = accept(ls, NULL, NULL))) {
        return KS_NG;
    }

    for (;;) {

        if (0 == (ret = recv(s, buff + cnt, KS_COMMON_BUFF_MAX - cnt, 0))) {
            break;
        }

        cnt += ret;

        if (cnt >= KS_COMMON_BUFF_MAX - 1) {
            cnt = KS_COMMON_BUFF_MAX;
            break;
        }

        buff[cnt] = '\0';

    }

    if (SOCKET_ERROR == shutdown(s, SD_SEND)) {
        closesocket(s);
        return KS_NG;
    }

    closesocket(s);

    if (buff == strstr(buff, KS_MSG[KS_MSG_EXE])) {
        *msg = KS_MSG_EXE;
        if (param) {
            strcpy_s(param, size, buff + 3);
        }
    }

    if (buff == strstr(buff, KS_MSG[KS_MSG_END])) {
        *msg = KS_MSG_END;
    }

    return KS_OK;

}

int ksIPCExecuteUninit()
{

    closesocket(ls);
    ls = INVALID_SOCKET;

    if (SOCKET_ERROR == WSACleanup()) {
        return KS_NG;
    }

    return KS_OK;

}
