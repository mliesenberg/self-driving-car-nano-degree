# CarND-Controls-PID
Self-Driving Car Engineer Nanodegree Program

---

## Discussion

The code implements a PID controller to steer a car around Udacity's car simulator lake course. The main program uses two PID controllers, one to control the steering angle and one to control the throttle.

Note: the output of the second controller has been capped at 0.25 to prevent the car from getting too fast, in fact rendering it useless. I have yet to fine-tune the controllers to work together reliable and not drive the car off the road every once in a while.

I have modified the structure of the code a little bit, namely directly computing and returning the next value to use in the controller instead of just updating the error and acting on that. Moreover, to make the controller work for both steering and speed computations I have introduced two new parameters - `min_range` and `max_range`. This allows to control for possible values together with a sigmoid function to smooth the output values. In addition to that I removed the error variables, keeping track of the `cte` with `prior_cte` and `sum_cte` to replace `i_error` and `p_error`

The tuning has been done manually.

### Steering Controller

* P = 0.2 to return relatively quickly to the center of the road after deviations, a small positive coefficient for the proportional component turned out to be enough
* I = 0.0 no need to compensate for a constant estimation error, we direct measurements and the wheels are perfectly aligned.
* D = -1.65 again to prevent overshooting, this time a large negative coefficient proved to be most effectful

### Throttle Controller
In contrast to the steering controller which takes the `cte` as its input, the throttle controller takes the absolute value of current steering angle to compute how to accelerate or not.

* P = 0.135 to get the car moving, applying a small positive coefficient here.
* I = 0.0 again nothing to correct for with the integral component
* D = -0.75 this one is probably to blame for the car becoming too fast and starting to steer more wildly, but it has been rather difficult to get it to a sane value

## Dependencies

* cmake >= 3.5
 * All OSes: [click here for installation instructions](https://cmake.org/install/)
* make >= 4.1
  * Linux: make is installed by default on most Linux distros
  * Mac: [install Xcode command line tools to get make](https://developer.apple.com/xcode/features/)
  * Windows: [Click here for installation instructions](http://gnuwin32.sourceforge.net/packages/make.htm)
* gcc/g++ >= 5.4
  * Linux: gcc / g++ is installed by default on most Linux distros
  * Mac: same deal as make - [install Xcode command line tools]((https://developer.apple.com/xcode/features/)
  * Windows: recommend using [MinGW](http://www.mingw.org/)
* [uWebSockets](https://github.com/uWebSockets/uWebSockets) == 0.13, but the master branch will probably work just fine
  * Follow the instructions in the [uWebSockets README](https://github.com/uWebSockets/uWebSockets/blob/master/README.md) to get setup for your platform. You can download the zip of the appropriate version from the [releases page](https://github.com/uWebSockets/uWebSockets/releases). Here's a link to the [v0.13 zip](https://github.com/uWebSockets/uWebSockets/archive/v0.13.0.zip).
  * If you run OSX and have homebrew installed you can just run the ./install-mac.sh script to install this
* Simulator. You can download these from the [project intro page](https://github.com/udacity/CarND-PID-Control-Project/releases) in the classroom.

## Basic Build Instructions

1. Clone this repo.
2. Make a build directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./pid`.

## Editor Settings

We've purposefully kept editor configuration files out of this repo in order to
keep it as simple and environment agnostic as possible. However, we recommend
using the following settings:

* indent using spaces
* set tab width to 2 spaces (keeps the matrices in source code aligned)

## Code Style

Please (do your best to) stick to [Google's C++ style guide](https://google.github.io/styleguide/cppguide.html).

## Project Instructions and Rubric

Note: regardless of the changes you make, your project must be buildable using
cmake and make!

More information is only accessible by people who are already enrolled in Term 2
of CarND. If you are enrolled, see [the project page](https://classroom.udacity.com/nanodegrees/nd013/parts/40f38239-66b6-46ec-ae68-03afd8a601c8/modules/f1820894-8322-4bb3-81aa-b26b3c6dcbaf/lessons/e8235395-22dd-4b87-88e0-d108c5e5bbf4/concepts/6a4d8d42-6a04-4aa6-b284-1697c0fd6562)
for instructions and the project rubric.

## Hints!

* You don't have to follow this directory structure, but if you do, your work
  will span all of the .cpp files here. Keep an eye out for TODOs.

## Call for IDE Profiles Pull Requests

Help your fellow students!

We decided to create Makefiles with cmake to keep this project as platform
agnostic as possible. Similarly, we omitted IDE profiles in order to we ensure
that students don't feel pressured to use one IDE or another.

However! I'd love to help people get up and running with their IDEs of choice.
If you've created a profile for an IDE that you think other students would
appreciate, we'd love to have you add the requisite profile files and
instructions to ide_profiles/. For example if you wanted to add a VS Code
profile, you'd add:

* /ide_profiles/vscode/.vscode
* /ide_profiles/vscode/README.md

The README should explain what the profile does, how to take advantage of it,
and how to install it.

Frankly, I've never been involved in a project with multiple IDE profiles
before. I believe the best way to handle this would be to keep them out of the
repo root to avoid clutter. My expectation is that most profiles will include
instructions to copy files to a new location to get picked up by the IDE, but
that's just a guess.

One last note here: regardless of the IDE used, every submitted project must
still be compilable with cmake and make./
