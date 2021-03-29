#include <stdafx.h>
//#include "Server.h"
#include "../ScriptAPI.h"

/*
void ScriptAPI::Define_Server(duk_context *ctx)
{
    duk_push_c_function(ctx, js_Server__constructor, 0);
    duk_push_object(ctx);

    duk_function_list_entry funcs[] = {
        { "address", js_Server_address, 0 },
        { "on", js_Server_on, 2 },
        { NULL, NULL, 0 }
    };

    duk_put_function_list(ctx, -1, funcs);

    duk_put_prop_string(ctx, -2, "prototype");
    duk_put_global_string(ctx, "Server");
}

duk_ret_t ScriptAPI::js_Server__constructor(duk_context *ctx)
{
    if (!duk_is_constructor_call(ctx))
    {
        return DUK_RET_TYPE_ERROR;
    }

    duk_push_this(ctx);

    void *js_object = duk_get_heapptr(ctx, -1);
    void *c_object = new Server(ctx, js_object);

    duk_push_pointer(ctx, c_object);
    duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("c_object"));

    duk_push_object(ctx);
    duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("callbacks"));

    duk_push_c_function(ctx, js_Server__finalizer, 1);
    duk_set_finalizer(ctx, -2);

    duk_pop(ctx);
    return 0;
}

duk_ret_t ScriptAPI::js_Server__finalizer(duk_context *ctx)
{
    duk_get_prop_string(ctx, 0, DUK_HIDDEN_SYMBOL("c_object"));

    void *ptr = duk_get_pointer(ctx, -1);

    Server *server = (Server *)ptr;
    delete server;

    duk_pop_n(ctx, 2);
    return 0;
}

duk_ret_t ScriptAPI::js_Server_address(duk_context *ctx)
{
    duk_push_uint(ctx, 0xAABBCCDD);
    return 1;
}

duk_ret_t ScriptAPI::js_Server_on(duk_context *ctx)
{
    duk_idx_t nargs = duk_get_top(ctx);

    if(nargs < 2 || !duk_is_string(ctx, -2) || !duk_is_function(ctx, -1))
    {
        duk_push_error_object(ctx, DUK_ERR_TYPE_ERROR, "invalid arguments");
        return duk_throw(ctx);
    }

    bool bEventIdValid = false;
    const char *eventId = duk_get_string(ctx, -2);
    const char *validEventIds[] = { "connection", "close", "error", NULL };

    for(size_t i = 0; validEventIds[i] != NULL; i++)
    {
        if(strcmp(validEventIds[i], eventId) == 0)
        {
            bEventIdValid = true;
            break;
        }
    }

    if(!bEventIdValid)
    {
        duk_push_error_object(ctx, DUK_ERR_ERROR, "invalid event id '%s'", eventId);
        return duk_throw(ctx);
    }

    duk_push_this(ctx); // [eventId callback this]
    duk_get_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("callbacks")); // [eventId callback this callbacks]
    duk_pull(ctx, -4);
    duk_pull(ctx, -4); // [this callbacks eventId callback]
    duk_put_prop(ctx, -3); // [this callbacks]
    duk_pop_n(ctx, 2);

    return 0;
}
*/