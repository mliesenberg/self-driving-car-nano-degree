#include "Constants.h"

#ifndef PATH_PLANNING_LANE_H
#define PATH_PLANNING_LANE_H

namespace {
    namespace lane_helper {
        int get_lane_d(int lane) {
            if (lane == LEFT_LANE)
                return LEFT_LANE_D;
            else if (lane == MIDDLE_LANE)
                return MIDDLE_LAND_D;
            else
                return RIGHT_LANE_D;
        }

        LANE_TYPE get_lane_from_d(double d) {
            if (d < LANE_WIDTH) {
                return LANE_TYPE::LEFT_LANE;
            } else if ((d >= LANE_WIDTH) && (d < (2 * LANE_WIDTH))) {
                return LANE_TYPE::MIDDLE_LANE;
            } else {
                return LANE_TYPE::RIGHT_LANE;
            }
        }

    }
}

#endif //PATH_PLANNING_LANE_H
