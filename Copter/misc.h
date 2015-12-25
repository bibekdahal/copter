
#define COPTER3D_MINX1 -0.246f
#define COPTER3D_MINY1 -0.236f
#define COPTER3D_MINZ1 -0.381f
#define COPTER3D_MAXX1 0.246f
#define COPTER3D_MAXY1 0.246f
#define COPTER3D_MAXZ1 0.619f

#define COPTER3D_MINX2 -0.063f
#define COPTER3D_MINY2 -0.182f
#define COPTER3D_MINZ2 -1.23999f
#define COPTER3D_MAXX2 0.063f
#define COPTER3D_MAXY2 0.098f
#define COPTER3D_MAXZ2 -0.347993f


#define COPTER_MINX1 8.0f
#define COPTER_MAXX1 60.0f
#define COPTER_MINY1 22.0f
#define COPTER_MAXY1 48.0f

#define COPTER_MINX2 60.0f
#define COPTER_MAXX2 123.0f
#define COPTER_MINY2 25.0f
#define COPTER_MAXY2 64.0f


#define COPTER_WIDTH 135.0f
#define COPTER_HEIGHT 89.0f


bool CheckCollision(float min1x, float max1x, float min1y, float max1y, float min2x, float max2x, float min2y, float max2y);
bool CheckCollisionCopter(float minx, float maxx, float miny, float maxy, float copterx, float coptery);

bool CheckCollision(float min1x, float max1x, float min1y, float max1y, float min1z, float max1z, 
					float min2x, float max2x, float min2y, float max2y, float min2z, float max2z, float out[3]);
bool CheckCollision(const float center1[3], const float center2[3], const float extents1[3], const float extents2[3], float out[3]);
/*bool CheckCollisionCopter(float minx, float maxx, float miny, float maxy, float minz, float maxz, 
						  float &copterx, float &coptery, float &copterz);*/
bool CheckCollisionCopter(float blockx, float blocky, float blockz, float extentx, float extenty, float extentz,
	float &copterx, float &coptery, float &copterz);