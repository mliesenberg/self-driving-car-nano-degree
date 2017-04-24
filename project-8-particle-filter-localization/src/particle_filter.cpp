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
    weights.push_back(particle.weight);
  }
  
  is_initialized = true;
}

void ParticleFilter::prediction(double delta_t, double std_pos[], double velocity, double yaw_rate) {
  // Add measurements to each particle and add random Gaussian noise.
  // NOTE: When adding noise you may find std::normal_distribution and std::default_random_engine useful.
  //  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
  //  http://www.cplusplus.com/reference/random/default_random_engine/

  std::default_random_engine gen;

  const double std_x = std_pos[0];
  const double std_y = std_pos[1];
  const double std_theta = std_pos[2];

  const double velocity_per_yaw = (velocity / yaw_rate);
  const double yaw_times_delta_t = yaw_rate * delta_t;

  for (Particle particle : particles) {

    if (abs(yaw_rate) > 0.00001) {
      particle.x = particle.x + velocity_per_yaw * (sin(particle.theta + yaw_times_delta_t) - sin(particle.theta));
      particle.y = particle.y + velocity_per_yaw * (cos(particle.theta) - cos(particle.theta + yaw_times_delta_t));
    } else {
      particle.x = particle.x + velocity * delta_t * cos(particle.theta);
      particle.y = particle.y + velocity * delta_t * sin(particle.theta);
    }

    particle.theta = particle.theta + yaw_times_delta_t;

    // distributions to add measurement noise to the updates.
    std::normal_distribution<double> dist_x(particle.x, std_x);
    std::normal_distribution<double> dist_y(particle.y, std_y);
    std::normal_distribution<double> dist_theta(particle.theta, std_theta);

    particle.x = dist_x(gen);
    particle.y = dist_y(gen);
    particle.theta = dist_theta(gen);
  }
}

void ParticleFilter::dataAssociation(std::vector<LandmarkObs> predicted, std::vector<LandmarkObs>& observations) {
  // Find the predicted measurement that is closest to each observed measurement and assign the 
  //   observed measurement to this particular landmark.
  // NOTE: this method will NOT be called by the grading code. But you will probably find it useful to 
  //   implement this method and use it as a helper during the updateWeights phase.
  
  for (LandmarkObs obs : observations) {
    double min_dist = std::numeric_limits<double>::max();
    
    for (int i = 0; i < predicted.size(); i++) {
      LandmarkObs current_prediction = predicted[i];
      double distance = dist(current_prediction.x, current_prediction.y, obs.x, obs.y);
      
      if (distance < min_dist) {
        obs.id = i;
        min_dist = distance;
      }
    }
  }
}

void ParticleFilter::updateWeights(double sensor_range, 
                                   double std_landmark[], 
                                   std::vector<LandmarkObs> observations, 
                                   Map map_landmarks) {
  // Update the weights of each particle using a mult-variate Gaussian distribution. You can read
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
  for (Particle p : particles) {

    // convert from vehicle to map coordinates
    std::vector<LandmarkObs> mapped;
    // mapping from one coordinate system to the other.
    for (LandmarkObs obs : observations) {
      LandmarkObs observed;

      // translation - rotate and shift
      observed.x = obs.x * cos(p.theta) + obs.y * sin(p.theta) + p.x;
      observed.y = obs.x * sin(p.theta) + obs.y * cos(p.theta) + p.y;

      mapped.push_back(observed);
    }

    // find predicted landmark for each observation
    std::vector<LandmarkObs> predicted_landmarks;
    for (Map::single_landmark_s landmark : map_landmarks.landmark_list) {
      if (dist(landmark.x_f, landmark.y_f, p.x, p.y) <= sensor_range) {
        LandmarkObs observed;
        observed.id = landmark.id_i;
        observed.x = landmark.x_f;
        observed.y = landmark.y_f;
        predicted_landmarks.push_back(observed);
      }
    }

    double new_weight;

    // no predictions found. not 100% sure what to do, assigning very small weight for now.
    if((predicted_landmarks.size() == 0 && mapped.size() > 0) ||
        (mapped.size() == 0 && predicted_landmarks.size() > 0)) {
      new_weight = 1e-10; // TODO is this correct?
    } else {
      new_weight = 1; // neutral element of *

      // use the nearest neighbor technique to associate observations and sensor measurements
      dataAssociation(predicted_landmarks, mapped);

      // calculate the multivariate gaussian probability density function for each landmark and
      // take the product of all of them to find the new weight
      for (LandmarkObs o: mapped) {
        if (o.id > 0) {
          const LandmarkObs current = predicted_landmarks[o.id];
          //std::cout << current.x << " " << obs.x << " " << current.y << " " << obs.y << std::endl;
          //std::cout << "new gauss stuff: " << calculate_likelihood(current.x - obs.x, current.y - obs.y, std_landmark[0], std_landmark[1]) << std::endl;
          const double dx = current.x - o.x;
          const double dy = current.y - o.y;
          const double sigma_x = std_landmark[0];
          const double sigma_y = std_landmark[1];
          const double exp_expression = -0.5 * (((dx * dx) / (sigma_x * sigma_x)) + ((dy * dy) / (sigma_y * sigma_y)));
          const double temp = exp(exp_expression) / (2 * M_PI * sigma_x * sigma_y);
          //std::cout << " temp value: " << temp << std::endl;
          //new_weight *= calculate_likelihood(current.x - o.x, current.y - o.y, std_landmark[0], std_landmark[1]);
          new_weight *= temp;
        }
      }
    }

    // update particle
    p.weight = new_weight;
    // and weights array used for resampling
    weights.push_back(new_weight);
  }
}

void ParticleFilter::resample() {
  // Resample particles with replacement with probability proportional to their weight. 
  // NOTE: You may find std::discrete_distribution helpful here.
  //   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
  
  std::default_random_engine gen;
  std::vector<Particle> resampled_particles;
  
  // discrete_distribution to sample particles with probability proportional to their weight
  std::discrete_distribution<> dd(weights.begin(), weights.end());
  
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
