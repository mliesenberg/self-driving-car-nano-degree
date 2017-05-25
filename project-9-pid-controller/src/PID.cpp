#include "PID.h"
#include "math.h"

PID::PID() {}

PID::~PID() {}

void PID::Init(double Kp, double Ki, double Kd, double min_range, double max_range) {

  prior_cte = 0.0;
  sum_cte = 0.0;

  // Initializing coefficients to given values.
  // see README.md for reasoning on initial values
  this->Kp = Kp;
  this->Ki = Ki;
  this->Kd = Kd;

  // Initializing ranges
  this->min_range = min_range;
  this->max_range = max_range;
}

double PID::NextMeasurement(double cte) {

	// Calculate the next measurement, replacing UpdateError
	double measurement = -Kp * cte - Kd * (prior_cte - cte) - Ki * sum_cte;

	// Since output values should be in the interval [-1. 1], let's apply a sigmoid transform
	measurement = (max_range - min_range) * (1 / (1 + exp(-measurement))) - (-1 * min_range);

	// Update the cte history variables
	prior_cte = cte;
	sum_cte += cte;

	return measurement;
}
