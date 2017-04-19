#include "ukf.h"
#include "tools.h"
#include "Eigen/Dense"
#include <iostream>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

/**
 * Initializes Unscented Kalman filter
 */
UKF::UKF() {
  // if this is false, laser measurements will be ignored (except during init)
  use_laser_ = true;

  // if this is false, radar measurements will be ignored (except during init)
  use_radar_ = true;

  // initial state vector
  x_ = VectorXd(5);

  // initial covariance matrix
  P_ = MatrixXd(5, 5);

  // Process noise standard deviation longitudinal acceleration in m/s^2
  std_a_ = 1;

  // Process noise standard deviation yaw acceleration in rad/s^2
  std_yawdd_ = 0.6;

  // Laser measurement noise standard deviation position1 in m
  std_laspx_ = 0.15;

  // Laser measurement noise standard deviation position2 in m
  std_laspy_ = 0.15;

  // Radar measurement noise standard deviation radius in m
  std_radr_ = 0.3;

  // Radar measurement noise standard deviation angle in rad
  std_radphi_ = 0.03;

  // Radar measurement noise standard deviation radius change in m/s
  std_radrd_ = 0.3;
  
  // State dimension
  n_x_ = 5;

  // Augmented state dimension
  n_aug_ = 7;

  // Sigma point spreading parameter
  lambda_ = 3 - n_aug_;

  // Weights of sigma points
  weights_ = VectorXd(2*n_aug_+1);;
  weights_(0) = lambda_ / (lambda_ + n_aug_);
  weights_.tail(2*n_aug_).fill( 0.5 / (lambda_ + n_aug_) );

  // create a 5D state vector, we don't know yet the values of the x state
  // [pos1 pos2 vel_abs yaw_angle yaw_rate] in SI units and rad
  x_ = VectorXd(n_x_);

  // state covariance matrix P
  // only position can be initialized with first measurement
  P_ = MatrixXd::Identity(n_x_, n_x_);

  // predicted sigma points matrix; we don't know yet the values
  Xsig_pred_ = MatrixXd(n_x_, 2 * n_aug_ + 1);

  // time when the state is true, in us
  time_us_ = 0;

  // the current NIS for radar
  NIS_radar_ = -99.0; //invalid

  // the current NIS for laser
  NIS_laser_ = -99.0; //invalid

  is_initialized_ = false;
}

UKF::~UKF() {}

/**
 * @param {MeasurementPackage} measurement_pack 
 * The latest measurement data of either radar or laser.
 */
void UKF::ProcessMeasurement(MeasurementPackage measurement_pack) {

  /*****************************************************************************
   *  Initialization
   ****************************************************************************/
  if (!is_initialized_) {

    // initialize with radar measurement
    if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
      /**
       *Convert radar from polar to cartesian coordinates and initialize state.
       */
      std::cout << "initializing with radar measurement" << std::endl;

      x_ << cos(measurement_pack.raw_measurements_[1]) * measurement_pack.raw_measurements_[0], // px
            sin(measurement_pack.raw_measurements_[1]) * measurement_pack.raw_measurements_[0], // py
            abs(measurement_pack.raw_measurements_[0]) * measurement_pack.raw_measurements_[2], // v_abs
            0., // psi
            0.; // psi_dot
      time_us_ = measurement_pack.timestamp_;
      is_initialized_ = true;
    } else if (measurement_pack.sensor_type_ == MeasurementPackage::LASER) {
      /**
       *Initialize state.
       */
      std::cout << "initializing with laser measurement" << std::endl;

      //set the state with the initial location and zero velocity
      x_ << measurement_pack.raw_measurements_[0], // px
            measurement_pack.raw_measurements_[1], // py
            0., // v_abs
            0., // psi
            0.; // psi_dot
      time_us_ = measurement_pack.timestamp_;
      is_initialized_ = true;
    } else {
      std::cout << "unkown sensor_type_" << measurement_pack.sensor_type_ << std::endl;
    }

    // done initializing, no need to predict or update
    return;
  }

  /*****************************************************************************
   *  Prediction
   ****************************************************************************/

  //compute the time elapsed between the current and previous measurements
  double delta_t = (measurement_pack.timestamp_ - time_us_) / 1000000.0;	//dt - expressed in seconds
  time_us_ = measurement_pack.timestamp_;

  while (delta_t > 0.1) {
    const double dt = 0.1;
    Prediction(dt);
    delta_t -= dt;
  }
  Prediction(delta_t);

  if (use_radar_ && measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
    // Radar updates
    UpdateRadar(measurement_pack);
  } else if (use_laser_ && measurement_pack.sensor_type_ == MeasurementPackage::LASER) {
    // Laser updates
    UpdateLidar(measurement_pack);
  } else {
    std::cout << "unknown sensor_type_ " << measurement_pack.sensor_type_ << std::endl;
  }
}

/**
 * Predicts sigma points, the state, and the state covariance matrix.
 * @param {double} delta_t the change in time (in seconds) between the last
 * measurement and this one.
 */
void UKF::Prediction(double delta_t) {

  /*****************************************************************************
   *  Create augmented sigma points
   ****************************************************************************/
  
  //create augmented mean vector
  VectorXd x_aug = VectorXd(n_aug_);
  //create augmented state covariance
  MatrixXd P_aug = MatrixXd(n_aug_, n_aug_);
  //create sigma point matrix
  MatrixXd Xsig_aug = MatrixXd(n_aug_, 2 * n_aug_ + 1);

  //set augmented mean state
  x_aug.head(5) = x_;
  x_aug(5) = 0;
  x_aug(6) = 0;

  //set augmented covariance matrix
  P_aug.fill(0.0);
  P_aug.topLeftCorner(n_x_,n_x_) = P_;
  P_aug(5,5) = std_a_*std_a_;
  P_aug(6,6) = std_yawdd_*std_yawdd_;
  
  //calculate square root matrix
  MatrixXd A = P_aug.llt().matrixL();
  
  //create augmented sigma points
  Xsig_aug.col(0) = x_aug;
  MatrixXd XsigDiff = sqrt(lambda_ + n_aug_) * A;
  for (int i=0; i<n_aug_; i++) {
    Xsig_aug.col(1+i) = x_aug + XsigDiff.col(i);
    Xsig_aug.col(1+i+n_aug_) = x_aug - XsigDiff.col(i);
  }

  /*****************************************************************************
  *  Predigt sigma points
  ****************************************************************************/
  //predict sigma points
  for (int i=0; i<2 * n_aug_ + 1; i++) {
    // for readability
    double px = Xsig_aug(0,i);
    double py = Xsig_aug(1,i);
    double v  = Xsig_aug(2,i);
    double psi = Xsig_aug(3,i);
    double psiDot = Xsig_aug(4,i);

    double nu_a = Xsig_aug(5,i);
    double nu_psiDDot = Xsig_aug(6,i);
    double px_pred, py_pred;
    
    //avoid division by zero
    if (fabs(psiDot) > 0.001) {
      px_pred = px + v/psiDot * ( sin (psi + psiDot*delta_t) - sin(psi));
      py_pred = py + v/psiDot * ( cos(psi) - cos(psi+psiDot*delta_t) );
    } else {
      px_pred = px + v*delta_t*cos(psi);
      py_pred = py + v*delta_t*sin(psi);
    }

    // add noise
    px_pred += 0.5*nu_a*delta_t*delta_t * cos(psi);
    py_pred += 0.5*nu_a*delta_t*delta_t * sin(psi);

    double v_pred = v + delta_t * nu_a;
    double psi_pred = psi + psiDot*delta_t + 0.5*delta_t*delta_t * nu_psiDDot;
    double psiDot_pred = psiDot + delta_t * nu_psiDDot;

    // write predicted sigma points into right column
    Xsig_pred_(0,i) = (((abs(px_pred)>0.0001) || (abs(py_pred)>0.0001)) ? px_pred : 0.0001); // avoid devision by zero later
    Xsig_pred_(1,i) = py_pred;
    Xsig_pred_(2,i) = v_pred;
    Xsig_pred_(3,i) = psi_pred;
    Xsig_pred_(4,i) = psiDot_pred;
  }

  /*****************************************************************************
  *  Calculate predicted state mean and covariance
  ****************************************************************************/
  //predicted state mean
  x_.fill(0.0);
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //iterate over sigma points
    x_ += weights_(i) * Xsig_pred_.col(i);
  }

  //predicted state covariance matrix
  P_.fill(0.0);
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {  // iterate over sigma points
    // state difference
    VectorXd x_diff = Xsig_pred_.col(i) - x_;
    //angle normalization
    while (x_diff(3) > M_PI) x_diff(3) -= 2. * M_PI;
    while (x_diff(3) < -M_PI) x_diff(3) += 2. * M_PI;

    P_ += weights_(i) * x_diff * x_diff.transpose() ;
  }
}

/**
 * Updates the state and the state covariance matrix using a laser measurement.
 * @param {MeasurementPackage} measurement_pack
 */
void UKF::UpdateLidar(MeasurementPackage measurement_pack) {
  //set measurement dimension, lidar can measure px and py
  int n_z = 2;

  // transform sigma points into measurement space
  MatrixXd Zsig = MatrixXd(n_z, 2 * n_aug_ + 1);
  for (int i=0; i<2 * n_aug_ + 1; i++) {
    Zsig(0,i) = Xsig_pred_(0,i);  // px
    Zsig(1,i) = Xsig_pred_(1,i);  // py
  }
  
  // calculate mean predicted measurement
  VectorXd z_pred = VectorXd(n_z);
  z_pred.fill(0.0);
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {  // iterate over sigma points
    z_pred += weights_(i) * Zsig.col(i);
  }
  
  // calculate measurement covariance matrix S and cross correlation matrix
  MatrixXd S = MatrixXd(n_z,n_z);
  S.fill(0.0);
  MatrixXd Tc = MatrixXd(n_x_, n_z);
  Tc.fill(0.0);

  for (int i = 0; i < 2 * n_aug_ + 1; i++) {  // iterate over sigma points

    // state difference
    VectorXd x_diff = Xsig_pred_.col(i) - x_;

    // angle normalization
    while (x_diff(3) > M_PI) x_diff(3) -= 2. * M_PI;
    while (x_diff(3) < -M_PI) x_diff(3) += 2. * M_PI;

    // residual
    VectorXd z_diff = Zsig.col(i) - z_pred;

    S += weights_(i) * z_diff * z_diff.transpose();
    Tc += weights_(i) * x_diff * z_diff.transpose();
  }
  S(0,0) += std_laspx_ * std_laspx_;
  S(1,1) += std_laspy_ * std_laspy_;

  /* Update state with lidar measurement */
  // calculate Kalman gain K;
  MatrixXd S_inv = S.inverse();
  MatrixXd K = Tc * S_inv;
  
  // update state mean and covariance matrix
  VectorXd z_diff_mean =  measurement_pack.raw_measurements_ - z_pred;
  x_ += K * z_diff_mean;
  P_ -= K * S * K.transpose();

  // calculate the NIS
  NIS_laser_ = z_diff_mean.transpose() * S_inv * z_diff_mean;
}

/**
 * Updates the state and the state covariance matrix using a radar measurement.
 * @param {MeasurementPackage} measurement_pack
 */
void UKF::UpdateRadar(MeasurementPackage measurement_pack) {
  // Predict radar measurement
  // set measurement dimension, radar can measure r, phi, and r_dot
  int n_z = 3;

  // transform sigma points into measurement space
  MatrixXd Zsig = MatrixXd(n_z, 2 * n_aug_ + 1);
  for (int i=0; i<2 * n_aug_ + 1; i++) {
    double px = Xsig_pred_(0,i);
    double py = Xsig_pred_(1,i);
    double v  = Xsig_pred_(2,i);
    double psi = Xsig_pred_(3,i);
    double psiDot = Xsig_pred_(4,i);

    Zsig(0,i) = sqrt(px * px + py * py);
    Zsig(1,i) = std::atan2(py, px);
    Zsig(2,i) = v * (px * cos(psi) + py * sin(psi)) / Zsig(0,i);
  }
  
  // calculate mean predicted measurement
  VectorXd z_pred = VectorXd(n_z);
  z_pred.fill(0.0);
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {  // iterate over sigma points
    z_pred += weights_(i) * Zsig.col(i);
  }
  
  // calculate measurement covariance matrix S
  // and cross correlation matrix
  MatrixXd S = MatrixXd(n_z,n_z);
  S.fill(0.0);
  MatrixXd Tc = MatrixXd(n_x_, n_z);
  Tc.fill(0.0);
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {  // iterate over sigma points
    // state difference
    VectorXd x_diff = Xsig_pred_.col(i) - x_;
    // angle normalization
    while (x_diff(3) > M_PI) x_diff(3) -= 2. * M_PI;
    while (x_diff(3) < -M_PI) x_diff(3) += 2. * M_PI;

    // residual
    VectorXd z_diff = Zsig.col(i) - z_pred;
    // angle normalization
    while (z_diff(1) > M_PI) z_diff(1) -= 2. * M_PI;
    while (z_diff(1) < -M_PI) z_diff(1) += 2. * M_PI;

    S += weights_(i) * z_diff * z_diff.transpose();
    Tc += weights_(i) * x_diff * z_diff.transpose();
  }

  S(0,0) += std_radr_ * std_radr_;
  S(1,1) += std_radphi_ * std_radphi_;
  S(2,2) += std_radrd_ * std_radrd_;

  // Update state with radar measurement
  // calculate Kalman gain K;
  MatrixXd S_inv = S.inverse();
  MatrixXd K = Tc * S_inv;
  // update state mean and covariance matrix
  VectorXd z_diff_mean =  measurement_pack.raw_measurements_ - z_pred;
  x_ += K * z_diff_mean;
  P_ -= K * S * K.transpose();

  // calculate the NIS
  NIS_radar_ = z_diff_mean.transpose() * S_inv * z_diff_mean;
}
