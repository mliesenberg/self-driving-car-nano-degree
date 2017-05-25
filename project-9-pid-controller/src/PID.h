#ifndef PID_H
#define PID_H

class PID {
public:

  /*
  * Coefficients
  */
  double Kp;
  double Ki;
  double Kd;

  /*
  * keep cte history
  */
  double prior_cte;
  double sum_cte;

  /*
  * parametrize to keep manage different ranges for steering and speed
  */
  double min_range;
  double max_range;

  /*
  * Constructor
  */
  PID();

  /*
  * Destructor.
  */
  virtual ~PID();

  /*
  * Initialize PID.
  */
  void Init(double Kp, double Ki, double Kd, double min_range, double max_range);

  /*
   * Get the next value to send to the controller in main.cpp
   */
  double NextMeasurement(double cte);

};

#endif /* PID_H */
