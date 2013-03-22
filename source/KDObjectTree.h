#pragma once
#include <vector>
#include <algorithm>
#include "Shapes.h"

enum Split_type_objects
{
	STO_X_COORD,
	STO_Y_COORD,
	STO_Z_COORD
};

class KDObjectTree
{
public:
	typedef std::vector<Shape*> Shapes;

	KDObjectTree(Shapes &shapes, BoundingBox &bb);
	~KDObjectTree();

	struct KDIntersectQueryReturn;
	KDIntersectQueryReturn FindIntersection(const Ray &ray, const double t_min, const double t_max);
	bool IsIntersection(const Ray &ray, const double t_min, const double t_max);

private:
	Shapes m_shapes;
	KDObjectTree *m_left, *m_right;
	BoundingBox m_BBox;
	Split_type_objects m_coord;
	double m_split_value; // D coord for plane (like "0*x + 1*y + 0*z - D = 0")

	//functions for sorting
	static bool x_comp_func_max(const Shape* i, const Shape* j);
	static bool x_comp_func_min(const Shape* i, const Shape* j);
	static bool y_comp_func_max(const Shape* i, const Shape* j);
	static bool y_comp_func_min(const Shape* i, const Shape* j);
	static bool z_comp_func_max(const Shape* i, const Shape* j);
	static bool z_comp_func_min(const Shape* i, const Shape* j);

	static const int emptyCost = 500;
};


/////////////////////////////////////////////////////////
// Вспомогательные структуры
/////////////////////////////////////////////////////////
struct KDObjectTree::KDIntersectQueryReturn
{
	// return index of primitive and distance to it (t in ray)
		  Shape		 *const  intersectShape;
	const double			 t_intersect;

	KDIntersectQueryReturn(
			  Shape		 *const shape,
		const double			t
		)
		:	intersectShape(shape),
			t_intersect(t)
	{}
};
/////////////////////////////////////////////////////////