#pragma once

#include <iostream>

void separate();

struct CAMERA_PROPERTIES
{
	static		 size_t width;
	static		 size_t height;
	static const double focus_distance;
	static const double view_angle;
	static const double camera_speed;
	static const double camera_rotation_speed;
};

struct GLOBAL_PROPERTIES
{
	static const double start_radius;
	static const size_t NumberPhotonTraseSteps;
	static const double attenuationCoeficient;
	static const size_t mappingTime;
	static const double EPS;
	static const double INFINITY;
	static const double PI;
	static const size_t kNearest;
	static const double initMaxDist;
	static const size_t numberOfPhotonsFirstIteration;
	static const size_t numberOfPhotonsPerIteration;
	static		 size_t numberOfEmittedPhotons;
	static		 size_t numberOfProcessedPhotons;
	static const double attenuationThreshold;
	static const unsigned char diffuseReflectionCoefficient;
	static const double alphaCoeficent;
	static const double PPMAlpha; 
	static const float  SamplingCoefficient;
	static const size_t numberOfThreads;
	static const double ErrorCoeficent;
	static const size_t hitNumberThreshold;
};