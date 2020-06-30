#pragma once

#include <stdafx.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159f
#endif

class CMtx;
class CVec4;
class CTri;
class CPlane;
class CProjection;
class CBasicMeshGeometry;

class CMtx
{
public:
    float m[4][4];
    CMtx(void);
};

class CVec4
{
public:
    float x, y, z, w;
    CVec4(void);
    CVec4(float x, float y, float z, float w = 1.0f);
    void Multiply(CMtx* mtx, CVec4 *result);
    void Multiply(CVec4* otherVec, CVec4 *result);
    void Multiply(float val, CVec4 *result);
    void Divide(float val, CVec4 *result);
    void PerspectiveDivide(CVec4 *result);
    void Translate(float x, float y, float z, CVec4 *result);
    void Scale(float x, float y, float z, CVec4 *result);
    void RotateX(float degrees, CVec4 *result);
    void RotateY(float degrees, CVec4 *result);
    void RotateZ(float degrees, CVec4 *result);
    void Subtract(CVec4 *otherVec, CVec4 *result);
    void Add(CVec4 *otherVec, CVec4 *result);
    float DotProduct(CVec4 *otherVec);
    void CrossProduct(CVec4 *otherVec, CVec4 *result);
    void Normalize(CVec4 *result);
};

class CTri
{
public:
    CVec4 v[3];

    CTri(void);
 
    COLORREF m_Color;
    bool m_Selected;
    int m_ClickIndex;

    void Multiply(CMtx *mtx, CTri *result);
    void Multiply(CVec4 *vec, CTri *result);
    void PerspectiveDivide(CTri *result);
    void Translate(float x, float y, float z, CTri *result);
    void RotateX(float degrees, CTri *result);
    void RotateY(float degrees, CTri *result);
    void RotateZ(float degrees, CTri *result);
    void YSortPoints(CTri *result);
    void CalculateNormal(CVec4 *result);
    void Center(CVec4 *result);
    void Weigh2d(float x, float y, CVec4 *weights);
    void Clip(std::vector<CPlane>& clippingPlanes, std::vector<CTri>& trisOut);
};

class CPlane
{
public:
    CPlane(void);
    CPlane(float pointX, float pointY, float pointZ, float normalX, float normalY, float normalZ);
    CVec4 m_Point, m_Normal;
    float DistanceToPoint(CVec4 &point);
    float Intersect(CVec4 lineStart, CVec4 lineEnd, CVec4 *point);
};

class CProjection
{
private:
    CProjection(void);
    float m_Near, m_Far, m_Fov, m_AspectRatio;
    std::vector<CPlane> m_ClippingPlanes;
public:
    CProjection(float fnear, float ffar, float fov, float aspectRatio);
    void GetMtx(CMtx *out);
    std::vector<CPlane>& GetClippingPlanes(void);
};

class CCamera
{
public:
    CCamera(void);
    CVec4 m_Pos, m_Rot;
    void GetViewMatrix(CMtx *out);
    void GetDirections(CVec4 *forward, CVec4 *up, CVec4 *right);
    void RotateY(float y);
    void RotateX(float x);
    void TranslateX(float x);
    void TranslateY(float y);
    void TranslateZ(float z);
    void SetPos(float x, float y, float z);
    void SetRot(float x, float y, float z);
};

typedef struct
{
    size_t vidx0, vidx1, vidx2;
    bool bSelected;
    int clickIndex;
    size_t nCommand;
} geom_tri_ref_t;

class CBasicMeshGeometry
{
public:
    CBasicMeshGeometry(void);
    int m_SelectedTriIdx;
    std::vector<CVec4> m_Vertices;
    std::vector<geom_tri_ref_t> m_TriangleRefs;
    void AddVertex(CVec4 vertex);
    size_t AddVertexUnique(CVec4 vertex);
    void AddTriangleRef(size_t vidx0, size_t vidx1, size_t vidx2, size_t nCommand);
    void AddTriangleRef(geom_tri_ref_t triref);
    void AddVertices(CVec4 vertices[], size_t count);
    void AddTriangleRefs(geom_tri_ref_t trirefs[], size_t count);
    size_t GetNumTriangles(void);
    bool GetTriangle(CTri *out, size_t index);
    void Clear(void);
};

class CGeometryInstance
{
    CMtx m_Matrix;
    uint32_t m_GeometryKey; // key of scene.m_Geometries
    CGeometryInstance* m_Parent;
    std::vector<CGeometryInstance> m_Children;
};

/*
class CScene
{
    // use dl address
    std::map<uint32_t, CBasicMeshGeometry> m_Geometries;
    //CGeometryInstance m_RootInstance;
public:
    bool AddGeometry(uint32_t dlAddr);
};*/

class CDrawBuffers
{
private:
    std::vector<CPlane> m_ClippingPlanes;
    bool Inside(int x, int y);
    int Index(int x, int y);
public:
    CDrawBuffers(int width, int height);
    ~CDrawBuffers(void);

    uint32_t *m_ColorBuffer;
    float *m_DepthBuffer;
    int *m_SelectBuffer;
    int m_Width, m_Height;

    void SetColor(int x, int y, uint32_t color);
    float GetDepth(int x, int y);
    void SetDepth(int x, int y, float w);
    int GetSelect(int x, int y);
    void SetSelect(int x, int y, int key);
    
    void Clear(void);
    void DrawTriangle(CTri &tri, uint32_t clickIndex);
    void DrawLine(CVec4& p0, CVec4& p1, uint32_t color);
    void FillRect(int x, int y, int w, int h, uint32_t color);
    void Render(CBasicMeshGeometry *geom, CCamera *camera);

    inline static uint32_t Color(uint8_t r, uint8_t g, uint8_t b);
};