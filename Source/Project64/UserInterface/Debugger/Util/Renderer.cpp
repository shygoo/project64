#include <stdafx.h>
#include "Renderer.h"

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
	return true;
}

size_t CBasicMeshGeometry::GetNumTriangles(void)
{
	return m_TriangleRefs.size();
}

/********************/

bool CScene::AddGeometry(uint32_t dlAddr)
{
    std::pair<uint32_t, CBasicMeshGeometry> ret;
    CBasicMeshGeometry geom;
    return m_Geometries.insert(std::pair<uint32_t, CBasicMeshGeometry>(dlAddr, geom)).second;
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

void Test_3d(HWND hwnd, CBasicMeshGeometry *geom)
{
    CRect rc;
    GetWindowRect(hwnd, &rc);

    float width = rc.Width();
    float height = rc.Height();

    HDC hdc = GetDC(hwnd);
    HBITMAP hbm = CreateCompatibleBitmap(hdc, width, height);
    HDC hdcMem = CreateCompatibleDC(hdc);
    SelectObject(hdcMem, hbm);

	CProjection projection(0.1f, 1000.0f, 90.0f, height / width);
	CMtx projMtx;
	projection.GetMtx(&projMtx);

	float zDistFromCam = 3.0f;

    HRGN hrgn = CreateRectRgn(1, 1, width-1, height-1);
    CRect fillRc(1, 1, width-1, height-1);
    HBRUSH hbrBlack = CreateSolidBrush(RGB(0x22, 0x22, 0x22));

    SelectClipRgn(hdcMem, hrgn);
	FillRect(hdcMem, &fillRc, hbrBlack);

	static float yrot = 0.0f;
	yrot += 1.0f;

    CVec3 cameraPos(0, 0, 0);

    std::vector<CTri> projectedTris;

	size_t numTris = geom->GetNumTriangles();
	for (size_t i = 0; i < numTris; i++)
	{
		CTri tri;
		geom->GetTriangle(&tri, i);

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
        tri.m_Color = RGB(intensity, intensity, intensity);

		tri.Mult(&tri, &projMtx);

        projectedTris.push_back(tri);
	}

    // zsort
    std::sort(projectedTris.begin(), projectedTris.end(), CompareTriangleDepth);

    for (size_t i = 0; i < projectedTris.size(); i++)
    {
        CTri tri = projectedTris[i];

        HPEN hPenOutline = CreatePen(PS_SOLID, 1, Highlight(tri.m_Color, -10));
        HBRUSH hBrushFill = CreateSolidBrush(tri.m_Color);
        SelectObject(hdcMem, hPenOutline);
        SelectObject(hdcMem, hBrushFill);

        // map 0:1 to screen center:edge
        float v0_x = tri.m_v[0].m_x * width + (width / 2);
        float v0_y = tri.m_v[0].m_y * height + (height / 2);
        float v1_x = tri.m_v[1].m_x * width + (width / 2);
        float v1_y = tri.m_v[1].m_y * height + (height / 2);
        float v2_x = tri.m_v[2].m_x * width + (width / 2);
        float v2_y = tri.m_v[2].m_y * height + (height / 2);

        BeginPath(hdcMem);
        MoveToEx(hdcMem, v0_x, v0_y, NULL);
        LineTo(hdcMem, v1_x, v1_y);
        LineTo(hdcMem, v2_x, v2_y);
        LineTo(hdcMem, v0_x, v0_y);
        EndPath(hdcMem);
        StrokeAndFillPath(hdcMem);

        SetPixel(hdcMem, v0_x, v0_y, RGB(100, 100, 100));
        SetPixel(hdcMem, v1_x, v1_y, RGB(100, 100, 100));
        SetPixel(hdcMem, v2_x, v2_y, RGB(100, 100, 100));

        DeleteObject(hPenOutline);
        DeleteObject(hBrushFill);
    }

    BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);
    
    DeleteObject(hrgn);
	DeleteObject(hbrBlack);
    ReleaseDC(hwnd, hdc);
    DeleteDC(hdcMem);
    DeleteObject(hbm);
}
