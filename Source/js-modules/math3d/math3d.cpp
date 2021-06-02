#define DUK_USE_PROJECT64_SETTINGS
#include <duktape/duktape.h>
#include <cstring>

/*

Modeled after glMatrix (https://glmatrix.net/docs/module-glMatrix.html)

vec2.create(): vec2
vec2.fromValues(x: number, y: number): vec2

vec3.create(): vec3
vec3.fromValues(x: number, y: number, z: number): vec3
vec3.dot(a: vec3, b: vec3): number
vec3.transformMat4(out: vec3, v3: vec3, m4: mat4): void

vec4.create(): vec4
vec4.fromValues(x: number, y: number, z: number, w: number): vec4
vec4.transformMat4(out: vec4, v4: vec4, m4: mat4): void
vec4.perspDivide(out: vec3, v4: vec4): void

math3d.barycentric(bcCoords3Out: vec3, point2: vec2, a: vec2, b: vec2, c: vec2): void
math3d.bcInside(bcCoords3: vec3): boolean

*/

extern "C" {
    __declspec(dllexport) duk_ret_t dukopen_math3d(duk_context* ctx);
}

int DukTimeoutCheck(void*) { return 0; }

#define VEC2 2
#define VEC3 3
#define VEC4 4
#define MAT4 16

#define VEC2_SIZE (VEC2 * sizeof(float))
#define VEC3_SIZE (VEC3 * sizeof(float))
#define VEC4_SIZE (VEC4 * sizeof(float))
#define MAT4_SIZE (MAT4 * sizeof(float))

static duk_ret_t vec2_create(duk_context* ctx);
static duk_ret_t vec2_fromValues(duk_context* ctx);

static duk_ret_t vec3_create(duk_context* ctx);
static duk_ret_t vec3_fromValues(duk_context* ctx);
static duk_ret_t vec3_transformMat4(duk_context* ctx);
static duk_ret_t vec3_dot(duk_context* ctx);

static duk_ret_t vec4_create(duk_context* ctx);
static duk_ret_t vec4_fromValues(duk_context* ctx);
static duk_ret_t vec4_transformMat4(duk_context* ctx);
static duk_ret_t vec4_perspDivide(duk_context* ctx);

static duk_ret_t mat4_create(duk_context* ctx);
static duk_ret_t mat4_mul(duk_context* ctx);

static duk_ret_t barycentric(duk_context* ctx);
static duk_ret_t bcInside(duk_context* ctx);

float* duk_require_float_array(duk_context* ctx, duk_idx_t idx, int num_elems);
float* duk_push_float_array(duk_context* ctx, int num_elems);

static const duk_function_list_entry vec2_funcs[] = {
    { "create",        vec2_create,        0 },
    { "fromValues",    vec2_fromValues,    2 },
    { nullptr, 0, 0 }
};

static const duk_function_list_entry vec3_funcs[] = {
    { "create",        vec3_create,        0 },
    { "fromValues",    vec3_fromValues,    3 },
    { "transformMat4", vec3_transformMat4, 3 },
    { "dot",           vec3_dot,           2 },
    { nullptr, 0, 0 }
};

static const duk_function_list_entry vec4_funcs[] = {
    { "create",        vec4_create,        0 },
    { "fromValues",    vec4_fromValues,    4 },
    { "transformMat4", vec4_transformMat4, 3 },
    { nullptr, 0, 0 }
};

static const duk_function_list_entry mat4_funcs[] = {
    { "create", mat4_create, 0 },
    { "mul",    mat4_mul,    3 },
    { nullptr, 0, 0 }
};

static const duk_function_list_entry math3d_funcs[] = {
    { "barycentric", barycentric, 5 },
    { "bcInside",    bcInside, 1 },
    { nullptr, 0, 0 }
};

duk_ret_t dukopen_math3d(duk_context* ctx)
{
    duk_push_object(ctx);

    duk_put_function_list(ctx, -1, math3d_funcs);
    
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, vec2_funcs);
    duk_put_prop_string(ctx, -2, "vec2");
    
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, vec3_funcs);
    duk_put_prop_string(ctx, -2, "vec3");
    
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, vec4_funcs);
    duk_put_prop_string(ctx, -2, "vec4");
    
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, mat4_funcs);
    duk_put_prop_string(ctx, -2, "mat4");

    return 1;
}

duk_ret_t vec2_create(duk_context* ctx)
{
    duk_push_float_array(ctx, VEC2);
    return 1;
}

duk_ret_t vec2_fromValues(duk_context* ctx)
{
    float* v2 = duk_push_float_array(ctx, VEC2);

    v2[0] = (float)duk_require_number(ctx, 0);
    v2[1] = (float)duk_require_number(ctx, 1);

    return 1;
}

duk_ret_t vec3_create(duk_context* ctx)
{
    duk_push_float_array(ctx, VEC3);
    return 1;
}

duk_ret_t vec3_fromValues(duk_context* ctx)
{
    float* v3 = duk_push_float_array(ctx, VEC3);

    v3[0] = (float)duk_require_number(ctx, 0);
    v3[1] = (float)duk_require_number(ctx, 1);
    v3[2] = (float)duk_require_number(ctx, 2);

    return 1;
}

static duk_ret_t vec3_dot(duk_context* ctx)
{
    float* a = duk_require_float_array(ctx, 0, VEC3);
    float* b = duk_require_float_array(ctx, 1, VEC3);

    float dp = (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);

    duk_push_number(ctx, dp);
    return 1;
}

duk_ret_t vec3_transformMat4(duk_context* ctx)
{
    float* out = duk_require_float_array(ctx, 0, VEC3);
    float* v3 = duk_require_float_array(ctx, 1, VEC3);
    float* m4 = duk_require_float_array(ctx, 2, MAT4);

    float ix = v3[0];
    float iy = v3[1];
    float iz = v3[2];

    float ox = (m4[0] * ix) + (m4[4] * iy) + (m4[ 8] * iz) + m4[12];
    float oy = (m4[1] * ix) + (m4[5] * iy) + (m4[ 9] * iz) + m4[13];
    float oz = (m4[2] * ix) + (m4[6] * iy) + (m4[10] * iz) + m4[14];
    float ow = (m4[3] * ix) + (m4[7] * iy) + (m4[11] * iz) + m4[15];

    out[0] = ow == 0.0f ? 0 : ox / ow;
    out[1] = ow == 0.0f ? 0 : oy / ow;
    out[2] = ow == 0.0f ? 0 : oz / ow;

    return 0;
}

duk_ret_t vec4_create(duk_context* ctx)
{
    duk_push_float_array(ctx, VEC4);
    return 1;
}

duk_ret_t vec4_fromValues(duk_context* ctx)
{
    float* v4 = duk_push_float_array(ctx, VEC4);

    v4[0] = (float)duk_require_number(ctx, 0);
    v4[1] = (float)duk_require_number(ctx, 1);
    v4[2] = (float)duk_require_number(ctx, 2);
    v4[3] = (float)duk_require_number(ctx, 3);

    return 1;
}

duk_ret_t vec4_transformMat4(duk_context* ctx)
{
    float* out = duk_require_float_array(ctx, 0, VEC4);
    float* v4 = duk_require_float_array(ctx, 1, VEC4);
    float* m4 = duk_require_float_array(ctx, 2, MAT4);

    float ix = v4[0];
    float iy = v4[1];
    float iz = v4[2];
    float iw = v4[3];

    float ox = (m4[0] * ix) + (m4[4] * iy) + (m4[ 8] * iz) + (m4[12] * iw);
    float oy = (m4[1] * ix) + (m4[5] * iy) + (m4[ 9] * iz) + (m4[13] * iw);
    float oz = (m4[2] * ix) + (m4[6] * iy) + (m4[10] * iz) + (m4[14] * iw);
    float ow = (m4[3] * ix) + (m4[7] * iy) + (m4[11] * iz) + (m4[15] * iw);

    out[0] = ox;
    out[1] = oy;
    out[2] = oz;
    out[3] = ow;

    return 0;
}

static duk_ret_t vec4_perspDivide(duk_context* ctx)
{
    float* out = duk_require_float_array(ctx, 0, VEC3);
    float* v4 = duk_require_float_array(ctx, 1, VEC4);

    out[0] = v4[0] / v4[3];
    out[1] = v4[1] / v4[3];
    out[2] = v4[2] / v4[3];

    return 0;
}

duk_ret_t mat4_create(duk_context* ctx)
{
    float* m4 = duk_push_float_array(ctx, MAT4);

    m4[ 0] = 1.0f;
    m4[ 5] = 1.0f;
    m4[10] = 1.0f;
    m4[15] = 1.0f;

    return 1;
}

duk_ret_t mat4_mul(duk_context* ctx)
{
    float* out = duk_require_float_array(ctx, 0, MAT4);
    float* a = duk_require_float_array(ctx, 1, MAT4);
    float* b = duk_require_float_array(ctx, 2, MAT4);

    float t[16];

    t[ 0] = (a[ 0] * b[0]) + (a[ 1] * b[4]) + (a[ 2] * b[ 8]) + (a[ 3] * b[12]);
    t[ 1] = (a[ 0] * b[1]) + (a[ 1] * b[5]) + (a[ 2] * b[ 9]) + (a[ 3] * b[13]);
    t[ 2] = (a[ 0] * b[2]) + (a[ 1] * b[6]) + (a[ 2] * b[10]) + (a[ 3] * b[14]);
    t[ 3] = (a[ 0] * b[3]) + (a[ 1] * b[7]) + (a[ 2] * b[11]) + (a[ 3] * b[15]);
    t[ 4] = (a[ 4] * b[0]) + (a[ 5] * b[4]) + (a[ 6] * b[ 8]) + (a[ 7] * b[12]);
    t[ 5] = (a[ 4] * b[1]) + (a[ 5] * b[5]) + (a[ 6] * b[ 9]) + (a[ 7] * b[13]);
    t[ 6] = (a[ 4] * b[2]) + (a[ 5] * b[6]) + (a[ 6] * b[10]) + (a[ 7] * b[14]);
    t[ 7] = (a[ 4] * b[3]) + (a[ 5] * b[7]) + (a[ 6] * b[11]) + (a[ 7] * b[15]);
    t[ 8] = (a[ 8] * b[0]) + (a[ 9] * b[4]) + (a[10] * b[ 8]) + (a[11] * b[12]);
    t[ 9] = (a[ 8] * b[1]) + (a[ 9] * b[5]) + (a[10] * b[ 9]) + (a[11] * b[13]);
    t[10] = (a[ 8] * b[2]) + (a[ 9] * b[6]) + (a[10] * b[10]) + (a[11] * b[14]);
    t[11] = (a[ 8] * b[3]) + (a[ 9] * b[7]) + (a[10] * b[11]) + (a[11] * b[15]);
    t[12] = (a[12] * b[0]) + (a[13] * b[4]) + (a[14] * b[ 8]) + (a[15] * b[12]);
    t[13] = (a[12] * b[1]) + (a[13] * b[5]) + (a[14] * b[ 9]) + (a[15] * b[13]);
    t[14] = (a[12] * b[2]) + (a[13] * b[6]) + (a[14] * b[10]) + (a[15] * b[14]);
    t[15] = (a[12] * b[3]) + (a[13] * b[7]) + (a[14] * b[11]) + (a[15] * b[15]);

    memcpy(out, t, sizeof(t));
    return 0;
}

static duk_ret_t barycentric(duk_context* ctx)
{
    float* out = duk_require_float_array(ctx, 0, VEC3);
    float* point2 = duk_require_float_array(ctx, 1, VEC2);
    float* a = duk_require_float_array(ctx, 2, VEC2);
    float* b = duk_require_float_array(ctx, 3, VEC2);
    float* c = duk_require_float_array(ctx, 4, VEC2);

    float ax = a[0];
    float ay = a[1];
    float x0 = b[0] - ax;
    float y0 = b[1] - ay;
    float x1 = c[0] - ax;
    float y1 = c[1] - ay;
    float x2 = point2[0] - ax;
    float y2 = point2[1] - ay;
    float den = x0 * y1 - x1 * y0;
    float v = (x2 * y1 - x1 * y2) / den;
    float w = (x0 * y2 - x2 * y0) / den;
    float u = 1.0f - v - w;

    out[0] = u;
    out[1] = v;
    out[2] = w;

    return 0;
}

static duk_ret_t bcInside(duk_context* ctx)
{
    float* bcCoords3 = duk_require_float_array(ctx, 0, VEC3);
    bool bInside = (bcCoords3[0] >= 0) && (bcCoords3[1] >= 0) && (bcCoords3[0] + bcCoords3[1] < 1);
    duk_push_boolean(ctx, bInside ? 1 : 0);
    return 1;
}

inline float* duk_require_float_array(duk_context* ctx, duk_idx_t idx, int num_elems)
{
    if (!duk_is_buffer_data(ctx, idx))
    {
        duk_push_error_object(ctx, DUK_ERR_TYPE_ERROR, "param %d is not a buffer type", idx);
        (void)duk_throw(ctx);
    }

    duk_size_t size;
    float* p = (float*)duk_get_buffer_data(ctx, idx, &size);

    if (size < num_elems * sizeof(float))
    {
        duk_push_error_object(ctx, DUK_ERR_RANGE_ERROR, "param %d size incorrect", idx);
        (void)duk_throw(ctx);
    }

    return p;
}


inline float* duk_push_float_array(duk_context* ctx, int num_elems)
{
    float* arr = (float*)duk_push_fixed_buffer(ctx, num_elems * sizeof(float));
    duk_push_buffer_object(ctx, -1, 0, num_elems * sizeof(float), DUK_BUFOBJ_FLOAT32ARRAY);
    duk_remove(ctx, -2);
    return arr;
}
