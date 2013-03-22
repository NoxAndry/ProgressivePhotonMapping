#include "TraceLib.h"
#include "KDPhotonsTree.h"

HitPoint::HitPoint(const Point& position, 
					const int x, const int y, 
					const Color& summationCoeficient, 
					const Color& ambient, 
					const Color& specular, 
					const Color& textureCoeficent, 
					size_t shapeIndex, const Vector& normal)
		:	position(position),
			x(x), y(y),
			scalingCoeficient(summationCoeficient),
			ambient(ambient), specular(specular), 
			textureCoeficent(textureCoeficent), 
			shapeIndex(shapeIndex), 
			Normal(normal)
{
	Radiance = Flux = Color(0,0,0);
	Radius = GLOBAL_PROPERTIES::start_radius;
	NumberPhotons = 0;
}

void HitPoint::UpdateHitPoint(KDPhotonsTree* photonsTree)
{
	KDPhotonsTree::KDRadiusQueryReturn updater = photonsTree->CalculatePhotonsInTheSphere(position, Radius, shapeIndex, Normal);

	if (updater.photonsNumber == 0) return;

#ifdef ERROR_CORRECTION

	double error = 1. - updater.photonsNumberOnSameShape/(double)updater.photonsNumber;

	if(error > GLOBAL_PROPERTIES::ErrorCoeficent)
	{
		Flux = Color();
		NumberPhotons = 0;

		Radius = sqrt(updater.squaredLengthToFirstForeignPhoton - GLOBAL_PROPERTIES::EPS);

		updater = photonsTree->CalculatePhotonsInTheSphere(position, Radius, shapeIndex, Normal);

		if (updater.photonsNumber == 0) return;

		error = 1. - updater.photonsNumberOnSameShape/(double)updater.photonsNumber;
	}

#endif

	const double ratioCoef = (NumberPhotons + GLOBAL_PROPERTIES::PPMAlpha*updater.photonsNumber)/(NumberPhotons + updater.photonsNumber);

	Radius *= sqrt(ratioCoef);
	Flux = (Flux + updater.photonsEnergy) * ratioCoef;
	NumberPhotons += floor(updater.photonsNumber * GLOBAL_PROPERTIES::PPMAlpha + 1); 
	Radiance = (Flux / GLOBAL_PROPERTIES::numberOfEmittedPhotons) / (GLOBAL_PROPERTIES::PI * Radius * Radius);
}