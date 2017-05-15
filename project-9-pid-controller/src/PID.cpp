#include "PID.h"

PID::PID() {}

PID::~PID() {}

void PID::Init(double Kp, double Ki, double Kd) {
  // Initializing errors to 0
  this->p_error = 0.0;
  this->i_error = 0.0;
  this->d_error = 0.0;

  // Initializing coefficients to given values.
  // see README.md for reasoning on initial values
  this->Kp = Kp;
  this->Ki = Ki;
  this->Kd = Kd;
}

void PID::UpdateError(double cte) {
  d_error = cte - p_error;
  p_error = cte;
  i_error += cte;
}

double PID::TotalError() {
  return this->Kp * this->p_error +
         this->Kd * this->d_error +
         this->Ki * this->i_error;
}
