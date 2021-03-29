#include <3rdparty/duktape/duktape.h>
#include <cstdint>
#include <string>

#pragma once

class CScriptSystem;
class CScriptInstance;

typedef std::string jsname_t;

struct jscallback_t;
typedef duk_idx_t (*jsargs_fn_t)(duk_context *ctx, void *param);
typedef bool (*jscond_fn_t)(struct jscallback_t *cb, void *param);

typedef size_t jscb_id_t;
#define JS_INVALID_CALLBACK ((jscb_id_t)(-1))

typedef struct jscallback_t
{
    CScriptInstance *inst;
    void         *heapptr;
    jsargs_fn_t   fnPushArgs;
    jscond_fn_t   fnCondition;

    struct {
        uint32_t addrStart, addrEnd;
        union {
            struct { uint32_t opcode, opcodeMask; };
            struct { uint32_t regIndices, regValue; };
        };
    } cond;
} jscallback_t;

typedef struct {
    uint32_t  pc;
    uint32_t  opcode;
    bool      bReadOp, bWriteOp;
    uint32_t  readWriteAddr;
    uint32_t  readWriteValue;
    uint32_t  *gpr; // TODO this will need to be changed
    // below fields are set by the condition function
    int       outAffectedRegIndex;
} jshook_env_cpustep_t;

enum {
    JS_EXEC_TIMEOUT = 500
};

typedef enum {
    JS_HOOK_CPUSTEP,
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
    CMD_EVAL,
    CMD_SHUTDOWN
} jssyscmd_id_t;
