#pragma once

#include <string>
#include "GeometryLib.h"
#include "TraceLib.h"
#include "freeglut\freeglut.h"

class Camera {
public:
	Camera(const Point &pos, const Point &targ);
	~Camera();
	void SetTransform();
	void Move_forward(const float delta);
	void Move_left	 (const float delta);
	void Rotate      (const float alpha);
	Ray GetRayForTracing(float pixel_x_coord, float pixel_y_coord) const;

private:
	void UpdateAngle();

	Point m_position;
	Vector m_target, m_up;
	double m_angle;
	size_t m_height;
};