#include "Camera.h"
#include <cmath>

Camera::Camera(const Point &pos, const Point &targ) 
	:	
		m_position(pos),
		m_target(pos + (!(targ - pos)) ),
		m_up		(0, 0, 1)
{

}

Camera::~Camera()
{
}

void Camera::SetTransform()
{
	gluLookAt(m_position.x(), m_position.y(), m_position.z(),
			  m_target.x()  , m_target.y()	, m_target.z()	,
			  m_up.x()      , m_up.y()		, m_up.z()       );
}

void Camera::Rotate(const float alpha)
{
	m_angle += alpha;
	if(m_angle < 0						)m_angle+=2*GLOBAL_PROPERTIES::PI;
	if(m_angle > 2*GLOBAL_PROPERTIES::PI)m_angle-=2*GLOBAL_PROPERTIES::PI;

	UpdateAngle();
}

void Camera::UpdateAngle()
{
	Point d = m_target - m_position;
	
	m_target = Point(sin(m_angle), cos(m_angle), 0);
	m_target += m_position;
}

void Camera::Move_forward(const float delta)
{
	Point d = m_target - m_position;

	m_position += (d*delta);
	m_target   += (d*delta);
}

void Camera::Move_left(const float delta)
{
	Point d = m_target - m_position;
	
	double tmp = d.y(); d.y(d.x()); d.x(-tmp);

	m_position += (d*delta);
	m_target   += (d*delta);
}

Ray Camera::GetRayForTracing(float pixel_x_coord, float pixel_y_coord) const 
{
	static double width  = 2*(sin(CAMERA_PROPERTIES::view_angle / 2));
	static double height = ( CAMERA_PROPERTIES::height / (double)CAMERA_PROPERTIES::width ) * width;

	Vector direction( !(m_target - m_position) );

	Vector right = direction*m_up;

	pixel_y_coord = CAMERA_PROPERTIES::height - pixel_y_coord;

	double x = (pixel_x_coord - CAMERA_PROPERTIES::width/2.);
	double y = (pixel_y_coord - CAMERA_PROPERTIES::height/2.);
	
	Point target =	right*(width  * x / (CAMERA_PROPERTIES::width  - 1) )
				+	m_up *(height * y / (CAMERA_PROPERTIES::height - 1) );

	target += direction;

	return Ray(m_position, !target, 1);
}