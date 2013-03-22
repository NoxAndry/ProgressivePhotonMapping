#include "KDObjectTree.h"

KDObjectTree::KDObjectTree(Shapes &shapes, BoundingBox &bb) 
	:  m_shapes(shapes), m_BBox(bb)
{
	m_left = m_right = NULL;
	
	size_t size = shapes.size();

	Shapes maxX; maxX.assign(shapes.begin(), shapes.end());
	Shapes maxY; maxY.assign(shapes.begin(), shapes.end());
	Shapes maxZ; maxZ.assign(shapes.begin(), shapes.end());
	Shapes minX; minX.assign(shapes.begin(), shapes.end());
	Shapes minY; minY.assign(shapes.begin(), shapes.end());
	Shapes minZ; minZ.assign(shapes.begin(), shapes.end());

	std::sort(maxX.begin(), maxX.end(), x_comp_func_max);
	std::sort(minX.begin(), minX.end(), x_comp_func_min);
	std::sort(maxY.begin(), maxY.end(), y_comp_func_max);
	std::sort(minY.begin(), minY.end(), y_comp_func_min);
	std::sort(maxZ.begin(), maxZ.end(), z_comp_func_max);
	std::sort(minZ.begin(), minZ.end(), z_comp_func_min);

		
	//////////////////////////////////////////////////////////
	// X COORD
	//////////////////////////////////////////////////////////
	double minSAHx = (m_BBox.maximum.x() - m_BBox.minimum.x())*size;
	double minSplitX = 0;
	for(size_t i=0, j=0, nl=0, nr=size; i<size-1; i++) {
		double maxXi = maxX[i]->BBox.maximum.x();
		for(;j<size;j++) {
			double minXj = minX[j]->BBox.minimum.x();
			if(minXj > maxXi)break;
			nl++;
		}
		nr--;

		double curSAH = ( nl*(maxXi - m_BBox.minimum.x()) + nr*(m_BBox.maximum.x() - maxXi) ) + emptyCost;
		if(curSAH < minSAHx) {
			minSAHx = curSAH;
			minSplitX = maxXi + GLOBAL_PROPERTIES::EPS;
		}
	}

	for(size_t i=0, j=0, nl=size, nr=0; i<size-1; i++) {
		double minXi = minX[size-1-i]->BBox.minimum.x();
		for(;j<size;j++) {
			double maxXj = maxX[size-1-j]->BBox.maximum.x();
			if(maxXj < minXi)break;
			nr++;
		}
		nl--;

		double curSAH = ( nl*(minXi - m_BBox.minimum.x()) + nr*(m_BBox.maximum.x() - minXi) ) + emptyCost;
		if(curSAH < minSAHx) {
			minSAHx = curSAH;
			minSplitX = minXi - GLOBAL_PROPERTIES::EPS;
		}
	}
	//////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////
	// Y COORD
	//////////////////////////////////////////////////////////
	double minSAHy = (m_BBox.maximum.y() - m_BBox.minimum.y())*size;
	double minSplitY = 0;
	for(size_t i=0, j=0, nl=0, nr=size; i<size-1; i++) {
		double maxYi = maxY[i]->BBox.maximum.y();
		for(;j<size;j++) {
			double minYj = minY[j]->BBox.minimum.y();
			if(minYj > maxYi)break;
			nl++;
		}
		nr--;

		double curSAH = ( nl*(maxYi - m_BBox.minimum.y()) + nr*(m_BBox.maximum.y() - maxYi) ) + emptyCost;
		if(curSAH < minSAHy) {
			minSAHy = curSAH;
			minSplitY = maxYi + GLOBAL_PROPERTIES::EPS;
		}
	}

	for(size_t i=0, j=0, nl=size, nr=0; i<size-1; i++) {
		double minYi = minY[size-1-i]->BBox.minimum.y();
		for(;j<size;j++) {
			double maxYj = maxY[size-1-j]->BBox.maximum.y();
			if(maxYj < minYi)break;
			nr++;
		}
		nl--;

		double curSAH = ( nl*(minYi - m_BBox.minimum.y()) + nr*(m_BBox.maximum.y() - minYi) ) + emptyCost;
		if(curSAH < minSAHy) {
			minSAHy = curSAH;
			minSplitY = minYi - GLOBAL_PROPERTIES::EPS;
		}
	}
	//////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////
	// Z COORD
	//////////////////////////////////////////////////////////
	double minSAHz = (m_BBox.maximum.z() - m_BBox.minimum.z())*size;
	double minSplitZ = 0;
	for(size_t i=0, j=0, nl=0, nr=size; i<size-1; i++) {
		double maxZi = maxZ[i]->BBox.maximum.z();
		for(;j<size;j++) {
			double minZj = minZ[j]->BBox.minimum.z();
			if(minZj > maxZi)break;
			nl++;
		}
		nr--;

		double curSAH = ( nl*(maxZi - m_BBox.minimum.z()) + nr*(m_BBox.maximum.z() - maxZi) ) + emptyCost;
		if(curSAH < minSAHz) {
			minSAHz = curSAH;
			minSplitZ = maxZi + GLOBAL_PROPERTIES::EPS;
		}
	}

	for(size_t i=0, j=0, nl=size, nr=0; i<size-1; i++) {
		double minZi = minZ[size-1-i]->BBox.minimum.z();
		for(;j<size;j++) {
			double maxZj = maxZ[size-1-j]->BBox.maximum.z();
			if(maxZj < minZi)break;
			nr++;
		}
		nl--;

		double curSAH = ( nl*(minZi - m_BBox.minimum.z()) + nr*(m_BBox.maximum.z() - minZi) ) + emptyCost;
		if(curSAH < minSAHz) {
			minSAHz = curSAH;
			minSplitZ = minZi - GLOBAL_PROPERTIES::EPS;
		}
	}
	//////////////////////////////////////////////////////////

	minSAHx *= (m_BBox.maximum.y() - m_BBox.minimum.y()) *
			   (m_BBox.maximum.z() - m_BBox.minimum.z());

	minSAHy *= (m_BBox.maximum.x() - m_BBox.minimum.x()) *
			   (m_BBox.maximum.z() - m_BBox.minimum.z());

	minSAHz *= (m_BBox.maximum.x() - m_BBox.minimum.x()) *
			   (m_BBox.maximum.y() - m_BBox.minimum.y());

	Shapes leftMas ;
	Shapes rightMas;

	BoundingBox leftBox (m_BBox);
	BoundingBox rightBox(m_BBox);

	if(minSAHx < (minSAHy - GLOBAL_PROPERTIES::EPS) && minSAHx < (minSAHz - GLOBAL_PROPERTIES::EPS)) {
		// X is the best split coord

		leftBox .maximum.x(minSplitX);
		rightBox.minimum.x(minSplitX);

		m_coord = STO_X_COORD;
		m_split_value = minSplitX;

		for(size_t i = 0; i<size; i++) {
			if(shapes[i]->BBox.maximum.x() > m_split_value)
					rightMas.push_back(shapes[i]);

			if(shapes[i]->BBox.minimum.x() < m_split_value)
					leftMas .push_back(shapes[i]);
		}

	} else if(minSAHy < (minSAHz - GLOBAL_PROPERTIES::EPS)) {
		// Y is the best split coord

		leftBox .maximum.y(minSplitY);
		rightBox.minimum.y(minSplitY);

		m_coord = STO_Y_COORD;
		m_split_value = minSplitY;

		for(size_t i = 0; i<size; i++) {
			if(shapes[i]->BBox.maximum.y() > m_split_value)
					rightMas.push_back(shapes[i]);

			if(shapes[i]->BBox.minimum.y() < m_split_value)
					leftMas .push_back(shapes[i]);

		}

	} else if(minSAHz < (minSAHy - GLOBAL_PROPERTIES::EPS)) {
		// Z is the best split coord

		leftBox .maximum.z(minSplitZ);
		rightBox.minimum.z(minSplitZ);

		m_coord = STO_Z_COORD;
		m_split_value = minSplitZ;

		for(size_t i = 0; i<size; i++) {
			if(shapes[i]->BBox.maximum.z() > m_split_value)
					rightMas.push_back(shapes[i]);

			if(shapes[i]->BBox.minimum.z() < m_split_value)
					leftMas .push_back(shapes[i]);
		}

	} else {
		// No need to split tree node
		return;
	}

	m_left  = new KDObjectTree(leftMas , leftBox );
	m_right = new KDObjectTree(rightMas, rightBox);

	shapes.clear();
}


KDObjectTree::~KDObjectTree() {
	if(m_left) delete m_left;
	if(m_right)delete m_right;
}

KDObjectTree::KDIntersectQueryReturn KDObjectTree::FindIntersection(const Ray &ray, const double t_min, const double t_max) {

	if(!m_left) {
		double min = GLOBAL_PROPERTIES::INFINITY;
		int minind = -1;
		for(size_t i = 0 ; i<m_shapes.size(); i++) {
			double t;
			if( (t = m_shapes[i]->Intersect(ray)) != (-GLOBAL_PROPERTIES::INFINITY) && t > GLOBAL_PROPERTIES::EPS && t < min) {
				min = t;
				minind = i;
			}
		}
		return KDIntersectQueryReturn(minind == -1 ? NULL : m_shapes[minind], min);
	} else {
		Plane splitPlane(
			m_coord == STO_X_COORD,
			m_coord == STO_Y_COORD, 
			m_coord == STO_Z_COORD,
			- m_split_value);
		double t_split = splitPlane.Intersect_param(ray);
		
		// find out if first point in the left box or not
		Point pos = ray.GetPointOnRay(t_min);
		bool isLeft;

		switch(m_coord)
		{
		case STO_X_COORD:
			isLeft = pos.x() < m_split_value;
			break;
		case STO_Y_COORD:
			isLeft = pos.y() < m_split_value;
			break;
		case STO_Z_COORD:
			isLeft = pos.z() < m_split_value;
			break;
		default:
			throw Error("impossible enum value");
		}
		


		if(t_split == (-GLOBAL_PROPERTIES::INFINITY) || t_split < t_min || t_split > t_max) {
			//
			// both t_min & t_max on the one side of plane
			//
			if(isLeft)return m_left->FindIntersection(ray, t_min, t_max);
			return m_right->FindIntersection(ray, t_min, t_max);
		} else {
			if(isLeft) {
				const KDIntersectQueryReturn t1 = m_left ->FindIntersection(ray, t_min, t_split);
				const KDIntersectQueryReturn t2 = m_right->FindIntersection(ray, t_split, t_max);
				

				if(t1.intersectShape == NULL)return t2;
				if(t2.intersectShape == NULL)return t1;

				return t1.t_intersect < t2.t_intersect ? t1 : t2;
			} else {
				KDIntersectQueryReturn t1 = m_right->FindIntersection(ray, t_min, t_split);
				KDIntersectQueryReturn t2 = m_left ->FindIntersection(ray, t_split, t_max);

				if(t1.intersectShape == NULL)return t2;
				if(t2.t_intersect == NULL)return t1;

				return t1.t_intersect < t2.t_intersect ? t1 : t2;

			}
		}
	}
}

bool KDObjectTree::IsIntersection(const Ray &ray, const double t_min, const double t_max) {
	if(!m_left) {
		for(size_t i = 0 ; i<m_shapes.size(); i++) {
			double t;
			if( (t = m_shapes[i]->Intersect(ray)) != (-GLOBAL_PROPERTIES::INFINITY) && t > GLOBAL_PROPERTIES::EPS && t < t_max) {
				return true;
			}
		}
		return false;
	} else {
		Plane splitPlane(
			m_coord == STO_X_COORD,
			m_coord == STO_Y_COORD, 
			m_coord == STO_Z_COORD,
			- m_split_value);
		double t_split = splitPlane.Intersect_param(ray);
		
		// find out if first point in the left box or not
		Point pos = ray.GetPointOnRay(t_min);
		bool isLeft;

		switch(m_coord)
		{
		case STO_X_COORD:
			isLeft = pos.x() < m_split_value;
			break;
		case STO_Y_COORD:
			isLeft = pos.y() < m_split_value;
			break;
		case STO_Z_COORD:
			isLeft = pos.z() < m_split_value;
			break;
		default:
			throw Error("impossible enum value");
		}				


		if(t_split == (-GLOBAL_PROPERTIES::INFINITY) || t_split < t_min || t_split > t_max) {
			//
			// both t_min & t_max on the same side of plane
			//
			if(isLeft)return m_left->IsIntersection(ray, t_min, t_max);
			return m_right->IsIntersection(ray, t_min, t_max);
		} else {
			if(isLeft) {
				return m_left ->IsIntersection(ray, t_min, t_split) 
					|| m_right->IsIntersection(ray, t_split, t_max);
				
			} else {
				return m_right->IsIntersection(ray, t_min, t_split)
					|| m_left ->IsIntersection(ray, t_split, t_max);

			}
		}
	}
}

bool KDObjectTree::x_comp_func_max(const Shape* i, const Shape* j) {
	return i->BBox.maximum.x() < j->BBox.maximum.x();
}

bool KDObjectTree::x_comp_func_min(const Shape* i, const Shape* j) {
	return i->BBox.minimum.x() < j->BBox.minimum.x();
}

bool KDObjectTree::y_comp_func_max(const Shape* i, const Shape* j) {
	return i->BBox.maximum.y() < j->BBox.maximum.y();
}

bool KDObjectTree::y_comp_func_min(const Shape* i, const Shape* j) {
	return i->BBox.minimum.y() < j->BBox.minimum.y();
}

bool KDObjectTree::z_comp_func_max(const Shape* i, const Shape* j) {
	return i->BBox.maximum.z() < j->BBox.maximum.z();
}

bool KDObjectTree::z_comp_func_min(const Shape* i, const Shape* j) {
	return i->BBox.minimum.z() < j->BBox.minimum.z();
}