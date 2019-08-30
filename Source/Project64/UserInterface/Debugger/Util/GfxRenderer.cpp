#include <stdafx.h>
#include "GfxRenderer.h"

CMtx::CMtx(void)
{
	memset(m_m, 0, sizeof(m_m));
}

/********************/

CVec3::CVec3(void) :
	m_x(0.0f),
	m_y(0.0f),
	m_z(0.0f)
{
}

CVec3::CVec3(float x, float y, float z) :
	m_x(x),
	m_y(y),
	m_z(z)
{
}

void CVec3::Mult(CVec3 *out, CMtx* mtx)
{
	
	float x = (m_x * mtx->m_m[0][0]) + (m_y * mtx->m_m[1][0]) + (m_z * mtx->m_m[2][0]) + mtx->m_m[3][0];
	float y = (m_x * mtx->m_m[0][1]) + (m_y * mtx->m_m[1][1]) + (m_z * mtx->m_m[2][1]) + mtx->m_m[3][1];
	float z = (m_x * mtx->m_m[0][2]) + (m_y * mtx->m_m[1][2]) + (m_z * mtx->m_m[2][2]) + mtx->m_m[3][2];
	float w = (m_x * mtx->m_m[0][3]) + (m_y * mtx->m_m[1][3]) + (m_z * mtx->m_m[2][3]) + mtx->m_m[3][3];

	out->m_x = x;
	out->m_y = y;
	out->m_z = z;

	if (w != 0.0f)
	{
		out->m_x /= w;
		out->m_y /= w;
		out->m_z /= w;
	}
}

void CVec3::Translate(CVec3 *out, float x, float y, float z)
{
	out->m_x = m_x + x;
	out->m_y = m_y + y;
	out->m_z = m_z + z;
}

void CVec3::Scale(CVec3 *out, float x, float y, float z)
{
    CMtx mtx;
    mtx.m_m[0][0] = x;
    mtx.m_m[1][1] = y;
    mtx.m_m[2][2] = z;
    mtx.m_m[3][3] = 1.0f;
    Mult(out, &mtx);
}

void CVec3::RotateX(CVec3 *out, float degrees)
{
	float rads = degrees * (M_PI / 180.0f);

	CMtx mtx;
	mtx.m_m[0][0] = 1.0f;
	mtx.m_m[1][1] = cosf(rads);
	mtx.m_m[1][2] = sinf(rads);
	mtx.m_m[2][1] = -sinf(rads);
	mtx.m_m[2][2] = cosf(rads);
	mtx.m_m[3][3] = 1.0f;

	Mult(out, &mtx);
}

void CVec3::RotateY(CVec3 *out, float degrees)
{
	float rads = degrees * (M_PI / 180.0f);

	CMtx mtx;
	memset(&mtx, 0, sizeof(mtx));

	mtx.m_m[0][0] = cosf(rads);
	mtx.m_m[0][2] = sinf(rads);
	mtx.m_m[1][1] = 1.0f;
	mtx.m_m[2][0] = -sinf(rads);
	mtx.m_m[2][2] = cosf(rads);
	mtx.m_m[3][3] = 1.0f;

	Mult(out, &mtx);
}

void CVec3::RotateZ(CVec3 *out, float degrees)
{
	float rads = degrees * (M_PI / 180.0f);

	CMtx mtx;
	mtx.m_m[0][0] = cosf(rads);
	mtx.m_m[0][1] = sinf(rads);
	mtx.m_m[1][0] = -sinf(rads);
	mtx.m_m[1][1] = cosf(rads);
	mtx.m_m[2][2] = 1.0f;
	mtx.m_m[3][3] = 1.0f;

	Mult(out, &mtx);
}

float CVec3::DotProduct(CVec3 *otherVec)
{
    return (otherVec->m_x * m_x) +
           (otherVec->m_y * m_y) +
           (otherVec->m_z * m_z);
}

void CVec3::Subtract(CVec3 *in, CVec3 *out)
{
    out->m_x = m_x - in->m_x;
    out->m_y = m_y - in->m_y;
    out->m_z = m_z - in->m_z;
}

void CVec3::Normalize(CVec3 *out)
{
    float length = sqrtf((m_x * m_x) + (m_y * m_y) + (m_z * m_z));
    out->m_x = m_x / length;
    out->m_y = m_y / length;
    out->m_z = m_z / length;
}

/********************/

CTri::CTri(void):
    m_Color(RGB(255, 255, 255))
{
}

void CTri::Mult(CTri *out, CMtx *mtx)
{
	m_v[0].Mult(&out->m_v[0], mtx);
	m_v[1].Mult(&out->m_v[1], mtx);
	m_v[2].Mult(&out->m_v[2], mtx);
}

void CTri::Translate(CTri *out, float x, float y, float z)
{
	m_v[0].Translate(&out->m_v[0], x, y, z);
	m_v[1].Translate(&out->m_v[1], x, y, z);
	m_v[2].Translate(&out->m_v[2], x, y, z);
}

void CTri::Scale(CTri *out, float x, float y, float z)
{
    m_v[0].Scale(&out->m_v[0], x, y, z);
    m_v[1].Scale(&out->m_v[1], x, y, z);
    m_v[2].Scale(&out->m_v[2], x, y, z);
}

void CTri::RotateX(CTri *out, float degrees)
{
	m_v[0].RotateX(&out->m_v[0], degrees);
	m_v[1].RotateX(&out->m_v[1], degrees);
	m_v[2].RotateX(&out->m_v[2], degrees);
}

void CTri::RotateY(CTri *out, float degrees)
{
	m_v[0].RotateY(&out->m_v[0], degrees);
	m_v[1].RotateY(&out->m_v[1], degrees);
	m_v[2].RotateY(&out->m_v[2], degrees);
}

void CTri::RotateZ(CTri *out, float degrees)
{
	m_v[0].RotateZ(&out->m_v[0], degrees);
	m_v[1].RotateZ(&out->m_v[1], degrees);
	m_v[2].RotateZ(&out->m_v[2], degrees);
}

void CTri::YSort(CTri *out)
{
    *out = *this;
    CVec3 t;
    CVec3 *v = out->m_v;

    if (v[0].m_y > v[2].m_y)
    {
        t = v[0]; v[0] = v[2]; v[2] = t;
    }

    if (v[0].m_y > v[1].m_y)
    {
        t = v[0]; v[0] = v[1]; v[1] = t;
    }

    if (v[1].m_y > v[2].m_y)
    {
        t = v[1]; v[1] = v[2]; v[2] = t;
    }
}

void CTri::CalculateNormal(CVec3 *out)
{
    CVec3 lineA, lineB;

    lineA = { m_v[1].m_x - m_v[0].m_x, m_v[1].m_y - m_v[0].m_y, m_v[1].m_z - m_v[0].m_z };
    lineB = { m_v[2].m_x - m_v[0].m_x, m_v[2].m_y - m_v[0].m_y, m_v[2].m_z - m_v[0].m_z };

    out->m_x = (lineA.m_y * lineB.m_z) - (lineA.m_z * lineB.m_y);
    out->m_y = (lineA.m_z * lineB.m_x) - (lineA.m_x * lineB.m_z);
    out->m_z = (lineA.m_x * lineB.m_y) - (lineA.m_y * lineB.m_x);

    out->Normalize(out);
}

void CTri::Center(CVec3 *out)
{
    float x = (m_v[0].m_x + m_v[1].m_x + m_v[2].m_x) / 3.0f;
    float y = (m_v[0].m_y + m_v[1].m_y + m_v[2].m_y) / 3.0f;
    float z = (m_v[0].m_z + m_v[1].m_z + m_v[2].m_z) / 3.0f;
    out->m_x = x;
    out->m_y = y;
    out->m_z = z;
}

/********************/

CProjection::CProjection(float fnear, float ffar, float fov, float aspectRatio) :
	m_Near(fnear),
	m_Far(ffar),
	m_Fov(fov),
	m_AspectRatio(aspectRatio)
{
}

void CProjection::GetMtx(CMtx *out)
{
	float fovRadians = 1.0f / tanf(((m_Fov * M_PI) / 180.0f) / 2.0f);
	out->m_m[0][0] = m_AspectRatio * fovRadians;
	out->m_m[1][1] = fovRadians;
	out->m_m[2][2] = m_Far / (m_Far - m_Near);
	out->m_m[3][2] = (-m_Far * m_Near) / (m_Far - m_Near);
	out->m_m[2][3] = 1.0f;
	out->m_m[3][3] = 0;
}

/********************/

void CBasicMeshGeometry::AddVertex(CVec3 vertex)
{
	m_Vertices.push_back(vertex);
}

// if vertex is already in the list, return its index
// else push vertex to list and return its index
size_t CBasicMeshGeometry::AddVertexUnique(CVec3 vertex)
{
    for (size_t i = 0; i < m_Vertices.size(); i++)
    {
        if (memcmp(&m_Vertices[i], &vertex, sizeof(CVec3) == 0))
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

void CBasicMeshGeometry::AddTriangleRef(size_t vidx0, size_t vidx1, size_t vidx2)
{
	AddTriangleRef({ vidx0, vidx1, vidx2 });
}

void CBasicMeshGeometry::AddTriangleRefs(geom_tri_ref_t trirefs[], size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		AddTriangleRef(trirefs[i]);
	}
}

void CBasicMeshGeometry::AddVertices(CVec3 vertices[], size_t count)
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

	out->m_v[0] = m_Vertices[v0];
	out->m_v[1] = m_Vertices[v1];
	out->m_v[2] = m_Vertices[v2];
    out->index = m_TriangleRefs[index].index;
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
    m_ColorBuffer = new uint32_t[width * height];
    m_SelectBuffer = new int[width * height];
    Clear();
}

CDrawBuffers::~CDrawBuffers(void)
{
    delete[] m_ColorBuffer;
}

void CDrawBuffers::SetPixel(int x, int y, uint32_t color)
{
    if (x >= m_Width || x < 0) return;
    if (y >= m_Height || y < 0) return;

    m_ColorBuffer[y * m_Width + x] = color;
}

void CDrawBuffers::SetSelect(int x, int y, int key)
{
    if (x >= m_Width || x < 0) return;
    if (y >= m_Height || y < 0) return;

    m_SelectBuffer[y * m_Width + x] = key;
}

int CDrawBuffers::GetSelect(int x, int y)
{
    if (x >= m_Width || x < 0) return -1;
    if (y >= m_Height || y < 0) return -1;

    return m_SelectBuffer[y * m_Width + x];
}

void CDrawBuffers::Clear(void)
{
    size_t bufferSize = m_Width * m_Height * sizeof(uint32_t);
    memset(m_ColorBuffer, 0, bufferSize);
    memset(m_SelectBuffer, 0, bufferSize);
}

/********************/

bool CompareTriangleDepth(CTri tri1, CTri tri2)
{
    CVec3 c1, c2;
    tri1.Center(&c1);
    tri2.Center(&c2);
    return c1.m_z > c2.m_z;
}

COLORREF Highlight(COLORREF in, int adjust)
{
    int r = (in & 0xFF) + adjust;
    int g = ((in >> 8) & 0xFF) + adjust;
    int b = ((in >> 16) & 0xFF) + adjust;
    r = max(0, r); r = min(255, r);
    g = max(0, g); g = min(255, g);
    b = max(0, b); b = min(255, b);
    return RGB(r, g, b);
}

void RasterTriangle(CTri *_tri, CDrawBuffers *db, uint32_t index)
{
    CTri tri;
    _tri->YSort(&tri);

    CVec3 *tv = tri.m_v;

    double invslope1 = (tv[1].m_x - tv[0].m_x) / (tv[1].m_y - tv[0].m_y);
    double invslope2 = (tv[2].m_x - tv[0].m_x) / (tv[2].m_y - tv[0].m_y);
    double invslope3 = (tv[2].m_x - tv[1].m_x) / (tv[2].m_y - tv[1].m_y);

    // top tri, flat bottom
    if ((int)tv[0].m_y != (int)tv[1].m_y)
    {
        double x1 = tv[0].m_x;
        double x2 = tv[0].m_x;

        for (int y = roundf(tv[0].m_y); y < roundf(tv[1].m_y); y++)
        {
            int cx1 = roundf((x1 < x2) ? x1 : x2);
            int cx2 = roundf((x1 > x2) ? x1 : x2);

            for (int x = cx1; x < cx2; x++)
            {
                db->SetPixel(x, y, tri.m_Color);
                db->SetSelect(x, y, index);
            }

            x1 += invslope1;
            x2 += invslope2;
        }
    }

    // bottom tri, flat top
    if ((int)tv[1].m_y != (int)tv[2].m_y)
    {
        double x1 = tv[2].m_x;
        double x2 = tv[2].m_x;

        for (int y = roundf(tv[2].m_y); y >= roundf(tv[1].m_y); y--)
        {
            int cx1 = roundf((x1 < x2) ? x1 : x2);
            int cx2 = roundf((x1 > x2) ? x1 : x2);

            for (int x = cx1; x < cx2; x++)
            {
                db->SetPixel(x, y, tri.m_Color);
                db->SetSelect(x, y, index);
            }

            x1 -= invslope2;
            x2 -= invslope3;
        }
    }
}

void Test_3d(HWND hwnd, CBasicMeshGeometry *geom, CDrawBuffers *db)
{
    db->Clear();
    float width = db->m_Width;
    float height = db->m_Height;

	CProjection projection(0.1f, 1000.0f, 90.0f, height / width);
	CMtx projMtx;
	projection.GetMtx(&projMtx);

	float zDistFromCam = 3.0f;

	static float yrot = 0.0f; // test
	yrot += 0.1f;

    CVec3 cameraPos(0, 0, 0);

    std::vector<CTri> projectedTris;

	size_t numTris = geom->GetNumTriangles();
	for (size_t i = 0; i < numTris; i++)
	{
		CTri tri;
		geom->GetTriangle(&tri, i);
        tri.index = i;

        tri.RotateY(&tri, yrot); // rotate at origin
        tri.Scale(&tri, 1.0f, -1.0f, -1.0f); // flip y and z
		tri.Translate(&tri, 0, 0, zDistFromCam); // push away from origin

        CVec3 normal;
        tri.CalculateNormal(&normal);

        CVec3 temp;
        tri.m_v[0].Subtract(&cameraPos, &temp);
        float d = normal.DotProduct(&temp);

        if (d > 0.0f) // cull backface
        {
            continue;
        }

        CVec3 lightDirection = { 0.0f, 0.0f, -1.0f };
        float dpLight = normal.DotProduct(&lightDirection);
        float intensity = (dpLight + 1.0f) * (200.0f / 2); // map -1:1 to 0:200

        if (geom->m_TriangleRefs[tri.index].bSelected)
        {
            tri.m_Color = RGB(0xFF, 0xFF, 0);
        }
        else
        {
            tri.m_Color = RGB(intensity, intensity, intensity);
        }

		tri.Mult(&tri, &projMtx);

        projectedTris.push_back(tri);
	}

    // zsort
    std::sort(projectedTris.begin(), projectedTris.end(), CompareTriangleDepth);

    //if(0) // debug
    for (size_t i = 0; i < projectedTris.size(); i++)
    {
        CTri tri = projectedTris[i];
        CTri triSc = tri;

        // map 0:1 to screen center:edge
        triSc.m_v[0].m_x = tri.m_v[0].m_x * width + (width / 2);
        triSc.m_v[0].m_y = tri.m_v[0].m_y * height + (height / 2);
        triSc.m_v[1].m_x = tri.m_v[1].m_x * width + (width / 2);
        triSc.m_v[1].m_y = tri.m_v[1].m_y * height + (height / 2);
        triSc.m_v[2].m_x = tri.m_v[2].m_x * width + (width / 2);
        triSc.m_v[2].m_y = tri.m_v[2].m_y * height + (height / 2);

        if (triSc.m_v[0].m_x >= width || triSc.m_v[0].m_x <= 0) continue;
        if (triSc.m_v[1].m_x >= width || triSc.m_v[1].m_x <= 0) continue;
        if (triSc.m_v[2].m_x >= width || triSc.m_v[2].m_x <= 0) continue;
        if (triSc.m_v[0].m_y >= width || triSc.m_v[0].m_y <= 0) continue;
        if (triSc.m_v[1].m_y >= width || triSc.m_v[1].m_y <= 0) continue;
        if (triSc.m_v[2].m_y >= width || triSc.m_v[2].m_y <= 0) continue;

        RasterTriangle(&triSc, db, tri.index);
    }

    //db->m_ColorBuffer[4000] = 0xFFFFFFFF;

    HDC hdc = GetDC(hwnd);
    HBITMAP hbm = CreateBitmap(width, height, 1, 32, db->m_ColorBuffer);
    HDC hdcMem = CreateCompatibleDC(hdc);
    SelectObject(hdcMem, hbm);

    BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);
    
    ReleaseDC(hwnd, hdc);
    DeleteDC(hdcMem);
    DeleteObject(hbm);
}
