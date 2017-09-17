## General approach

Using the data we get data from the simulator, the following steps are performed to generate/plan a path.
- we compute the jerk minimizing trajectory, first converting the coordinates to car coordinates
- for the generation of the JMT path we first pick 5 anchor points. 2 are coming from the previously reported path given by the simulator and 3 which we generate ourselves each 50m apart
- then we fit a spline to the above points and then upsample in order to have enough points for the path to steer the car along
- afterwards the generated path is fed to the simulator after converting the points back to global map coordinates
- based on proximity of the next cars the behavior planner takes an appropriate action - slowing down, overtaking or keeping the current speed

## Potential Improvements

- the implementation is rather simplistic and works fairly well, however there might be problems when other cars quickly move into our lane
- slowing down of other cars often leads to lane changes by them, that is not yet incorporated but could be a useful thing to include in future implementations
