#include "BehaviorPlanner.h"
#include "Constants.h"
#include "OtherCar.h"
#include <vector>
#include <iostream>

#include <math.h>

LANE_TYPE
BehaviorPlanner::reco_lane(double ref_vel,
                           LANE_TYPE lane,
                           vector<OtherCar> left_lane,
                           vector<OtherCar> middle_lane,
                           vector<OtherCar> right_lane,
                           double car_s) {
    LANE_TYPE reco_lane;
    reco_lane = lane;

    if (ref_vel > MIN_SPEED && ref_vel < MIN_SPEED_FOR_LANE_CHANGE) {
        if (lane == LANE_TYPE::MIDDLE_LANE) {
            if (is_lane_safe(left_lane, car_s)) {
                // try to change left
                cout << "CHANGE_LEFT" << "\n";
                reco_lane = LANE_TYPE::LEFT_LANE;
            } else {
                // try to change right
                if (is_lane_safe(right_lane, car_s)) {
                    cout << "CHANGE_RIGHT" << "\n";
                    reco_lane = LANE_TYPE::RIGHT_LANE;
                }
            }
        } else if (lane == LANE_TYPE::LEFT_LANE) {
            // try to change middle
            if (is_lane_safe(middle_lane, car_s)) {
                cout << "CHANGE_MIDDLE" << "\n";
                reco_lane = LANE_TYPE::MIDDLE_LANE;
            }
        } else {
            // try to change middle
            if (is_lane_safe(middle_lane, car_s)) {
                cout << "CHANGE_MIDDLE" << "\n";
                reco_lane = LANE_TYPE::MIDDLE_LANE;
            }
        }
    }
    return reco_lane;
}

bool BehaviorPlanner::is_lane_safe(vector<OtherCar> destination_lane_cars, double car_s) {
    for (int i = 0; i < destination_lane_cars.size(); i++) {
        double distance = destination_lane_cars[i].s - car_s;
        if (distance > 0 && distance < SAFE_LANE_DISTANCE_FRONT) {
            return false;
        } else {
            // distance < 0
            if (fabs(distance) < SAFE_LANE_DISTANCE_BACK) {
                return false;
            }
        }
    }
    return true;
}
