#include "MaxHeap.h"

MaxHeap::MaxHeap()
	:	heapSize(0),
		arraySize(GLOBAL_PROPERTIES::kNearest + 1)
{
	heapArray = new Element[arraySize];
}

MaxHeap::~MaxHeap()
{
	delete[] heapArray;
}

void MaxHeap::Clear()
{
	heapSize = 0;
}

void MaxHeap::Add(const Element& newEl)
{
	heapArray[++heapSize] = newEl;
	size_t i = heapSize;
	while (i > 1 && heapArray[i].distance > heapArray[i>>1].distance)
	{
		SwapElements(i,i>>1);
		i >>= 1;
	}
}

const MaxHeap::Element MaxHeap::GetMax() const
{
	if (!heapSize) 
		throw Error("No elements in heap");
	return heapArray[1];
}

const MaxHeap::Element MaxHeap::RemoveMax()
{
	const Element ret = heapArray[1];
	if (!heapSize)
		throw Error("No elements in heap");
	heapArray[1] = heapArray[1+(--heapSize)];
	UpdateHeap(1);
	return ret;
}

const size_t MaxHeap::Size() const
{
	return heapSize;
}

const bool MaxHeap::IsEmpty() const
{
	return !heapSize;
}

void MaxHeap::UpdateHeap(const size_t numElem)
{
	size_t childL = (numElem << 1) > heapSize? numElem: (numElem<<1), 
		   childR = (childL  |  1) > heapSize? numElem: (childL | 1);
	if (heapArray[numElem].distance >= heapArray[childL].distance && heapArray[numElem].distance >= heapArray[childR].distance )
	{
		return;
	}
	size_t maxIndex = heapArray[childL].distance > heapArray[childR].distance ?  childL:childR;

	SwapElements(numElem, maxIndex);

	UpdateHeap(maxIndex);
}

void MaxHeap::SwapElements(const size_t i1,const size_t i2)
{
	const Element temp = heapArray[i1];
	heapArray[i1] = heapArray[i2];
	heapArray[i2] = temp;
}