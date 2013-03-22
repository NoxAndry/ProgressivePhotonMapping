#include "KDPhotonsTree.h"
#include <ctime>

KDPhotonsTree::KDPhotonsTree(Photons* toAdd)
{
	DWORD startTime = timeGetTime();

	size_t curNumberOfThreads = GLOBAL_PROPERTIES::numberOfThreads;
	m_logOfNumberOfThreads = 0;
	while(curNumberOfThreads>>=1)
		++m_logOfNumberOfThreads;

	size_t n = toAdd->size();
	if(!n) 
		throw Error("Empty photon vector");
	GLOBAL_PROPERTIES::numberOfProcessedPhotons += n;

	for(int i = 1 ; ; ++i) {
		n >>= i;
		if(!n) {
			n = 1 << i;
			break;
		}
		n <<= i;
	}


	m_numberOfPhotons = n;

	m_treeArray = new TreeElement[m_numberOfPhotons];

	InitTree(*toAdd, 1, toAdd->begin(), toAdd->end(), 0);

	m_photonsHeaps = new MaxHeap[GLOBAL_PROPERTIES::numberOfThreads];

	std::cout << "Photons KDtree build was over in " << (timeGetTime() - startTime) << " ms" << std::endl;

	delete toAdd;
}

KDPhotonsTree::~KDPhotonsTree()
{
	delete[] m_photonsHeaps;
	delete[] m_treeArray;
}

KDPhotonsTree::KDRadiusQueryReturn KDPhotonsTree::CalculatePhotonsInTheSphere(const Point &center, const double fixedRadius, const size_t shapeIndex, const Vector& normal) const
{
	KDRadiusQueryReturn ans(Color(), 0, 0, fixedRadius*fixedRadius);
	FindPhotonsWithinSphere(center, 1, shapeIndex, fixedRadius*fixedRadius, ans, normal);
	return ans;
}

double KDPhotonsTree::GetRadiusForHitPoint(const HitPoint& hitPoint, const size_t threadID)
{
	m_photonsHeaps[threadID].Clear();
	double d_square = GLOBAL_PROPERTIES::initMaxDist*GLOBAL_PROPERTIES::initMaxDist;
	FullHeap(hitPoint.position, 1, d_square, threadID);
	return sqrt(m_photonsHeaps[threadID].GetMax().distance);
}

void KDPhotonsTree::FindPhotonsWithinSphere(const Point& center, const size_t treeindex, const size_t shapeIndex, double fixedRadius, KDRadiusQueryReturn &ans, const Vector& normal) const
{
	if(m_treeArray[treeindex].split != ST_LEAF) {
		double sigma = 0;
		switch(m_treeArray[treeindex].split) {
		case ST_NON_EXIST:
			return;

		case ST_X_COORD:
			sigma = center.x() - m_treeArray[treeindex].photon.position.x();
			break;
		
		case ST_Y_COORD:
			sigma = center.y() - m_treeArray[treeindex].photon.position.y();
			break;
			
		case ST_Z_COORD:
			sigma = center.z() - m_treeArray[treeindex].photon.position.z();
			break;
		default: 
			throw Error("error in splitting dimension");
		}

		if(sigma < 0) 
		{
			FindPhotonsWithinSphere(center, treeindex << 1, shapeIndex, fixedRadius, ans, normal);
			if(sigma*sigma < fixedRadius)
			{
				FindPhotonsWithinSphere(center, (treeindex << 1) | 1, shapeIndex, fixedRadius, ans, normal);
			}
		}
		else 
		{
			FindPhotonsWithinSphere(center, (treeindex << 1) | 1, shapeIndex, fixedRadius, ans, normal);
			if(sigma*sigma < fixedRadius) 
			{
				FindPhotonsWithinSphere(center, treeindex << 1, shapeIndex, fixedRadius, ans, normal);
			}
		}

	}

	double sigma_square = ((m_treeArray[treeindex].photon.position-center).getSqLength());

	
#ifdef ELLIPSE_CORRECTION
	double bSq = 1;
	double aSq = 0.001;	
	double correction = normal%((center - treeArray[treeindex].photon.position));
	correction *= correction;
	correction *= bSq - aSq;
	correction *= fixedRadius;
	correction *= 100;
	sigma_square += correction;
#endif

	if(sigma_square < fixedRadius 
#ifdef SHAPE_CORRECTION
		&& m_treeArray[treeindex].photon.shapeIndex == shapeIndex
#endif
		)
	{
		// Change value of tau & increment number of photons
		
		ans.photonsEnergy += m_treeArray[treeindex].photon.power;
		ans.photonsNumber ++;
		if(m_treeArray[treeindex].photon.shapeIndex == shapeIndex)
			ans.photonsNumberOnSameShape ++;
		else
			if(sigma_square < ans.squaredLengthToFirstForeignPhoton)
				ans.squaredLengthToFirstForeignPhoton = sigma_square;
	}
}

void KDPhotonsTree::FullHeap(const Point& center, const size_t index, double& d_square, size_t threadID) {

	if(m_treeArray[index].split != ST_LEAF) {
		double sigma = 0;
		switch(m_treeArray[index].split) {
		case ST_NON_EXIST:
			return;

		case ST_X_COORD:
			sigma = center.x() - m_treeArray[index].photon.position.x();
			break;
		
		case ST_Y_COORD:
			sigma = center.y() - m_treeArray[index].photon.position.y();
			break;
			
		case ST_Z_COORD:
			sigma = center.z() - m_treeArray[index].photon.position.z();
			break;
		default:
			throw Error("error in splitting dimension");
		}

		if(sigma < 0) 
		{
			FullHeap(center, index << 1, d_square, threadID);
			if(sigma*sigma < d_square)
			{
				FullHeap(center, (index << 1) | 1, d_square, threadID);
			}
		}
		else 
		{
			FullHeap(center, (index << 1) | 1, d_square, threadID);
			if(sigma*sigma < d_square) 
			{
				FullHeap(center, index << 1, d_square, threadID);
			}
		}

	}

	double sigma_square = (m_treeArray[index].photon.position - center).getSqLength();
	if(sigma_square < d_square)
	{
		if(m_photonsHeaps[threadID].Size() >= GLOBAL_PROPERTIES::kNearest) 
		{
			m_photonsHeaps[threadID].RemoveMax();
			m_photonsHeaps[threadID].Add(HeapElement(sigma_square, m_treeArray[index].photon));
			d_square = m_photonsHeaps[threadID].GetMax().distance;
		} else {
			m_photonsHeaps[threadID].Add(HeapElement(sigma_square, m_treeArray[index].photon));
		}
		
	}
}

void KDPhotonsTree::InitTree(const Photons &photons, const size_t index, const Photons::iterator begin, const Photons::iterator end, const size_t depth) {
	const size_t dist_it = end - begin;
	if(dist_it < 0) {
		throw Error("initializing KD tree error");
	}
	if(dist_it < 1) {
		if(index < m_numberOfPhotons && begin != photons.end())m_treeArray[index] = TreeElement(*begin, ST_NON_EXIST);
		return;
	}

	if(dist_it == 1) {
		if(index < m_numberOfPhotons)m_treeArray[index] = TreeElement(*begin, ST_LEAF);
		return;
	}
	
	Point min( GLOBAL_PROPERTIES::INFINITY,  GLOBAL_PROPERTIES::INFINITY,  GLOBAL_PROPERTIES::INFINITY);
	Point max(-GLOBAL_PROPERTIES::INFINITY, -GLOBAL_PROPERTIES::INFINITY, -GLOBAL_PROPERTIES::INFINITY);
	for(Photons::iterator it = begin; it != end; it++) {
		if(it->position.x() < min.x()) min.x(it->position.x());
		if(it->position.y() < min.y()) min.y(it->position.y());
		if(it->position.z() < min.z()) min.z(it->position.z());
		if(it->position.x() > max.x()) max.x(it->position.x());
		if(it->position.y() > max.y()) max.y(it->position.y());
		if(it->position.z() > max.z()) max.z(it->position.z());
	}

	Point distPoint = max - min;
	Split_type split = 
			(distPoint.x() >= distPoint.y() && distPoint.x() >= distPoint.z()) ? ST_X_COORD 
			: ( (distPoint.y() >= distPoint.z()) ? ST_Y_COORD : ST_Z_COORD );

	std::sort(begin, end, split == ST_X_COORD ? x_comp_func : (split == ST_Y_COORD ? y_comp_func : z_comp_func) );

	const Photons::iterator middle = begin + ((end - begin)/2);
	if(((index << 1) | 1) < m_numberOfPhotons) {
		m_treeArray[index] = TreeElement(*middle, split);


		if(depth < m_logOfNumberOfThreads) {
			// Need to split to two threads (create extra one)

			/////////////////////////////////////////////////////////////
			// parallelization
			/////////////////////////////////////////////////////////////
			DWORD tID;
			HANDLE handle;

			PhotonsTreeBuildThreadArguments *arguments = 
				new PhotonsTreeBuildThreadArguments(*this, photons, (index << 1), begin, middle, depth+1);
			
			handle = CreateThread(
				NULL,
				0,
				ThreadBuildFunc,
				(LPVOID)arguments,
				0,
				&tID);
			/////////////////////////////////////////////////////////////

			// Complete second part of tree itself
			InitTree(photons, (index << 1) | 1, middle+1, end   , depth+1);

			// wait for sibling thread to finish his work
			WaitForSingleObject(handle, INFINITE);


		} else {
			InitTree(photons, (index << 1)    , begin   , middle, depth+1);
			InitTree(photons, (index << 1) | 1, middle+1, end   , depth+1);
		}
	} else {
		m_treeArray[index] = TreeElement(*middle, ST_LEAF);
	}
} 

bool KDPhotonsTree::x_comp_func(const Photon &ph1, const Photon &ph2)
{
	return ph1.position.x() < ph2.position.x();
}

bool KDPhotonsTree::y_comp_func(const Photon &ph1, const Photon &ph2)
{
	return ph1.position.y() < ph2.position.y();
}

bool KDPhotonsTree::z_comp_func(const Photon &ph1, const Photon &ph2)
{
	return ph1.position.z() < ph2.position.z();
}

DWORD WINAPI KDPhotonsTree::ThreadBuildFunc(LPVOID lp)
{
	try
	{
		PhotonsTreeBuildThreadArguments *arguments = 
			(PhotonsTreeBuildThreadArguments*)lp;

		arguments->tree.InitTree(
			arguments->photons, arguments->index, arguments->begin, arguments->end, arguments->depth);

	}
	catch(const Error& err)
	{
		err.HandleError();
	}
	catch(...)
	{
		Error::HandleDefaultError();
	}
	
	return 0;
}