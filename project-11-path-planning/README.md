## General approach

Using the data we get data from the simulator, the following steps are performed to generate/plan a path.
- we compute the jerk minimizing trajectory, first converting the coordinates to car coordinates
- for the generation of the JMT path we first pick 5 anchor points. 2 are coming from the previously reported path given by the simulator and 3 which we generate ourselves each 50m apart
- then we fit a spline to the above points and then upsample in order to have enough points for the path to steer the car along
- afterwards the generated path is fed to the simulator after converting the points back to global map coordinates
- based on proximity of the next cars the behavior planner takes an appropriate action - slowing down, overtaking or keeping the current speed

## Code model generation
- `BehaviorPlanner.cpp` contains the logic to compute if it is safe to change lanes and how to do it, taking into account the current speed, cars in other lanes - represented by the class `OtherCar` which tracks `s` and `d` as well as an identifier.
- `Constants.h` defines a set of common values used throughout the code
- `Lane.h` defines methods to get from `d` to a lane and the other way around
- in `main.cpp` lines 270 to 330 the sensor data is evaluated and the decision on what to do is made

## Potential Improvements

- the implementation is rather simplistic and works fairly well, however there might be problems when other cars quickly move into our lane
- slowing down of other cars often leads to lane changes by them, that is not yet incorporated but could be a useful thing to include in future implementations
- the structure of the code could be more modular, e.g. there's lot's of logic in the `main` method which should be moved to it's own module
