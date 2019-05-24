#pragma once

#include <stdafx.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159f
#endif

class CMtx
{
public:
	float m_m[4][4];
	CMtx(void);
};

class CVec3
{
public:
	float m_x, m_y, m_z;
	CVec3(void);
	CVec3(float x, float y, float z);
	void Mult(CVec3 *out, CMtx* mtx);
	void Translate(CVec3 *out, float x, float y, float z);
    void Scale(CVec3 *out, float x, float y, float z);
	void RotateX(CVec3 *out, float degrees);
	void RotateY(CVec3 *out, float degrees);
	void RotateZ(CVec3 *out, float degrees);
    void Subtract(CVec3 *in, CVec3 *out);
    float DotProduct(CVec3 *otherVec);
    void Normalize(CVec3 *out);
};

class CTri
{
public:
    CTri(void);
	CVec3 m_v[3];
    COLORREF m_Color;
	void Mult(CTri *out, CMtx *mtx);
	void Translate(CTri *out, float x, float y, float z);
    void Scale(CTri *out, float x, float y, float z);
	void RotateX(CTri *out, float degrees);
	void RotateY(CTri *out, float degrees);
	void RotateZ(CTri *out, float degrees);
    void CalculateNormal(CVec3 *out);
    void Center(CVec3 *out);
};

class CProjection
{
private:
	CProjection(void);
public:
	float m_Near;
	float m_Far;
	float m_Fov;
	float m_AspectRatio;
	CProjection(float fnear, float ffar, float fov, float aspectRatio);
	void GetMtx(CMtx *out);
};

typedef struct
{
	size_t vidx0, vidx1, vidx2;
} geom_tri_ref_t;

class CBasicMeshGeometry
{
	std::vector<CVec3> m_Vertices;
	std::vector<geom_tri_ref_t> m_TriangleRefs;
public:
    void AddVertex(CVec3 vertex);
    size_t AddVertexUnique(CVec3 vertex);
	void AddTriangleRef(size_t vidx0, size_t vidx1, size_t vidx2);
	void AddTriangleRef(geom_tri_ref_t triref);
	void AddVertices(CVec3 vertices[], size_t count);
	void AddTriangleRefs(geom_tri_ref_t trirefs[], size_t count);
	size_t GetNumTriangles(void);
	bool GetTriangle(CTri *out, size_t index);
};

class CGeometryInstance
{
    CMtx m_Matrix;
    uint32_t m_GeometryKey; // key of scene.m_Geometries
    CGeometryInstance* m_Parent;
    std::vector<CGeometryInstance> m_Children;
};

class CScene
{
    // use dl address
    std::map<uint32_t, CBasicMeshGeometry> m_Geometries;
    //CGeometryInstance m_RootInstance;
public:
    bool AddGeometry(uint32_t dlAddr);
};

void Test_3d(HWND hwnd, CBasicMeshGeometry *geom);
