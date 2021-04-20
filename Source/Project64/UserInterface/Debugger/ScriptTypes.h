#include <3rdparty/duktape/duktape.h>
#include <3rdparty/duktape/duk_module_duktape.h>
#include <cstdint>
#include <string>

#include "OpInfo.h"

#pragma once

class CScriptSystem;
class CScriptInstance;

typedef std::string jsname_t;

struct JSCallback;
typedef duk_idx_t (*jsargs_fn_t)(duk_context *ctx, void *param);
typedef bool (*jscond_fn_t)(JSCallback* cb, void* param);
typedef void (*jsfn_t)(duk_context* ctx, void *param);

typedef size_t jscb_id_t;
#define JS_INVALID_CALLBACK ((jscb_id_t)(-1))

struct JSCallback
{
    CScriptInstance *instance;
    void            *heapptr;
    jscond_fn_t      Condition;
    jsargs_fn_t      PushArguments;
    jsfn_t           Finish;
    // assigned by scriptsys when this is added to a callback map
    jscb_id_t        id;

    struct {
        uint32_t addrStart, addrEnd;
        union {
            struct { uint32_t opcode, opcodeMask; };
            struct { uint32_t regIndices, regValue; };
        };
    } params;

    JSCallback(CScriptInstance* inst, void* heapptr, jscond_fn_t fnCondition = nullptr,
               jsargs_fn_t fnPushArgs = nullptr, jsfn_t fnFinish = nullptr) :
        instance(inst),
        heapptr(heapptr),
        Condition(fnCondition),
        PushArguments(fnPushArgs),
        Finish(fnFinish),
        id(JS_INVALID_CALLBACK)
    {
        params = {};
    }

    JSCallback() :
        instance(nullptr),
        heapptr(nullptr),
        Condition(nullptr),
        PushArguments(nullptr),
        Finish(nullptr),
        id(JS_INVALID_CALLBACK)
    {
        params = {};
    }
};

typedef struct {
    uint32_t  pc;
    COpInfo   opInfo;
    // below fields are set by the condition function
    int       outAffectedRegIndex;
} jshook_env_cpustep_t;

typedef struct {
    CScriptRenderWindow* scriptRenderWindow;
    void* jsDrawingContext;
} jshook_env_gfxupdate_t;

enum {
    JS_EXEC_TIMEOUT = 500
};

typedef enum {
    JS_HOOK_CPUSTEP,
    JS_HOOK_PIFREAD,
    JS_HOOK_GFXUPDATE,
    JS_NUM_APP_HOOKS
} jshook_id_t;

typedef enum {
    JS_STATUS_STOPPED,
    JS_STATUS_STARTING,
    JS_STATUS_STARTED
} jsstatus_t;

typedef enum {
    CMD_IDLE,
    CMD_START_SCRIPT,
    CMD_STOP_SCRIPT,
    CMD_SWEEP,
    CMD_INPUT,
    CMD_SHUTDOWN,
    CMD_INVOKE
} jssyscmd_id_t;

typedef struct {
    jssyscmd_id_t id;
    stdstr        paramA;
    stdstr        paramB;
    void*         paramC;
    HANDLE        hDoneEvent;
} jssys_cmd_t;
