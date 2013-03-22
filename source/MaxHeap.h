#pragma once
#include "TraceLib.h"
#include <map>
#include <vector>

class MaxHeap
{
public:
	struct Element;
	
	MaxHeap();
	~MaxHeap();

	void Clear();

	void Add(const Element& newEl);
	const Element GetMax() const;
	const Element RemoveMax();
	const size_t Size() const;
	const bool IsEmpty() const;

private:
	void UpdateHeap(const size_t numElem);
	void SwapElements(const size_t i1,const size_t i2);

	Element* heapArray;
	size_t heapSize;
	size_t arraySize;
};

/////////////////////////////////////////////////////////
// Вспомогательные структуры
/////////////////////////////////////////////////////////
struct MaxHeap::Element
{
	friend class MaxHeap;

	double distance;
	Photon photon;

	Element(const double distance, const Photon& photon)
		:	distance(distance),
			photon(photon)
	{ }

private:
	Element() {}
};
/////////////////////////////////////////////////////////