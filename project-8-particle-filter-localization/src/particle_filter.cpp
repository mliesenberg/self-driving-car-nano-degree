/*
 * particle_filter.cpp
 *
 *  Created on: Dec 12, 2016
 *      Author: Tiffany Huang
 */

#include <random>
#include <algorithm>
#include <iostream>
#include <numeric>

#include "particle_filter.h"

void ParticleFilter::init(double gps_x, double gps_y, double theta, double std[]) {
  // Set the number of particles. Initialize all particles to first position (based on estimates of 
  //   x, y, theta and their uncertainties from GPS) and all weights to 1. 
  // Add random Gaussian noise to each particle.
  // NOTE: Consult particle_filter.h for more information about this method (and others in this file).
  std::default_random_engine gen;
  
  const double std_x = std[0];
  const double std_y = std[1];
  const double std_theta = std[2];
    
  // creating normal gaussian distributions for each variable.
  std::normal_distribution<double> dist_x(gps_x, std_x);
  std::normal_distribution<double> dist_y(gps_y, std_y);
  std::normal_distribution<double> dist_theta(theta, std_theta);
  
  num_particles = 75;
  
  // initialize all particles with noise using the distributions created above.
  for (int i = 0; i < num_particles; i++) {
    Particle particle;
    particle.id = i;
    
    const double sample_x = dist_x(gen);
    const double sample_y = dist_y(gen);
    const double sample_theta = dist_theta(gen);
    
    particle.x = sample_x;
    particle.y = sample_y;
    particle.theta = sample_theta;
    particle.weight = 1.0;
    
    particles.push_back(particle);
  }
  
  is_initialized = true;
}

void ParticleFilter::prediction(double delta_t, double std_pos[], double velocity, double yaw_rate) {
  // TODO: Add measurements to each particle and add random Gaussian noise.
  // NOTE: When adding noise you may find std::normal_distribution and std::default_random_engine useful.
  //  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
  //  http://www.cplusplus.com/reference/random/default_random_engine/

}

void ParticleFilter::dataAssociation(std::vector<LandmarkObs> predicted, std::vector<LandmarkObs>& observations) {
  // TODO: Find the predicted measurement that is closest to each observed measurement and assign the 
  //   observed measurement to this particular landmark.
  // NOTE: this method will NOT be called by the grading code. But you will probably find it useful to 
  //   implement this method and use it as a helper during the updateWeights phase.

}

void ParticleFilter::updateWeights(double sensor_range, 
                                   double std_landmark[], 
                                   std::vector<LandmarkObs> observations, 
                                   Map map_landmarks) {
  // TODO: Update the weights of each particle using a mult-variate Gaussian distribution. You can read
  //   more about this distribution here: https://en.wikipedia.org/wiki/Multivariate_normal_distribution
  // NOTE: The observations are given in the VEHICLE'S coordinate system. Your particles are located
  //   according to the MAP'S coordinate system. You will need to transform between the two systems.
  //   Keep in mind that this transformation requires both rotation AND translation (but no scaling).
  //   The following is a good resource for the theory:
  //   https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
  //   and the following is a good resource for the actual equation to implement (look at equation 
  //   3.33. Note that you'll need to switch the minus sign in that equation to a plus to account 
  //   for the fact that the map's y-axis actually points downwards.)
  //   http://planning.cs.uiuc.edu/node99.html
}

void ParticleFilter::resample() {
  // Resample particles with replacement with probability proportional to their weight. 
  // NOTE: You may find std::discrete_distribution helpful here.
  //   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
  
  std::default_random_engine gen;
  std::vector<double> particle_weights;
  std::vector<Particle> resampled_particles;
  
  for (Particle particle : particles) {
    particle_weights.push_back(particle.weight);
  }

  // discrete_distribution to sample particles with probability proportional to their weight
  std::discrete_distribution<> dd(particle_weights.begin(), particle_weights.end());
  
  for (int i = 0; i < num_particles; i++) {
    int idx_to_sample = dd(gen);
    resampled_particles.push_back(particles[idx_to_sample]);
  }
  
  // update state of the particle filter.
  particles = resampled_particles;
}

void ParticleFilter::write(std::string filename) {
  // You don't need to modify this file.
  std::ofstream dataFile;
  dataFile.open(filename, std::ios::app);
  for (int i = 0; i < num_particles; ++i) {
      dataFile << particles[i].x << " " << particles[i].y << " " << particles[i].theta << "\n";
  }
  dataFile.close();
}
