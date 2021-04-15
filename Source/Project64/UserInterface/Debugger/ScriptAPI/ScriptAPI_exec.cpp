#include <stdafx.h>
#include <windows.h>
#include "../ScriptAPI.h"

void ScriptAPI::Define_exec(duk_context* ctx)
{
    duk_push_global_object(ctx);
    duk_push_string(ctx, "exec");
    duk_push_c_function(ctx, js_exec, DUK_VARARGS);
    duk_freeze(ctx, -1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    duk_pop(ctx);
}

duk_ret_t ScriptAPI::js_exec(duk_context* ctx)
{
    CScriptInstance* inst = GetInstance(ctx);
    CScriptSystem* sys = inst->System();

    duk_idx_t nargs = duk_get_top(ctx);

    if (nargs > 2 || !duk_is_string(ctx, 0) ||
        (nargs == 2 && !duk_is_object(ctx, 1)))
    {
        return ThrowInvalidArgsError(ctx);
    }

    const char* command = duk_get_string(ctx, 0);

    struct
    {
        bool bShowWindow = false;
        bool bVerbose = false;
        const char* cwd = nullptr;
    } options;

    if (duk_is_object(ctx, 1))
    {
        duk_get_prop_string(ctx, 1, "hidden");
        options.bShowWindow = duk_get_boolean_default(ctx, -1, false);
        duk_pop(ctx);

        duk_get_prop_string(ctx, 1, "verbose");
        options.bVerbose = duk_get_boolean_default(ctx, -1, false);
        duk_pop(ctx);

        duk_get_prop_string(ctx, 1, "cwd");
        options.cwd = duk_get_string_default(ctx, -1, nullptr);
        duk_pop(ctx);
    }

    stdstr resultStdOut;
    stdstr resultStdErr;
    DWORD  resultExitCode = EXIT_FAILURE;

    SECURITY_ATTRIBUTES secAttr;
    secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    secAttr.bInheritHandle = TRUE;
    secAttr.lpSecurityDescriptor = nullptr;

    HANDLE hStdIn_r, hStdIn_w;
    HANDLE hStdOut_r, hStdOut_w;
    HANDLE hStdErr_r, hStdErr_w;

    CreatePipe(&hStdIn_r, &hStdIn_w, &secAttr, 0);
    CreatePipe(&hStdOut_r, &hStdOut_w, &secAttr, 0);
    CreatePipe(&hStdErr_r, &hStdErr_w, &secAttr, 0);

    STARTUPINFOA startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdInput = hStdIn_r;
    startupInfo.hStdOutput = hStdOut_w;
    startupInfo.hStdError = hStdErr_w;

    if (!options.bShowWindow)
    {
        startupInfo.dwFlags |= STARTF_USESHOWWINDOW;
        startupInfo.wShowWindow = SW_HIDE;
    }

    PROCESS_INFORMATION processInfo;

    BOOL bSuccess = CreateProcessA(nullptr, (LPSTR)stdstr_f("cmd /c %s", command).c_str(), nullptr, nullptr, TRUE,
        0, nullptr, options.cwd, &startupInfo, &processInfo);

    if (bSuccess)
    {
        CloseHandle(hStdIn_r);
        CloseHandle(hStdIn_w);
        CloseHandle(hStdOut_w);
        CloseHandle(hStdErr_w);

        char buffer[1024];
        DWORD nBytesRead;
        while (ReadFile(hStdOut_r, buffer, sizeof(buffer), &nBytesRead, nullptr) && nBytesRead != 0)
        {
            for (size_t i = 0; i < nBytesRead; i++)
            {
                resultStdOut += buffer[i];
            }

            if (options.bVerbose)
            {
                sys->Print("%.*s", nBytesRead, buffer);
            }
        }

        while (ReadFile(hStdErr_r, buffer, sizeof(buffer), &nBytesRead, nullptr) && nBytesRead != 0)
        {
            for (size_t i = 0; i < nBytesRead; i++)
            {
                resultStdErr += buffer[i];
            }
        }

        CloseHandle(hStdErr_r);
        CloseHandle(hStdOut_r);
        CloseHandle(processInfo.hThread);

        WaitForSingleObject(processInfo.hProcess, INFINITE);
        GetExitCodeProcess(processInfo.hProcess, &resultExitCode);

        CloseHandle(processInfo.hProcess);
    }
    else
    {
        CloseHandle(hStdIn_r);
        CloseHandle(hStdIn_w);
        CloseHandle(hStdOut_r);
        CloseHandle(hStdOut_w);
        CloseHandle(hStdErr_r);
        CloseHandle(hStdErr_w);
    }

    if (!bSuccess || resultExitCode != 0)
    {
        duk_push_error_object(ctx, resultExitCode, "command failed");
        duk_push_number(ctx, resultExitCode);
        duk_put_prop_string(ctx, -2, "status");
        duk_push_string(ctx, resultStdOut.c_str());
        duk_put_prop_string(ctx, -2, "stdout");
        duk_push_string(ctx, resultStdErr.c_str());
        duk_put_prop_string(ctx, -2, "stderr");
        duk_push_number(ctx, processInfo.dwProcessId);
        duk_put_prop_string(ctx, -2, "pid");

        return duk_throw(ctx);
    }
    else
    {
        duk_push_string(ctx, resultStdOut.c_str());
    }

    return 1;
}