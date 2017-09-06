#include "Constants.h"
#include "OtherCar.h"
#include <vector>

using namespace std;

#ifndef PATH_PLANNING_BEHAVIORPLANNER_H
#define PATH_PLANNING_BEHAVIORPLANNER_H


class BehaviorPlanner {
    static bool is_lane_safe(vector<OtherCar> lane_cars, double car_s);
public:
    static LANE_TYPE
    reco_lane(double ref_vel, LANE_TYPE lane, vector <OtherCar> left_lane, vector <OtherCar> middle_lane,
              vector <OtherCar> right_lane, double car_s);
};


#endif //PATH_PLANNING_BEHAVIORPLANNER_H
