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

/********************/

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

void Test_3d(HDC hdc)
{
	CVec3 vertices[] = {
		{-0.5f, -0.5f, -0.5f }, // 0 lower left
		{-0.5f,  0.5f, -0.5f }, // 1 upper left
		{ 0.5f,  0.5f, -0.5f }, // 2 upper right
		{ 0.5f, -0.5f, -0.5f }, // 3 lower right
		{-0.5f, -0.5f,  0.5f }, // 4 back lower left
		{-0.5f,  0.5f,  0.5f }, // 5 back upper left
		{ 0.5f,  0.5f,  0.5f }, // 6 back upper right
		{ 0.5f, -0.5f,  0.5f }  // 7 back lower right
	};

	geom_tri_ref_t triangles[] = {
		{ 0, 1, 2 }, // front
		{ 0, 2, 3 },
		{ 1, 5, 6 }, // top
		{ 1, 6, 2 },
		{ 2, 7, 3 }, // right
		{ 2, 6, 7 },
		{ 0, 5, 1 }, // left
		{ 0, 4, 5 },
		{ 0, 3, 7 }, // bottom
		{ 0, 7, 4 },
	};

	CBasicMeshGeometry geom;
	geom.AddVertices(vertices, sizeof(vertices) / sizeof(vertices[0]));
	geom.AddTriangleRefs(triangles, sizeof(triangles)/sizeof(triangles[0]));

	CProjection projection(0.1f, 1000.0f, 90.0f, 256.0f / 256.0f);
	CMtx projMtx;
	projection.GetMtx(&projMtx);

	float zDistFromCam = 4.0f;

	HBRUSH hbrBlack = CreateSolidBrush(RGB(0x22, 0x22, 0x22));
	HBRUSH hbrWhite = CreateSolidBrush(RGB(0xCC, 0xCC, 0xCC));

	CRect rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = 256;
	rc.bottom = 256;
	FillRect(hdc, &rc, hbrBlack);

	HPEN hpen = CreatePen(PS_SOLID, 1, RGB(0xCC, 0xCC, 0xCC));

	SelectObject(hdc, hpen);

	static float yrot = 0.0f;
	yrot += 1.0f;

	size_t numTris = geom.GetNumTriangles();
	for (size_t i = 0; i < numTris; i++)
	{
		CTri tri;
		geom.GetTriangle(&tri, i);

		tri.RotateX(&tri, yrot);
		tri.RotateY(&tri, yrot);
		tri.Translate(&tri, 0, 0, zDistFromCam);
		tri.Mult(&tri, &projMtx);

		BeginPath(hdc);
		SelectObject(hdc, hbrWhite);
		MoveToEx(hdc, tri.m_v[0].m_x * 256.0f + (256.0f / 2), tri.m_v[0].m_y * 256.0f + (256.0f / 2), NULL);
		LineTo(hdc, tri.m_v[1].m_x * 256.0f + (256.0f / 2), tri.m_v[1].m_y * 256.0f + (256.0f / 2));
		LineTo(hdc, tri.m_v[2].m_x * 256.0f + (256.0f / 2), tri.m_v[2].m_y * 256.0f + (256.0f / 2));
		LineTo(hdc, tri.m_v[0].m_x * 256.0f + (256.0f / 2), tri.m_v[0].m_y * 256.0f + (256.0f / 2));
		EndPath(hdc);
		StrokePath(hdc);
	}

	DeleteObject(hbrBlack);
	DeleteObject(hbrWhite);
}