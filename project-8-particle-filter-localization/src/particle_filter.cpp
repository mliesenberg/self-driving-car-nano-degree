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

using namespace std;

double calculate_likelihood(double dx, double dy, double sigma_x, double sigma_y) {
  double temp = -0.5 * ((dx * dx / (sigma_x * sigma_x)) + (dy * dy / (sigma_y * sigma_y)));
  return exp(temp) / (2 * M_PI * sigma_x * sigma_y);
}

void ParticleFilter::init(double gps_x, double gps_y, double theta, double std[]) {

  default_random_engine gen;
  // creating normal gaussian distributions for each variable.
  normal_distribution<double> dist_x(gps_x, std[0]);
  normal_distribution<double> dist_y(gps_y, std[1]);
  normal_distribution<double> dist_theta(theta, std[2]);

  num_particles = 100;

  // initialize all particles with noise using the distributions created above.
  for (int i = 0; i < num_particles; i++) {
    Particle particle;
    particle.id = i;
    particle.x = dist_x(gen);
    particle.y = dist_y(gen);
    particle.theta = dist_theta(gen);
    particle.weight = 1.0;

    particles.push_back(particle);
  }
  is_initialized = true;
}

void ParticleFilter::prediction(double delta_t, double std_pos[], double velocity, double yaw_rate) {

  default_random_engine gen;
  // distributions to add measurement noise to the updates.
  normal_distribution<double> dist_x(0, std_pos[0]);
  normal_distribution<double> dist_y(0, std_pos[1]);
  normal_distribution<double> dist_theta(0, std_pos[2]);

  double velocity_per_yaw = (velocity / yaw_rate);
  double yaw_times_delta_t = yaw_rate * delta_t;

  for (Particle &particle : particles) {

    if (std::abs(yaw_rate) > 0.00001) {
      particle.x = particle.x + velocity_per_yaw * (sin(particle.theta + yaw_times_delta_t) - sin(particle.theta));
      particle.y = particle.y + velocity_per_yaw * (cos(particle.theta) - cos(particle.theta + yaw_times_delta_t));
    } else {
      particle.x = particle.x + velocity * delta_t * cos(particle.theta);
      particle.y = particle.y + velocity * delta_t * sin(particle.theta);
    }

    particle.theta = particle.theta + yaw_times_delta_t;

    particle.x += dist_x(gen);
    particle.y += dist_y(gen);
    particle.theta += dist_theta(gen);
  }
}

void ParticleFilter::dataAssociation(std::vector<LandmarkObs> predicted, std::vector<LandmarkObs>& observations) {

  const double max_val = std::numeric_limits<double>::max();

  for (auto &obs : observations) {

    double min_dist = max_val;

    for (int i = 0; i < predicted.size(); i++) {
      LandmarkObs &current_prediction = predicted[i];
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

  weights.clear();

  for (Particle &p : particles) {

    // convert from vehicle to map coordinates
    vector<LandmarkObs> mapped;
    // mapping from one coordinate system to the other.
    for (LandmarkObs &obs : observations) {
      LandmarkObs global;

      global.id = std::numeric_limits<double>::min();
      // translation - rotate and shift
      global.x = obs.x * cos(p.theta) - obs.y * sin(p.theta) + p.x;
      global.y = obs.x * sin(p.theta) + obs.y * cos(p.theta) + p.y;

      mapped.push_back(global);
    }

    // find predicted landmark for each observation
    std::vector<LandmarkObs> predicted_landmarks;
    for (auto &landmark : map_landmarks.landmark_list) {
      if (dist(landmark.x_f, landmark.y_f, p.x, p.y) <= sensor_range) {
        LandmarkObs l;
        l.id = landmark.id_i;
        l.x = landmark.x_f;
        l.y = landmark.y_f;
        predicted_landmarks.push_back(l);
      }
    }

    double new_weight = 1.0; // neutral element for *

    // no predictions found. not 100% sure what to do, assigning very small weight for now.
    if ((predicted_landmarks.size() == 0 && mapped.size() > 0) ||
        (mapped.size() == 0 && predicted_landmarks.size() > 0)) {
      new_weight = 1e-10; // TODO is this correct?
    } else {
      // use the nearest neighbor technique to associate observations and sensor measurements
      dataAssociation(predicted_landmarks, mapped);

      // calculate the multivariate gaussian probability density function for each landmark and
      // take the product of all of them to find the new weight
      for (LandmarkObs &o: mapped) {

        if (o.id != std::numeric_limits<double>::min()) {
          LandmarkObs &current = predicted_landmarks[o.id];
          double update = calculate_likelihood(current.x - o.x, current.y - o.y,
                                               std_landmark[0], std_landmark[1]);
          new_weight *= update;
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
