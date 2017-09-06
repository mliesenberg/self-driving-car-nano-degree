#ifndef PATH_PLANNING_CONSTANTS_H
#define PATH_PLANNING_CONSTANTS_H

const double SPEED_LIMIT = 49.5; // mph
const int LANE_WIDTH = 4; // m
const double WAYPOINT_SPACING = 40.0; // m
const double D_MAG = 1.0;
const double MPH_TO_MS = 2.24;
const double AT = 0.02; // accelaration time in secs

enum LANE_TYPE {
    LEFT_LANE, MIDDLE_LANE, RIGHT_LANE
};

const unsigned int LEFT_LANE_D = 2 + LANE_WIDTH * LEFT_LANE;
const unsigned int MIDDLE_LAND_D = 2 + LANE_WIDTH * MIDDLE_LANE;
const unsigned int RIGHT_LANE_D = 2 + LANE_WIDTH * RIGHT_LANE;

const int NUM_POINTS = 50;
const int D_INDEX = 6;
const int VX_INDEX = 3;
const int VY_INDEX = 4;
const int S_INDEX = 5;
const double SAFE_DISTANCE = 50.0;
const double SAFE_LANE_DISTANCE_FRONT = 75;
const double SAFE_LANE_DISTANCE_BACK = 50;
const double MIN_SPEED_FOR_LANE_CHANGE = 40; //mph
const double MIN_SPEED = 29.5; //mph

const double DELTA_VELOCITY = 0.224; // 5 m/s^2
#endif //PATH_PLANNING_CONSTANTS_H
