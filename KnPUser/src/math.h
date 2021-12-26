#pragma once

namespace math
{
	typedef struct
	{
		float x, y;
	} vec2d_f;

	typedef struct
	{
		float x, y, z;
	} vec3d_f;

	typedef struct
	{
		float x, y, z, w;
	} vec4d_f;

	bool WorldToScreen(const vec3d_f& pos, vec4d_f& clipCoords, vec4d_f& NDC, vec3d_f& screen, float matrix[16],const int& windowWidth,const int& windowHeight);

	float GetDistance3D(vec3d_f m_pos, vec3d_f en_pos);
}