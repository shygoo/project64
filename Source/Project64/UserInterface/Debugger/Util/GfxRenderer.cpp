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

void CVec3::Multiply(CMtx* mtx, CVec3 *result)
{
	float x = (m_x * mtx->m_m[0][0]) + (m_y * mtx->m_m[1][0]) + (m_z * mtx->m_m[2][0]) + mtx->m_m[3][0];
	float y = (m_x * mtx->m_m[0][1]) + (m_y * mtx->m_m[1][1]) + (m_z * mtx->m_m[2][1]) + mtx->m_m[3][1];
	float z = (m_x * mtx->m_m[0][2]) + (m_y * mtx->m_m[1][2]) + (m_z * mtx->m_m[2][2]) + mtx->m_m[3][2];
	float w = (m_x * mtx->m_m[0][3]) + (m_y * mtx->m_m[1][3]) + (m_z * mtx->m_m[2][3]) + mtx->m_m[3][3];

	result->m_x = x;
	result->m_y = y;
	result->m_z = z;

	if (w != 0.0f)
	{
		result->m_x /= w;
		result->m_y /= w;
		result->m_z /= w;
	}
}

void CVec3::Multiply(float val, CVec3 *result)
{
	result->m_x = m_x * val;
	result->m_y = m_y * val;
	result->m_z = m_z * val;
}

void CVec3::Translate(float x, float y, float z, CVec3 *result)
{
	result->m_x = m_x + x;
	result->m_y = m_y + y;
	result->m_z = m_z + z;
}

void CVec3::Scale(float x, float y, float z, CVec3 *result)
{
    CMtx mtx;
    mtx.m_m[0][0] = x;
    mtx.m_m[1][1] = y;
    mtx.m_m[2][2] = z;
    mtx.m_m[3][3] = 1.0f;
	this->Multiply(&mtx, result);
}

void CVec3::RotateX(float degrees, CVec3 *result)
{
	float rads = degrees * (M_PI / 180.0f);

	CMtx mtx;
	mtx.m_m[0][0] = 1.0f;
	mtx.m_m[1][1] = cosf(rads);
	mtx.m_m[1][2] = sinf(rads);
	mtx.m_m[2][1] = -sinf(rads);
	mtx.m_m[2][2] = cosf(rads);
	mtx.m_m[3][3] = 1.0f;
	this->Multiply(&mtx, result);
}

void CVec3::RotateY(float degrees, CVec3 *result)
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
	this->Multiply(&mtx, result);
}

void CVec3::RotateZ(float degrees, CVec3 *result)
{
	float rads = degrees * (M_PI / 180.0f);

	CMtx mtx;
	mtx.m_m[0][0] = cosf(rads);
	mtx.m_m[0][1] = sinf(rads);
	mtx.m_m[1][0] = -sinf(rads);
	mtx.m_m[1][1] = cosf(rads);
	mtx.m_m[2][2] = 1.0f;
	mtx.m_m[3][3] = 1.0f;

	this->Multiply(&mtx, result);
}

float CVec3::DotProduct(CVec3 *otherVec)
{
    return (otherVec->m_x * m_x) +
           (otherVec->m_y * m_y) +
           (otherVec->m_z * m_z);
}

void CVec3::CrossProduct(CVec3 *otherVec, CVec3 *result)
{
	result->m_x = (m_y * otherVec->m_z) - (m_z * otherVec->m_y);
	result->m_y = (m_z * otherVec->m_x) - (m_x * otherVec->m_z);
	result->m_z = (m_x * otherVec->m_y) - (m_y * otherVec->m_x);
}

void CVec3::Subtract(CVec3 *otherVec, CVec3 *result)
{
	result->m_x = m_x - otherVec->m_x;
	result->m_y = m_y - otherVec->m_y;
	result->m_z = m_z - otherVec->m_z;
}

void CVec3::Add(CVec3 *otherVec, CVec3 *result)
{
	result->m_x = m_x + otherVec->m_x;
	result->m_y = m_y + otherVec->m_y;
	result->m_z = m_z + otherVec->m_z;
}

void CVec3::Normalize(CVec3 *result)
{
    float length = sqrtf((m_x * m_x) + (m_y * m_y) + (m_z * m_z));
	result->m_x = m_x / length;
	result->m_y = m_y / length;
	result->m_z = m_z / length;
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

float CPlane::DistanceToPoint(CVec3& point)
{
	CVec3 pn;
	m_Normal.Normalize(&pn);
	return pn.DotProduct(&point) - pn.DotProduct(&m_Point);
}

float CPlane::Intersect(CVec3 lineStart, CVec3 lineEnd, CVec3 *out)
{
	CVec3 lineStartToEnd, lineToIntersect;

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
	m_v[0].Multiply(mtx, &out->m_v[0]);
	m_v[1].Multiply(mtx, &out->m_v[1]);
	m_v[2].Multiply(mtx, &out->m_v[2]);
}

void CTri::Translate(float x, float y, float z, CTri *out)
{
	m_v[0].Translate(x, y, z, &out->m_v[0]);
	m_v[1].Translate(x, y, z, &out->m_v[1]);
	m_v[2].Translate(x, y, z, &out->m_v[2]);
}

void CTri::Scale(float x, float y, float z, CTri *out)
{
    m_v[0].Scale(x, y, z, &out->m_v[0]);
	m_v[1].Scale(x, y, z, &out->m_v[1]);
	m_v[2].Scale(x, y, z, &out->m_v[2]);
}

void CTri::RotateX(float degrees, CTri *out)
{
	m_v[0].RotateX(degrees, &out->m_v[0]);
	m_v[1].RotateX(degrees, &out->m_v[1]);
	m_v[2].RotateX(degrees, &out->m_v[2]);
}

void CTri::RotateY(float degrees, CTri *out)
{
	m_v[0].RotateY(degrees, &out->m_v[0]);
	m_v[1].RotateY(degrees, &out->m_v[1]);
	m_v[2].RotateY(degrees, &out->m_v[2]);
}

void CTri::RotateZ(float degrees, CTri *out)
{
	m_v[0].RotateZ(degrees, &out->m_v[0]);
	m_v[1].RotateZ(degrees, &out->m_v[1]);
	m_v[2].RotateZ(degrees, &out->m_v[2]);
}

void CTri::YSortPoints(CTri *out)
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

bool CTri::ZSortBackToFrontCompare(CTri& tri1, CTri& tri2)
{
    CVec3 c1, c2;
    tri1.Center(&c1);
    tri2.Center(&c2);
    return c1.m_z > c2.m_z;
}

void CTri::Weigh2d(float x, float y, CVec3 *weights)
{
	CVec3 *v = m_v;

	float t0 = (v[1].m_y - v[2].m_y);
	float t1 = (x - v[2].m_x);
	float t2 = (v[2].m_x - v[1].m_x);
	float t3 = (y - v[2].m_y);
	float t4 = (v[0].m_x - v[2].m_x);
	float t5 = (v[0].m_y - v[2].m_y);
	float t6 = (v[2].m_y - v[0].m_y);

	float denom = (t0 * t4 + t2 * t5);

	float w1 = (t0 * t1 + t2 * t3) / denom;
	float w2 = (t6 * t1 + t4 * t3) / denom;
	float w3 = 1 - w1 - w2;

	if (w1 < 0) w1 = 0;
	if (w2 < 0) w2 = 0;
	if (w3 < 0) w3 = 0;

	weights->m_x = w1;
	weights->m_y = w2;
	weights->m_z = w3;
}

void CTri::Clip(CPlane clippingPlanes[], int numPlanes, std::vector<CTri>& trisOut)
{
	std::vector<CTri> outputTris;
	outputTris.push_back(*this);

	for (int nPlane = 0; nPlane < numPlanes; nPlane++)
	{
		std::vector<CTri> inputTris = outputTris;
		outputTris.clear();

		for (size_t nTri = 0; nTri < inputTris.size(); nTri++)
		{
			std::vector<int> insideIndeces;
			std::vector<int> outsideIndeces;

			for (int i = 0; i < 3; i++)
			{
				float d = clippingPlanes[nPlane].DistanceToPoint(inputTris[nTri].m_v[i]);

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
			if (insideIndeces.size() == 2)
			{
				// two new triangles
				CTri newTriA = inputTris[nTri], newTriB = inputTris[nTri];
				CVec3 intersectA, intersectB;

				CVec3 outsidePoint = inputTris[nTri].m_v[outsideIndeces[0]];
				CVec3 insidePointA = inputTris[nTri].m_v[insideIndeces[0]];
				CVec3 insidePointB = inputTris[nTri].m_v[insideIndeces[1]];

				clippingPlanes[nPlane].Intersect(insidePointA, outsidePoint, &intersectA);
				clippingPlanes[nPlane].Intersect(insidePointB, outsidePoint, &intersectB);

				newTriA.m_v[insideIndeces[0]] = insidePointA;
				newTriA.m_v[insideIndeces[1]] = intersectA;
				newTriA.m_v[outsideIndeces[0]] = intersectB;

				newTriB.m_v[insideIndeces[0]] = insidePointA;
				newTriB.m_v[insideIndeces[1]] = insidePointB;
				newTriB.m_v[outsideIndeces[0]] = intersectB;

				outputTris.push_back(newTriA);
				outputTris.push_back(newTriB);
			}
			else if (insideIndeces.size() == 1)
			{
				// new triangle
				CTri newTri = inputTris[nTri];
				CVec3 intersectA, intersectB;

				CVec3 insidePoint = inputTris[nTri].m_v[insideIndeces[0]];
				CVec3 outsidePointA = inputTris[nTri].m_v[outsideIndeces[0]];
				CVec3 outsidePointB = inputTris[nTri].m_v[outsideIndeces[1]];

				clippingPlanes[nPlane].Intersect(insidePoint, outsidePointA, &intersectA);
				clippingPlanes[nPlane].Intersect(insidePoint, outsidePointB, &intersectB);

				newTri.m_v[insideIndeces[0]] = insidePoint;
				newTri.m_v[outsideIndeces[0]] = intersectA;
				newTri.m_v[outsideIndeces[1]] = intersectB;

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

CCamera::CCamera(void) :
	m_Pos(0.0f, 0.0f, 0.0f),
	m_Yaw(0.0f),
	m_Pitch(0.0f),
	m_Roll(0.0f)
{
}

CCamera::CCamera(float x, float y, float z) :
	m_Pos(x, y, z),
	m_Yaw(0.0f),
	m_Pitch(0.0f),
	m_Roll(0.0f)
{
}

void CCamera::GetDirections(CVec3 *_forward, CVec3 *_up, CVec3 *_right)
{
	CVec3 forward(0, 0, 1);
	CVec3 up(0, 1, 0);
	CVec3 right;

	forward.RotateY(m_Yaw, &forward);
	forward.RotateX(m_Pitch, &forward);
	forward.RotateZ(m_Roll, &forward);

	CVec3 a;
	forward.Multiply(up.DotProduct(&forward), &a);
	up.Subtract(&a, &up);

	up.CrossProduct(&forward, &right);

	*_forward = forward;
	*_up = up;
	*_right = right;
}

void CCamera::GetViewMatrix(CMtx *out)
{
	CVec3 forward, up, right;
	GetDirections(&forward, &up, &right);

	out->m_m[0][0] = right.m_x;
	out->m_m[1][0] = right.m_y;
	out->m_m[2][0] = right.m_z;
	out->m_m[3][0] = -m_Pos.DotProduct(&right);

	out->m_m[0][1] = up.m_x;
	out->m_m[1][1] = up.m_y;
	out->m_m[2][1] = up.m_z;
	out->m_m[3][1] = -m_Pos.DotProduct(&up);

	out->m_m[0][2] = forward.m_x;
	out->m_m[1][2] = forward.m_y;
	out->m_m[2][2] = forward.m_z;
	out->m_m[3][2] = -m_Pos.DotProduct(&forward);

	out->m_m[0][3] = 0.0f;
	out->m_m[1][3] = 0.0f;
	out->m_m[2][3] = 0.0f;
	out->m_m[3][3] = 1.0f;
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
    CPlane topPlane(0, 0, 0, 0, 1, 0);
    CPlane bottomPlane(0, height - 1, 0, 0, -1, 0);
    CPlane leftPlane(0, 0, 0, 1, 0, 0);
    CPlane rightPlane(width - 1, 0, 0, -1, 0, 0);

    m_ClippingPlanes[0] = topPlane;
    m_ClippingPlanes[1] = bottomPlane;
    m_ClippingPlanes[2] = leftPlane;
    m_ClippingPlanes[3] = rightPlane;

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
	for (size_t i = 0; i < m_Width * m_Height; i++)
	{
		m_ColorBuffer[i] = 0x44444444;
	}
    memset(m_SelectBuffer, 0, bufferSize);
}

void CDrawBuffers::FillRect(int x, int y, int w, int h, uint32_t color)
{
	for (int cy = y; cy < y + h; cy++)
	{
		for (int cx = x; cx < x + w; cx++)
		{
			SetPixel(cx, cy, color);
		}
	}
}

void CDrawBuffers::DrawLine(int x0, int y0, int x1, int y1, uint32_t color)
{
    int deltaX = abs(x1 - x0);
    int deltaY = -abs(y1 - y0);
    int signX = x0 < x1 ? 1 : -1;
    int signY = y0 < y1 ? 1 : -1;
    int error = deltaX + deltaY;

    while (!(x0 == x1 && y0 == y1))
    {
        int e2 = 2 * error;

        if (e2 >= deltaY)
        {
            error += deltaY;
            x0 += signX;
        }

        if (e2 <= deltaX)
        {
            error += deltaX;
            y0 += signY;
        }

        SetPixel(x0, y0, color);
    }
}

void CDrawBuffers::DrawTriangle(CTri &tri, uint32_t clickIndex)
{
    std::vector<CTri> clippedTris;
    tri.Clip(m_ClippingPlanes, 4, clippedTris);
    
    for (int i = 0; i < clippedTris.size(); i++)
    {
        CTri clippedTri;
        clippedTris[i].YSortPoints(&clippedTri);
        CVec3 *v = clippedTri.m_v;

        v[0].m_x = (int)(v[0].m_x);
        v[0].m_y = (int)(v[0].m_y);
        v[1].m_x = (int)(v[1].m_x);
        v[1].m_y = (int)(v[1].m_y);
        v[2].m_x = (int)(v[2].m_x);
        v[2].m_y = (int)(v[2].m_y);

        double invslope1 = (v[1].m_x - v[0].m_x) / (v[1].m_y - v[0].m_y);
        double invslope2 = (v[2].m_x - v[0].m_x) / (v[2].m_y - v[0].m_y);
        double invslope3 = (v[2].m_x - v[1].m_x) / (v[2].m_y - v[1].m_y);

        // top tri, flat bottom
        if (v[0].m_y != v[1].m_y)
        {
            double x1 = v[0].m_x;
            double x2 = v[0].m_x;

            for (int y = v[0].m_y; y <= v[1].m_y; y++)
            {
                int cx1 = (x1 < x2) ? x1 : x2;
                int cx2 = (x1 > x2) ? x1 : x2;

                for (int x = cx1; x <= cx2; x++)
                {
                    SetPixel(x, y, tri.m_Color);
                    SetSelect(x, y, clickIndex);
                }

                x1 += invslope1;
                x2 += invslope2;
            }
        }

        // bottom tri, flat top
        if (v[1].m_y != v[2].m_y)
        {
            double x1 = v[2].m_x;
            double x2 = v[2].m_x;

            for (double y = v[2].m_y; y >= v[1].m_y; y--)
            {
                int cx1 = (x1 < x2) ? x1 : x2;
                int cx2 = (x1 > x2) ? x1 : x2;

                for (int x = cx1; x <= cx2; x++)
                {
                    SetPixel(x, y, tri.m_Color);
                    SetSelect(x, y, clickIndex);
                }

                x1 -= invslope2;
                x2 -= invslope3;
            }
        }
    }

	//DrawLine(tri.m_v[0].m_x, tri.m_v[0].m_y, tri.m_v[1].m_x, tri.m_v[1].m_y, 0x44444444);
	//DrawLine(tri.m_v[1].m_x, tri.m_v[1].m_y, tri.m_v[2].m_x, tri.m_v[2].m_y, 0x44444444);
	//DrawLine(tri.m_v[1].m_x, tri.m_v[2].m_y, tri.m_v[0].m_x, tri.m_v[0].m_y, 0x44444444);
	//FillRect(tri.m_v[0].m_x - 1, tri.m_v[0].m_y - 1, 2, 2, 0x22222222);
    //FillRect(tri.m_v[1].m_x - 1, tri.m_v[1].m_y - 1, 2, 2, 0x22222222);
    //FillRect(tri.m_v[2].m_x - 1, tri.m_v[2].m_y - 1, 2, 2, 0x22222222);
}

/********************/

void CDrawBuffers::Render(CBasicMeshGeometry *geom, CCamera *camera)
{
    Clear();

	CProjection projection(0.1f, 1000.0f, 90.0f, (float)m_Height / (float)m_Width);

    CMtx projectionMtx, viewMtx;
    projection.GetMtx(&projectionMtx);
	camera->GetViewMatrix(&viewMtx);

	CPlane nearAndFarPlanes[2] = {
		{ 0, 0, 0.5f,     0, 0, 1.0f },
		{ 0, 0, 1000.0f,  0, 0, -1.0f }
	};

    CVec3 lightDirection = { 0.0f, 0.0f, -1.0f };
    
    std::vector<CTri> projectedTris;

    size_t numTris = geom->GetNumTriangles();

	for (size_t i = 0; i < numTris; i++)
	{
        CTri tri;
        geom->GetTriangle(&tri, i);

        tri.index = i;
        tri.Scale(1.0f, -1.0f, -1.0f, &tri); // flip y and z

        CVec3 normal;
        tri.CalculateNormal(&normal);

        CVec3 temp;
        tri.m_v[0].Subtract(&camera->m_Pos, &temp);
        float d = normal.DotProduct(&temp);

        if (d > 0.0f) // cull backface
        {
            continue;
        }
        
        float dpLight = normal.DotProduct(&lightDirection);
        float intensity = (dpLight + 1.0f) * (200.0f / 2); // map -1:1 to 0:200

        if (geom->m_TriangleRefs[tri.index].bSelected)
        {
            tri.m_Color = RGB(intensity, intensity, 0);
        }
        else
        {
            tri.m_Color = RGB(intensity, intensity, intensity);
        }

		std::vector<CTri> clippedTris;
		tri.Multiply(&viewMtx, &tri);
		tri.Clip(nearAndFarPlanes, 2, clippedTris);

		for (size_t i = 0; i < clippedTris.size(); i++)
		{
			tri.Multiply(&projectionMtx, &tri);
			projectedTris.push_back(tri);
		}
	}

    // zsort projectedTris
    std::sort(projectedTris.begin(), projectedTris.end(), CTri::ZSortBackToFrontCompare);

    // generate bitmap
    for (size_t i = 0; i < projectedTris.size(); i++)
    {
        // map 0->1 to screen center->screen right edge
        CTri screenTri = projectedTris[i];
        screenTri.Scale(m_Width, m_Height, 1, &screenTri);
        screenTri.Translate(m_Width / 2, m_Height / 2, 0, &screenTri);
        DrawTriangle(screenTri, screenTri.index);
    }
}
