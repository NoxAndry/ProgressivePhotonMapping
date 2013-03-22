#pragma once
#include "MaxHeap.h"
#include <vector>
#include "GeometryLib.h"
#include <Windows.h>
#include "TraceLib.h"

enum Split_type
{
	ST_NON_EXIST,
	ST_X_COORD,
	ST_Y_COORD,
	ST_Z_COORD,
	ST_LEAF
};

class KDPhotonsTree
{ 
public:
	typedef MaxHeap::Element HeapElement;
	typedef std::vector<Photon> Photons;

	KDPhotonsTree(Photons* toAdd);
	~KDPhotonsTree();

	struct KDRadiusQueryReturn;
	KDRadiusQueryReturn CalculatePhotonsInTheSphere(const Point &point, const double fixedRadius, const size_t shapeIndex, const Vector& normal) const;
	double GetRadiusForHitPoint(const HitPoint& hitPoint, const size_t threadID);

private:
	struct TreeElement;
	struct PhotonsTreeBuildThreadArguments;

	TreeElement *m_treeArray;
	size_t m_numberOfPhotons;
	MaxHeap *m_photonsHeaps;

	size_t m_logOfNumberOfThreads;


	void FindPhotonsWithinSphere(const Point& center, const size_t treeIndex, const size_t shapeIndex, double fixedRadius, KDRadiusQueryReturn &ans /*out*/, const Vector& normal) const;
	void FullHeap(const Point& center, const size_t index, double& d_square, size_t threadID);
	void InitTree(const Photons &photons, const size_t index, const Photons::iterator begin, const Photons::iterator end, const size_t depth);

	static DWORD WINAPI ThreadBuildFunc(LPVOID lp);
	
	static bool x_comp_func(const Photon&, const Photon&);
	static bool y_comp_func(const Photon&, const Photon&);
	static bool z_comp_func(const Photon&, const Photon&);
};

/////////////////////////////////////////////////////////
// Вспомогательные структуры
/////////////////////////////////////////////////////////
struct KDPhotonsTree::KDRadiusQueryReturn
{
	Color  photonsEnergy;
	size_t photonsNumber;
	size_t photonsNumberOnSameShape;
	double squaredLengthToFirstForeignPhoton;

	KDRadiusQueryReturn(const Color& pE, const size_t pN, const size_t pNS, const double d)
		: photonsEnergy(pE), photonsNumber(pN), photonsNumberOnSameShape(pNS), squaredLengthToFirstForeignPhoton(d)
	{ }
};

struct KDPhotonsTree::TreeElement {
	Photon photon;
	Split_type split; // splitting plane for KD

	TreeElement() : photon(), split(Split_type::ST_NON_EXIST) {}
	TreeElement(const Photon &photon, const Split_type split) : photon(photon), split(split) {}
};

struct KDPhotonsTree::PhotonsTreeBuildThreadArguments
{
	typedef KDPhotonsTree::Photons Photons;

		  KDPhotonsTree				&tree;
	const Photons					&photons;
	const size_t					 index;
	const Photons::iterator			 begin;
	const Photons::iterator			 end;
	const size_t					 depth;
	
	PhotonsTreeBuildThreadArguments(
				  KDPhotonsTree			&tree,
			const Photons				&photons,
			const size_t				 index,
			const Photons::iterator		 begin,
			const Photons::iterator		 end,
			const size_t				 depth
		)
	:	tree(tree),
		photons(photons),
		index(index),
		begin(begin),
		end(end),
		depth(depth)
	{}
};
/////////////////////////////////////////////////////////

