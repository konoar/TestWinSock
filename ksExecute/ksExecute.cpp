/****************************************************

  ksExecute.cpp
   Copyright 2019.08.11 konoar

 ****************************************************/

#include "ksCommon.h"
#include <stdio.h>

int main(int argc, const char* argv[])
{

    int ret, msg, abort = 1;
    char buff[256];

    if (KS_OK != (ret = ksIPCExecuteInit())) {
        return ret;
    }

    while (abort) {

        if (KS_OK == ksIPCExecuteFetch(&msg, buff, 256)) {

            switch (msg) {

            case KS_MSG_EXE:
                printf("Execute: %s\n", buff);
                break;

            case KS_MSG_END:
                abort = 0;
                break;

            }

        }

    }

    if (KS_OK != (ret = ksIPCExecuteUninit())) {
        return ret;
    }

    return KS_OK;

}
