#include <stdafx.h>
#include "../ScriptAPI.h"

#define MAT4_NUMELEMS  16
#define MAT4_SIZE      (MAT4_NUMELEMS * sizeof(float))
#define VEC3_NUMELEMS  3
#define VEC3_SIZE      (VEC3_NUMELEMS * sizeof(float))

void ScriptAPI::Define_3d(duk_context* ctx)
{
    const duk_function_list_entry vec3_funcs[] = {
        { "create",        vec3_create,        0 },
        { "transformMat4", vec3_transformMat4, 3 },
        { NULL, 0, 0 }
    };

    const duk_function_list_entry mat4_funcs[] = {
        { "create", mat4_create, 0 },
        { "mul",    mat4_mul,    3 },
        { NULL, 0, 0 }
    };

    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, vec3_funcs);
    duk_put_global_string(ctx, "vec3");

    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, mat4_funcs);
    duk_put_global_string(ctx, "mat4");
}

duk_ret_t ScriptAPI::vec3_create(duk_context* ctx)
{
    duk_push_fixed_buffer(ctx, VEC3_SIZE);
    duk_push_buffer_object(ctx, -1, 0, VEC3_SIZE, DUK_BUFOBJ_FLOAT32ARRAY);
    return 1;
}

duk_ret_t ScriptAPI::vec3_transformMat4(duk_context* ctx)
{
    duk_size_t out_size, v3_size, m4_size;
    float* out = (float*)duk_get_buffer_data(ctx, 0, &out_size);
    float* v3 = (float*)duk_get_buffer_data(ctx, 1, &v3_size);
    float* m4 = (float*)duk_get_buffer_data(ctx, 2, &m4_size);

    if (out_size != VEC3_SIZE ||
        v3_size != VEC3_SIZE ||
        m4_size != MAT4_SIZE)
    {
        return DUK_RET_TYPE_ERROR;
    }

    float ix = v3[0];
    float iy = v3[1];
    float iz = v3[2];

    float ox = (m4[0] * ix) + (m4[4] * iy) + (m4[8] * iz) + m4[12];
    float oy = (m4[1] * ix) + (m4[5] * iy) + (m4[9] * iz) + m4[13];
    float oz = (m4[2] * ix) + (m4[6] * iy) + (m4[10] * iz) + m4[14];
    float ow = (m4[3] * ix) + (m4[7] * iy) + (m4[11] * iz) + m4[15];

    out[0] = ow == 0.0f ? 0 : ox / ow;
    out[1] = ow == 0.0f ? 0 : oy / ow;
    out[2] = ow == 0.0f ? 0 : oz / ow;

    return 0;
}

duk_ret_t ScriptAPI::mat4_create(duk_context* ctx)
{
    duk_push_fixed_buffer(ctx, MAT4_SIZE);
    duk_size_t unused;
    float* m4 = (float*)duk_get_buffer_data(ctx, -1, &unused);

    m4[0] = 1.0f;
    m4[5] = 1.0f;
    m4[10] = 1.0f;
    m4[15] = 1.0f;

    duk_push_buffer_object(ctx, -1, 0, MAT4_SIZE, DUK_BUFOBJ_FLOAT32ARRAY);
    return 1;
}

duk_ret_t ScriptAPI::mat4_mul(duk_context* ctx)
{
    duk_size_t out_size, a_size, b_size;

    float* out = (float*)duk_get_buffer_data(ctx, 0, &out_size);
    float* a = (float*)duk_get_buffer_data(ctx, 1, &a_size);
    float* b = (float*)duk_get_buffer_data(ctx, 2, &b_size);

    if (out_size != MAT4_SIZE ||
        a_size != MAT4_SIZE ||
        a_size != MAT4_SIZE)
    {
        return DUK_RET_TYPE_ERROR;
    }

    float t[MAT4_NUMELEMS];

    t[0] = (a[0] * b[0]) + (a[1] * b[4]) + (a[2] * b[8]) + (a[3] * b[12]);
    t[1] = (a[0] * b[1]) + (a[1] * b[5]) + (a[2] * b[9]) + (a[3] * b[13]);
    t[2] = (a[0] * b[2]) + (a[1] * b[6]) + (a[2] * b[10]) + (a[3] * b[14]);
    t[3] = (a[0] * b[3]) + (a[1] * b[7]) + (a[2] * b[11]) + (a[3] * b[15]);
    t[4] = (a[4] * b[0]) + (a[5] * b[4]) + (a[6] * b[8]) + (a[7] * b[12]);
    t[5] = (a[4] * b[1]) + (a[5] * b[5]) + (a[6] * b[9]) + (a[7] * b[13]);
    t[6] = (a[4] * b[2]) + (a[5] * b[6]) + (a[6] * b[10]) + (a[7] * b[14]);
    t[7] = (a[4] * b[3]) + (a[5] * b[7]) + (a[6] * b[11]) + (a[7] * b[15]);
    t[8] = (a[8] * b[0]) + (a[9] * b[4]) + (a[10] * b[8]) + (a[11] * b[12]);
    t[9] = (a[8] * b[1]) + (a[9] * b[5]) + (a[10] * b[9]) + (a[11] * b[13]);
    t[10] = (a[8] * b[2]) + (a[9] * b[6]) + (a[10] * b[10]) + (a[11] * b[14]);
    t[11] = (a[8] * b[3]) + (a[9] * b[7]) + (a[10] * b[11]) + (a[11] * b[15]);
    t[12] = (a[12] * b[0]) + (a[13] * b[4]) + (a[14] * b[8]) + (a[15] * b[12]);
    t[13] = (a[12] * b[1]) + (a[13] * b[5]) + (a[14] * b[9]) + (a[15] * b[13]);
    t[14] = (a[12] * b[2]) + (a[13] * b[6]) + (a[14] * b[10]) + (a[15] * b[14]);
    t[15] = (a[12] * b[3]) + (a[13] * b[7]) + (a[14] * b[11]) + (a[15] * b[15]);

    memcpy(out, t, sizeof(t));
    return 0;
}
