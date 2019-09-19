#include <stdafx.h>
#include "GfxRenderer.h"

CMtx::CMtx(void)
{
    memset(m, 0, sizeof(m));
}

/********************/

CVec4::CVec4(void) :
    x(0.0f), y(0.0f), z(0.0f), w(1.0f)
{
}

CVec4::CVec4(float x, float y, float z, float w) :
    x(x), y(y), z(z), w(w)
{
}

void CVec4::Multiply(CVec4* otherVec, CVec4 *result)
{
    result->x = x * otherVec->x;
    result->y = y * otherVec->y;
    result->z = z * otherVec->z;
}

void CVec4::Multiply(CMtx* mtx, CVec4 *result)
{
    float rx = (x * mtx->m[0][0]) + (y * mtx->m[1][0]) + (z * mtx->m[2][0]) + (w * mtx->m[3][0]);
    float ry = (x * mtx->m[0][1]) + (y * mtx->m[1][1]) + (z * mtx->m[2][1]) + (w * mtx->m[3][1]);
    float rz = (x * mtx->m[0][2]) + (y * mtx->m[1][2]) + (z * mtx->m[2][2]) + (w * mtx->m[3][2]);
    float rw = (x * mtx->m[0][3]) + (y * mtx->m[1][3]) + (z * mtx->m[2][3]) + (w * mtx->m[3][3]);

    result->x = rx;
    result->y = ry;
    result->z = rz;
    result->w = rw;
}

void CVec4::Multiply(float val, CVec4 *result)
{
    result->x = x * val;
    result->y = y * val;
    result->z = z * val;
}

void CVec4::Divide(float val, CVec4 *result)
{
    result->x = x / val;
    result->y = y / val;
    result->z = z / val;
}

void CVec4::PerspectiveDivide(CVec4 *result)
{
    if (w == 0.0f)
    {
        *result = *this;
    }

    Divide(w, result);
}

void CVec4::Translate(float _x, float _y, float _z, CVec4 *result)
{
    result->x = x + _x;
    result->y = y + _y;
    result->z = z + _z;
}

void CVec4::Scale(float _x, float _y, float _z, CVec4 *result)
{
    CMtx mtx;
    mtx.m[0][0] = _x;
    mtx.m[1][1] = _y;
    mtx.m[2][2] = _z;
    mtx.m[3][3] = 1.0f;
    Multiply(&mtx, result);
}

void CVec4::RotateX(float degrees, CVec4 *result)
{
    float rads = degrees * (M_PI / 180.0f);

    CMtx mtx;
    mtx.m[0][0] = 1.0f;
    mtx.m[1][1] = cosf(rads);
    mtx.m[1][2] = sinf(rads);
    mtx.m[2][1] = -sinf(rads);
    mtx.m[2][2] = cosf(rads);
    mtx.m[3][3] = 1.0f;
    Multiply(&mtx, result);
}

void CVec4::RotateY(float degrees, CVec4 *result)
{
    float rads = degrees * (M_PI / 180.0f);

    CMtx mtx;
    memset(&mtx, 0, sizeof(mtx));

    mtx.m[0][0] = cosf(rads);
    mtx.m[0][2] = sinf(rads);
    mtx.m[1][1] = 1.0f;
    mtx.m[2][0] = -sinf(rads);
    mtx.m[2][2] = cosf(rads);
    mtx.m[3][3] = 1.0f;
    Multiply(&mtx, result);
}

void CVec4::RotateZ(float degrees, CVec4 *result)
{
    float rads = degrees * (M_PI / 180.0f);

    CMtx mtx;
    mtx.m[0][0] = cosf(rads);
    mtx.m[0][1] = sinf(rads);
    mtx.m[1][0] = -sinf(rads);
    mtx.m[1][1] = cosf(rads);
    mtx.m[2][2] = 1.0f;
    mtx.m[3][3] = 1.0f;

    this->Multiply(&mtx, result);
}

float CVec4::DotProduct(CVec4 *otherVec)
{
    return (otherVec->x * x) +
           (otherVec->y * y) +
           (otherVec->z * z);
}

void CVec4::CrossProduct(CVec4 *otherVec, CVec4 *result)
{
    result->x = (y * otherVec->z) - (z * otherVec->y);
    result->y = (z * otherVec->x) - (x * otherVec->z);
    result->z = (x * otherVec->y) - (y * otherVec->x);
}

void CVec4::Subtract(CVec4 *otherVec, CVec4 *result)
{
    result->x = x - otherVec->x;
    result->y = y - otherVec->y;
    result->z = z - otherVec->z;
}

void CVec4::Add(CVec4 *otherVec, CVec4 *result)
{
    result->x = x + otherVec->x;
    result->y = y + otherVec->y;
    result->z = z + otherVec->z;
}

void CVec4::Normalize(CVec4 *result)
{
    float length = sqrtf((x * x) + (y * y) + (z * z));
    result->x = x / length;
    result->y = y / length;
    result->z = z / length;
}

/********************/

CPlane::CPlane(float pointX, float pointY, float pointZ, float normalX, float normalY, float normalZ) :
    m_Point(pointX, pointY, pointZ),
    m_Normal(normalX, normalY, normalZ)
{
}

CPlane::CPlane(void) :
    m_Point(0, 0, 0),
    m_Normal(0, 0, 0)
{
}

float CPlane::DistanceToPoint(CVec4& point)
{
    CVec4 pn;
    m_Normal.Normalize(&pn);
    return pn.DotProduct(&point) - pn.DotProduct(&m_Point);
}

float CPlane::Intersect(CVec4 lineStart, CVec4 lineEnd, CVec4 *out)
{
    CVec4 lineStartToEnd, lineToIntersect;

    float pd = m_Point.DotProduct(&m_Normal);
    float ad = lineStart.DotProduct(&m_Normal);
    float bd = lineEnd.DotProduct(&m_Normal);
    float t = (pd - ad) / (bd - ad);

    lineEnd.Subtract(&lineStart, &lineStartToEnd);
    lineStartToEnd.Multiply(t, &lineToIntersect);
    lineStart.Add(&lineToIntersect, out);

    return t;
}

/********************/

CTri::CTri(void):
    m_Color(RGB(255, 255, 255))
{
}

void CTri::Multiply(CMtx *mtx, CTri *out)
{
    v[0].Multiply(mtx, &out->v[0]);
    v[1].Multiply(mtx, &out->v[1]);
    v[2].Multiply(mtx, &out->v[2]);
}

void CTri::Multiply(CVec4 *vec, CTri *out)
{
    v[0].Multiply(vec, &out->v[0]);
    v[1].Multiply(vec, &out->v[1]);
    v[2].Multiply(vec, &out->v[2]);
}

void CTri::Translate(float x, float y, float z, CTri *out)
{
    v[0].Translate(x, y, z, &out->v[0]);
    v[1].Translate(x, y, z, &out->v[1]);
    v[2].Translate(x, y, z, &out->v[2]);
}

void CTri::PerspectiveDivide(CTri *out)
{
    v[0].PerspectiveDivide(&out->v[0]);
    v[1].PerspectiveDivide(&out->v[1]);
    v[2].PerspectiveDivide(&out->v[2]);
}

void CTri::RotateX(float degrees, CTri *out)
{
    v[0].RotateX(degrees, &out->v[0]);
    v[1].RotateX(degrees, &out->v[1]);
    v[2].RotateX(degrees, &out->v[2]);
}

void CTri::RotateY(float degrees, CTri *out)
{
    v[0].RotateY(degrees, &out->v[0]);
    v[1].RotateY(degrees, &out->v[1]);
    v[2].RotateY(degrees, &out->v[2]);
}

void CTri::RotateZ(float degrees, CTri *out)
{
    v[0].RotateZ(degrees, &out->v[0]);
    v[1].RotateZ(degrees, &out->v[1]);
    v[2].RotateZ(degrees, &out->v[2]);
}

void CTri::YSortPoints(CTri *out)
{
    *out = *this;
    CVec4* ov = out->v;
    CVec4 t;
    
    if (ov[0].y > ov[2].y)
    {
        t = ov[0]; ov[0] = ov[2]; ov[2] = t;
    }

    if (ov[0].y > ov[1].y)
    {
        t = ov[0]; ov[0] = ov[1]; ov[1] = t;
    }

    if (ov[1].y > ov[2].y)
    {
        t = ov[1]; ov[1] = ov[2]; ov[2] = t;
    }
}

void CTri::CalculateNormal(CVec4 *out)
{
    CVec4 lineA, lineB;
    v[1].Subtract(&v[0], &lineA);
    v[2].Subtract(&v[0], &lineB);
    lineA.CrossProduct(&lineB, out);
    out->Normalize(out);
}

void CTri::Center(CVec4 *out)
{
    float x = (v[0].x + v[1].x + v[2].x) / 3.0f;
    float y = (v[0].y + v[1].y + v[2].y) / 3.0f;
    float z = (v[0].z + v[1].z + v[2].z) / 3.0f;
    out->x = x;
    out->y = y;
    out->z = z;
}

void CTri::Weigh2d(float x, float y, CVec4 *weights)
{
    float t0 = (v[1].y - v[2].y);
    float t1 = (x - v[2].x);
    float t2 = (v[2].x - v[1].x);
    float t3 = (y - v[2].y);
    float t4 = (v[0].x - v[2].x);
    float t5 = (v[0].y - v[2].y);
    float t6 = (v[2].y - v[0].y);

    float denom = (t0 * t4 + t2 * t5);

    float w1 = (t0 * t1 + t2 * t3) / denom;
    float w2 = (t6 * t1 + t4 * t3) / denom;
    float w3 = 1 - w1 - w2;

    if (w1 < 0) w1 = 0;
    if (w2 < 0) w2 = 0;
    if (w3 < 0) w3 = 0;

    weights->x = w1;
    weights->y = w2;
    weights->z = w3;
}

void CTri::Clip(std::vector<CPlane>& clippingPlanes, std::vector<CTri>& trisOut)
{
    std::vector<CTri> outputTris;
    outputTris.push_back(*this);

    for (int nPlane = 0; nPlane < clippingPlanes.size(); nPlane++)
    {
        std::vector<CTri> inputTris = outputTris;
        outputTris.clear();

        for (size_t nTri = 0; nTri < inputTris.size(); nTri++)
        {
            std::vector<int> insideIndeces;
            std::vector<int> outsideIndeces;

            for (int i = 0; i < 3; i++)
            {
                float d = clippingPlanes[nPlane].DistanceToPoint(inputTris[nTri].v[i]);

                if (d >= 0)
                {
                    insideIndeces.push_back(i);
                }
                else
                {
                    outsideIndeces.push_back(i);
                }
            }

            if (insideIndeces.size() == 3)
            {
                outputTris.push_back(inputTris[nTri]);
            }
            else if (insideIndeces.size() == 2)
            {
                // two new triangles
                CTri newTriA = inputTris[nTri], newTriB = inputTris[nTri];
                CVec4 intersectA, intersectB;

                CVec4 outsidePoint = inputTris[nTri].v[outsideIndeces[0]];
                CVec4 insidePointA = inputTris[nTri].v[insideIndeces[0]];
                CVec4 insidePointB = inputTris[nTri].v[insideIndeces[1]];

                clippingPlanes[nPlane].Intersect(insidePointA, outsidePoint, &intersectA);
                clippingPlanes[nPlane].Intersect(insidePointB, outsidePoint, &intersectB);

                newTriA.v[insideIndeces[0]] = insidePointA;
                newTriA.v[insideIndeces[1]] = intersectA;
                newTriA.v[outsideIndeces[0]] = intersectB;

                newTriB.v[insideIndeces[0]] = insidePointA;
                newTriB.v[insideIndeces[1]] = insidePointB;
                newTriB.v[outsideIndeces[0]] = intersectB;

                outputTris.push_back(newTriA);
                outputTris.push_back(newTriB);
            }
            else if (insideIndeces.size() == 1)
            {
                // new triangle
                CTri newTri = inputTris[nTri];
                CVec4 intersectA, intersectB;

                CVec4 insidePoint = inputTris[nTri].v[insideIndeces[0]];
                CVec4 outsidePointA = inputTris[nTri].v[outsideIndeces[0]];
                CVec4 outsidePointB = inputTris[nTri].v[outsideIndeces[1]];

                clippingPlanes[nPlane].Intersect(insidePoint, outsidePointA, &intersectA);
                clippingPlanes[nPlane].Intersect(insidePoint, outsidePointB, &intersectB);

                newTri.v[insideIndeces[0]] = insidePoint;
                newTri.v[outsideIndeces[0]] = intersectA;
                newTri.v[outsideIndeces[1]] = intersectB;

                outputTris.push_back(newTri);
            }
        }
    }

    for (size_t i = 0; i < outputTris.size(); i++)
    {
        trisOut.push_back(outputTris[i]);
    }
}

/********************/

CProjection::CProjection(float fnear, float ffar, float fov, float aspectRatio) :
    m_Near(fnear),
    m_Far(ffar),
    m_Fov(fov),
    m_AspectRatio(aspectRatio)
{
    m_ClippingPlanes.push_back({ 0, 0, 0.5f,     0, 0, 1.0f }); // near plane
    m_ClippingPlanes.push_back({ 0, 0, 1000.0f,  0, 0, -1.0f }); // far plane
}

void CProjection::GetMtx(CMtx *out)
{
    float fovRadians = 1.0f / tanf(((m_Fov * M_PI) / 180.0f) / 2.0f);
    out->m[0][0] = m_AspectRatio * fovRadians;
    out->m[1][1] = fovRadians;
    out->m[2][2] = m_Far / (m_Far - m_Near);
    out->m[3][2] = (-m_Far * m_Near) / (m_Far - m_Near);
    out->m[2][3] = 1.0f;
    out->m[3][3] = 0;
}

std::vector<CPlane>& CProjection::GetClippingPlanes(void)
{
    return m_ClippingPlanes;
}

/********************/

CCamera::CCamera(void) :
    m_Pos(0.0f, 0.0f, 0.0f),
    m_Rot(0.0f, 0.0f, 0.0f)
{
}

void CCamera::RotateY(float y)
{
    m_Rot.y = fmod(m_Rot.y + y, 360.0f);
}

void CCamera::RotateX(float x)
{
    m_Rot.x += x;

    if (m_Rot.x > 90.0f) m_Rot.x = 90.0f;
    if (m_Rot.x < -90.0f) m_Rot.x = -90.0f;
}

void CCamera::TranslateX(float x)
{
    CVec4 right;
    GetDirections(NULL, NULL, &right);
    right.Multiply(x, &right);
    m_Pos.Add(&right, &m_Pos);
}

void CCamera::TranslateY(float y)
{
    CVec4 up;
    GetDirections(NULL, &up, NULL);
    up.Multiply(y, &up);
    m_Pos.Add(&up, &m_Pos);
}

void CCamera::TranslateZ(float z)
{
    CVec4 forward;
    GetDirections(&forward, NULL, NULL);
    forward.Multiply(z, &forward);
    m_Pos.Add(&forward, &m_Pos);
}

void CCamera::SetPos(float x, float y, float z)
{
    m_Pos = {x, y, z};
}

void CCamera::SetRot(float x, float y, float z)
{
    m_Rot = { x, y, z };
}

void CCamera::GetDirections(CVec4 *_forward, CVec4 *_up, CVec4 *_right)
{
    CVec4 forward(0, 0, 1);
    CVec4 up(0, 1, 0);
    CVec4 right;

    forward.RotateX(m_Rot.x, &forward);
    forward.RotateY(m_Rot.y, &forward);
    forward.RotateZ(m_Rot.z, &forward);

    if (_forward != NULL)
    {
        forward.Normalize(_forward);
    }

    CVec4 a;
    forward.Multiply(up.DotProduct(&forward), &a);
    up.Subtract(&a, &up);

    if (_up != NULL)
    {
        up.Normalize(_up);
    }

    up.CrossProduct(&forward, &right);

    if (_right != NULL)
    {
        right.Normalize(_right);
    }
}

void CCamera::GetViewMatrix(CMtx *out)
{
    CVec4 forward, up, right;
    GetDirections(&forward, &up, &right);

    out->m[0][0] = right.x;
    out->m[1][0] = right.y;
    out->m[2][0] = right.z;
    out->m[3][0] = -m_Pos.DotProduct(&right);

    out->m[0][1] = up.x;
    out->m[1][1] = up.y;
    out->m[2][1] = up.z;
    out->m[3][1] = -m_Pos.DotProduct(&up);

    out->m[0][2] = forward.x;
    out->m[1][2] = forward.y;
    out->m[2][2] = forward.z;
    out->m[3][2] = -m_Pos.DotProduct(&forward);

    out->m[0][3] = 0.0f;
    out->m[1][3] = 0.0f;
    out->m[2][3] = 0.0f;
    out->m[3][3] = 1.0f;
}

/********************/

CBasicMeshGeometry::CBasicMeshGeometry(void):
    m_SelectedTriIdx(-1)
{
}

void CBasicMeshGeometry::AddVertex(CVec4 vertex)
{
    m_Vertices.push_back(vertex);
}

// if vertex is already in the list, return its index
// else push vertex to list and return its index
size_t CBasicMeshGeometry::AddVertexUnique(CVec4 vertex)
{
    for (size_t i = 0; i < m_Vertices.size(); i++)
    {
        if (memcmp(&m_Vertices[i], &vertex, sizeof(CVec4) == 0))
        {
            return i;
        }
    }

    size_t index = m_Vertices.size();
    AddVertex(vertex);
    return index;
}

void CBasicMeshGeometry::AddTriangleRef(geom_tri_ref_t triref)
{
    m_TriangleRefs.push_back(triref);
}

void CBasicMeshGeometry::AddTriangleRef(size_t vidx0, size_t vidx1, size_t vidx2, size_t nCommand)
{
    AddTriangleRef({ vidx0, vidx1, vidx2, false, 0, nCommand });
}

void CBasicMeshGeometry::AddTriangleRefs(geom_tri_ref_t trirefs[], size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        AddTriangleRef(trirefs[i]);
    }
}

void CBasicMeshGeometry::AddVertices(CVec4 vertices[], size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        AddVertex(vertices[i]);
    }
}

bool CBasicMeshGeometry::GetTriangle(CTri *out, size_t index)
{
    size_t numTris = GetNumTriangles();
    size_t numVerts = m_Vertices.size();
    
    if (index >= numTris)
    {
        return false;
    }

    size_t v0 = m_TriangleRefs[index].vidx0;
    size_t v1 = m_TriangleRefs[index].vidx1;
    size_t v2 = m_TriangleRefs[index].vidx2;

    if (v0 >= numVerts || v1 >= numVerts || v2 >= numVerts)
    {
        return false;
    }

    out->v[0] = m_Vertices[v0];
    out->v[1] = m_Vertices[v1];
    out->v[2] = m_Vertices[v2];
    out->m_ClickIndex = m_TriangleRefs[index].clickIndex;
    return true;
}

size_t CBasicMeshGeometry::GetNumTriangles(void)
{
    return m_TriangleRefs.size();
}

void CBasicMeshGeometry::Clear(void)
{
    m_TriangleRefs.clear();
    m_Vertices.clear();
    m_SelectedTriIdx = -1;
}

/********************/

bool CScene::AddGeometry(uint32_t dlAddr)
{
    std::pair<uint32_t, CBasicMeshGeometry> ret;
    CBasicMeshGeometry geom;
    return m_Geometries.insert(std::pair<uint32_t, CBasicMeshGeometry>(dlAddr, geom)).second;
}

/********************/

CDrawBuffers::CDrawBuffers(int width, int height):
    m_Width(width),
    m_Height(height)
{
    m_ClippingPlanes.push_back({ 0, 0, 0, 0, 1, 0 }); // top
    m_ClippingPlanes.push_back({ 0, (float)height - 1, 0, 0, -1, 0 }); // bottom
    m_ClippingPlanes.push_back({ 0, 0, 0, 1, 0, 0 }); // left
    m_ClippingPlanes.push_back({ (float)width - 1, 0, 0, -1, 0, 0 }); // right

    m_ColorBuffer = new uint32_t[width * height];
    m_SelectBuffer = new int[width * height];
    m_DepthBuffer = new float[width * height];
    Clear();
}

CDrawBuffers::~CDrawBuffers(void)
{
    delete[] m_ColorBuffer;
    delete[] m_SelectBuffer;
    delete[] m_DepthBuffer;
}

bool CDrawBuffers::Inside(int x, int y)
{
    return (x > 0 && x < m_Width && y > 0 && y < m_Height);
}

int CDrawBuffers::Index(int x, int y)
{
    return (y * m_Width + x);
}

void CDrawBuffers::SetColor(int x, int y, uint32_t color)
{
    if (!Inside(x, y)) return;
    m_ColorBuffer[Index(x, y)] = color;
}

void CDrawBuffers::SetSelect(int x, int y, int key)
{
    if (!Inside(x, y)) return;
    m_SelectBuffer[Index(x, y)] = key;
}

int CDrawBuffers::GetSelect(int x, int y)
{
    if (!Inside(x, y)) return -1;
    return m_SelectBuffer[Index(x, y)];
}

float CDrawBuffers::GetDepth(int x, int y)
{
    if (!Inside(x, y)) return -1;
    return m_DepthBuffer[Index(x, y)];
}

void CDrawBuffers::SetDepth(int x, int y, float w)
{
    if (!Inside(x, y)) return;
    m_DepthBuffer[Index(x, y)] = w;
}

void CDrawBuffers::Clear(void)
{
    size_t bufferSize = m_Width * m_Height * sizeof(uint32_t);
    for (size_t i = 0; i < m_Width * m_Height; i++)
    {
        m_ColorBuffer[i] = 0x44444444;
    }

    for (size_t i = 0; i < m_Width * m_Height; i++)
    {
        m_DepthBuffer[i] = INFINITY;
    }

    memset(m_SelectBuffer, 0, bufferSize);
}

uint32_t CDrawBuffers::Color(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 16) | (g << 8) | b;
}

void CDrawBuffers::FillRect(int x, int y, int w, int h, uint32_t color)
{
    for (int cy = y; cy < y + h; cy++)
    {
        for (int cx = x; cx < x + w; cx++)
        {
            SetColor(cx, cy, color);
        }
    }
}

void CDrawBuffers::DrawLine(CVec4& p0, CVec4& p1, uint32_t color)
{
    int deltaX = abs((int)p1.x - (int)p0.x);
    int deltaY = -abs((int)p1.y - (int)p0.y);
    int signX = (int)p0.x < (int)p1.x ? 1 : -1;
    int signY = (int)p0.y < (int)p1.y ? 1 : -1;
    int error = deltaX + deltaY;

    int x = (int)p0.x;
    int y = (int)p0.y;

    while (!(x == (int)p1.x && y == (int)p1.y))
    {
        int e2 = 2 * error;

        if (e2 >= deltaY)
        {
            error += deltaY;
            x += signX;
        }

        if (e2 <= deltaX)
        {
            error += deltaX;
            y += signY;
        }

        SetColor(x, y, color);
    }
}

void CDrawBuffers::DrawTriangle(CTri &tri, uint32_t clickIndex)
{
    std::vector<CTri> clippedTris;
    tri.Clip(m_ClippingPlanes, clippedTris);
    
    for (int i = 0; i < clippedTris.size(); i++)
    {
        CTri clippedTri;
        clippedTris[i].YSortPoints(&clippedTri);
        CVec4 *v = clippedTri.v;

        v[0].x = (int)(v[0].x);
        v[0].y = (int)(v[0].y);
        v[1].x = (int)(v[1].x);
        v[1].y = (int)(v[1].y);
        v[2].x = (int)(v[2].x);
        v[2].y = (int)(v[2].y);

        double invslope1 = (v[1].x - v[0].x) / (v[1].y - v[0].y);
        double invslope2 = (v[2].x - v[0].x) / (v[2].y - v[0].y);
        double invslope3 = (v[2].x - v[1].x) / (v[2].y - v[1].y);

        // top tri, flat bottom
        if (v[0].y != v[1].y)
        {
            float x1 = v[0].x;
            float x2 = v[0].x;

            for (int y = v[0].y; y <= v[1].y; y++)
            {
                int cx1 = (x1 < x2) ? x1 : x2;
                int cx2 = (x1 > x2) ? x1 : x2;

                for (int x = cx1; x <= cx2; x++)
                {
                    CVec4 weights;
                    tri.Weigh2d(x, y, &weights);
                    float w = (tri.v[0].w * weights.x + tri.v[1].w * weights.y + tri.v[2].w * weights.z);

                    if (w < GetDepth(x, y))
                    {
                        SetDepth(x, y, w);
                        SetColor(x, y, tri.m_Color);
                        SetSelect(x, y, clickIndex);
                    }
                }

                x1 += invslope1;
                x2 += invslope2;
            }
        }

        // bottom tri, flat top
        if (v[1].y != v[2].y)
        {
            float x1 = v[2].x;
            float x2 = v[2].x;

            for (int y = v[2].y; y >= v[1].y; y--)
            {
                int cx1 = (x1 < x2) ? x1 : x2;
                int cx2 = (x1 > x2) ? x1 : x2;

                for (int x = cx1; x <= cx2; x++)
                {
                    CVec4 weights;
                    tri.Weigh2d(x, y, &weights);
                    float w = (tri.v[0].w * weights.x + tri.v[1].w * weights.y + tri.v[2].w * weights.z);

                    if (w < GetDepth(x, y))
                    {
                        SetDepth(x, y, w);
                        SetColor(x, y, tri.m_Color);
                        SetSelect(x, y, clickIndex);
                    }
                }

                x1 -= invslope2;
                x2 -= invslope3;
            }
        }
    }

    //DrawLine(tri.v[0], tri.v[1], 0x44444444);
    //DrawLine(tri.v[1], tri.v[2], 0x44444444);
    //DrawLine(tri.v[2], tri.v[0], 0x44444444);

    //FillRect(tri.v[0].x - 1, tri.v[0].y - 1, 2, 2, 0x22222222);
    //FillRect(tri.v[1].x - 1, tri.v[1].y - 1, 2, 2, 0x22222222);
    //FillRect(tri.v[2].x - 1, tri.v[2].y - 1, 2, 2, 0x22222222);
}

void CDrawBuffers::Render(CBasicMeshGeometry *geom, CCamera *camera)
{
    Clear();

    CMtx projectionMtx, viewMtx;
    CProjection projection(0.5f, 1000.0f, 90.0f, (float)m_Height / (float)m_Width);
    CVec4 camForward, camUp, camRight, lightDirection;
    CVec4 scaleFlipYZ(1.0f, -1.0f, -1.0f);
    CVec4 scaleScreenSpace(m_Width, m_Height, 1.0f);

    std::vector<CTri> projectedTris, clippedTris;

    projection.GetMtx(&projectionMtx);
    camera->GetViewMatrix(&viewMtx);
    camera->GetDirections(&camForward, &camUp, &camRight);

    lightDirection = camForward;
    
    size_t numTris = geom->GetNumTriangles();

    for (size_t i = 0; i < numTris; i++)
    {
        CTri tri;
        geom->GetTriangle(&tri, i);

        tri.m_ClickIndex = i;
        tri.Multiply(&scaleFlipYZ, &tri);

        CVec4 triNormal;
        tri.CalculateNormal(&triNormal);

        CVec4 temp;
        tri.v[0].Subtract(&camera->m_Pos, &temp);
        float d = triNormal.DotProduct(&temp);
        bool bBackface = d > 0.0f;

        float dpLight = -triNormal.DotProduct(&lightDirection);
        uint8_t intensity = (dpLight + 1.0f) * (200.0f / 2); // map -1:1 to 0:200

        if (bBackface)
        {
            intensity = 200 - intensity;
            //continue;
        }

        if (tri.m_ClickIndex == geom->m_SelectedTriIdx)
        {
            // highlight selected triangle in yellow
            tri.m_Color = Color(intensity, intensity, 0);
        }
        else
        {
            if (d > 0.0f) // backface
            {
                tri.m_Color = Color(intensity/2, intensity, intensity);
            }
            else
            {
                tri.m_Color = Color(intensity, intensity, intensity);
            }
            
        }

        tri.Multiply(&viewMtx, &tri);
        tri.Clip(projection.GetClippingPlanes(), clippedTris);
    }

    for (size_t i = 0; i < clippedTris.size(); i++)
    {
        CTri tri = clippedTris[i];
        tri.Multiply(&projectionMtx, &tri);
        tri.PerspectiveDivide(&tri);
        projectedTris.push_back(tri);
    }

    // generate bitmap
    for (size_t i = 0; i < projectedTris.size(); i++)
    {
        CTri tri = projectedTris[i];
        tri.Multiply(&scaleScreenSpace, &tri);
        tri.Translate(m_Width / 2, m_Height / 2, 0, &tri);
        DrawTriangle(tri, tri.m_ClickIndex);
    }
}
