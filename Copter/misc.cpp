
#include "pch.h"
#include "Game.h"
#include <stdlib.h>

using namespace DirectX;
using namespace Copter;


Vertex cubeVertices[] = 
{ 
	{  0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f },
	{  0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f },
	{  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f },
	{  0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f },
		
	{ -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f },
	{ -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f },
	{ -0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f },
	{ -0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f },
		
	{  0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f },
	{ -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f },
	{ -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f },
	{  0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f },
		
	{  0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f },
	{ -0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f },
	{ -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f },
	{  0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f },
		
	{  0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f },
	{ -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f },
	{ -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f },
	{  0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f },
		
	{ -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f },
	{  0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f },
	{  0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f },
	{ -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f },
}; 
 
unsigned int cubeIndices[] = 
{ 
	0, 1, 2,	0, 3, 1,
	4, 5, 6,	4, 7, 5,
	8, 9, 10,	8, 11, 9,
	12, 13, 14,	12, 15, 13,
	16, 17, 18,	16, 19, 17,
	20, 21, 22,	20, 23, 21,
}; 



bool CheckCollision(float min1x, float max1x, float min1y, float max1y, float min2x, float max2x, float min2y, float max2y)
{
	if (min2x > max1x) return false;
	if (min1x > max2x) return false;

	if (min2y > max1y) return false;
	if (min1y > max2y) return false;

	return true;
}

bool CheckCollisionCopter(float minx, float maxx, float miny, float maxy, float copterx, float coptery)
{
	if (CheckCollision(minx, maxx, miny, maxy,
		copterx+COPTER_MINX1, copterx+COPTER_MAXX1, coptery+COPTER_MINY1, coptery+COPTER_MAXY1)) return true;

	return CheckCollision(minx, maxx, miny, maxy,
		copterx+COPTER_MINX2, copterx+COPTER_MAXX2, coptery+COPTER_MINY2, coptery+COPTER_MAXY2);
}



bool CheckCollision(float min1x, float max1x, float min1y, float max1y, float min1z, float max1z, 
					float min2x, float max2x, float min2y, float max2y, float min2z, float max2z, float out[3])
{
	if (min2x > max1x) return false;
	float overlap = max1x - min2x; out[0] = 1.0f; out[1] = 0.0f; out[2] = 0.0f;
	if (min1x > max2x) return false;
	if (max2x - min1x < overlap) { overlap = max2x - min1x; out[0] = -1.0f; out[1] = 0.0f; out[2] = 0.0f; }

	if (min2y > max1y) return false;
	if (max1y - min2y < overlap) { overlap = max1y - min2y; out[0] = 0.0f; out[1] = 1.0f; out[2] = 0.0f; }
	if (min1y > max2y) return false;
	if (max2y - min1y < overlap) { overlap = max2y - min1y; out[0] = 0.0f; out[1] = -1.0f; out[2] = 0.0f; }

	if (min2z > max1z) return false;
	if (max1z - min2z < overlap) { overlap = max1z - min2z; out[0] = 0.0f; out[1] = 0.0f; out[2] = 1.0f; }
	if (min1z > max2z) return false;
	if (max2z - min1z < overlap) { overlap = max2z - min1z; out[0] = 0.0f; out[1] = 0.0f; out[2] = -1.0f; }
	out[0] *= overlap; out[1] *= overlap; out[2] *= overlap;
	return true;
}


#define sign(x) (((x)<0?-1.0f:1.0f))
bool CheckCollision(const float center1[3], const float center2[3], const float extents1[3], const float extents2[3], float out[3])
{
	float d[3]; int i = 0;
	d[0] = fabs(center1[0] - center2[0]) - (extents1[0] + extents2[0]);
	if (d[0]>=0) return false;
	d[1] = fabs(center1[1] - center2[1]) - (extents1[1] + extents2[1]);
	if (d[1]>=0) return false;
	d[2] = fabs(center1[2] - center2[2]) - (extents1[2] + extents2[2]);
	if (d[2]>=0) return false;

	d[0] = -d[0]; d[1] = -d[1]; d[2] = -d[2];
	if (d[1]<d[0]) i = 1;
	if (d[2]<d[i]) i = 2;
	out[0] = 0.0f; out[1] = 0.0f; out[2] = 0.0f;
	out[i] = d[i] * sign(center2[i] - center1[i]);


	return true;
}

bool CheckCollisionCopter(float blockx, float blocky, float blockz, float extentx, float extenty, float extentz,
	float &copterx, float &coptery, float &copterz)
{
	float out[3];
	float extents1[3] = { extentx, extenty, extentz };
	float center1[3] = { blockx, blocky, blockz };


	float extents2[3] = { (COPTER3D_MAXX1 - COPTER3D_MINX1) / 2.0f, (COPTER3D_MAXY1 - COPTER3D_MINY1) / 2.0f, (COPTER3D_MAXZ1 - COPTER3D_MINZ1) / 2.0f };
	float center2[3] = { copterx + (COPTER3D_MAXX1 + COPTER3D_MINX1) / 2.0f, coptery + (COPTER3D_MAXY1 + COPTER3D_MINY1) / 2.0f, copterz + (COPTER3D_MAXZ1 + COPTER3D_MINZ1) / 2.0f };

	/*if (CheckCollision(minx, maxx, miny, maxy, minz, maxz,
		copterx + COPTER3D_MINX1, copterx + COPTER3D_MAXX1, coptery + COPTER3D_MINY1, coptery + COPTER3D_MAXY1,
		copterz + COPTER3D_MINZ1, copterz + COPTER3D_MAXZ1, out))*/
	if (CheckCollision(center1, center2, extents1, extents2, out))
	{
  		copterx += out[0];
		coptery += out[1];
		copterz += out[2];
		return true;
	}
	
	extents2[0] = (COPTER3D_MAXX2 - COPTER3D_MINX2) / 2.0f; extents2[1] = (COPTER3D_MAXY2 - COPTER3D_MINY2) / 2.0f;  extents2[2] = (COPTER3D_MAXZ2 - COPTER3D_MINZ2) / 2.0f;
	center2[0] = copterx + (COPTER3D_MAXX2 + COPTER3D_MINX2) / 2.0f; center2[1] = coptery + (COPTER3D_MAXY2 + COPTER3D_MINY2) / 2.0f; center2[2] = copterz + (COPTER3D_MAXZ2 + COPTER3D_MINZ2) / 2.0f;

	/*if (CheckCollision(minx, maxx, miny, maxy, minz, maxz,
		copterx + COPTER3D_MINX2, copterx + COPTER3D_MAXX2, coptery + COPTER3D_MINY2, coptery + COPTER3D_MAXY2,
		copterz + COPTER3D_MINZ2, copterz + COPTER3D_MAXZ2, out))*/
	if (CheckCollision(center1, center2, extents1, extents2, out))
	{
		copterx += out[0];
		coptery += out[1];
		copterz += out[2];
		return true;
	}
	return false;
}
