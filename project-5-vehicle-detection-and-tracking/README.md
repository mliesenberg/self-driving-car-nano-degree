#Vehicle Detection and Tracking Project

## Overview
- [source code](#source-code)
- [pipeline](#pipline)
- [outlook and discussion](#outlook-and-discussion)


## source code
The jupyter notebook uses a number of python files which contain the bulk of the work. Those are
- `feature_extraction.py` which defines the functions to extract features from the image data. The functions and their parameters will
be discussed in detail in the following selection
- `video_processing.py` contains the code analyzing each frame of the video
- `heatmap.py` contains the code to create and update a heatmap from the detections of the classifier as well as a function
to turn the heatmap into a bounding box to be drawn on the output frame
- `pipeline.py` defines methods to generate training and test data
- `window_detection.py` contains the code to generate and evaluate sliding windows used for detection


## pipeline
The pipeline of the project comprises of a number of steps, two of which I'd like to discuss further - feature extraction and vehicle detection.

### feature extraction
Feature extraction is defined in the file `pipeline.py` in the function `extract_features`. It extracts three types of features into one long one-dimensional feature vector for each training image. As mentioned above, the functions to compute the features are defined in the file `feature_extraction.py`
- **Histogram of Oriented Gradient (HOG)**: HOG features are extracted by the function `hog_features` and returned as a one-dimensional feature vector. Trying a number of possible parameters, I found that `orient=9, pix_per_cell=8, cell_per_block=2` applied on all channels of the `YCrCb` color space work well. Below an example of a car image and its corresponding HOG feature image.
![hog features](img/hog_features.png)
- **Spatial binning of color**: the function `spatial_binning_features` returns a scaled down version (16 by 16 pixels) of an image as a one-dimensional feature vector
- **Histograms of color**: the function `color_histogram_features` returns the histograms of all three color channels of the `YUV` color space as a single one-dimensional feature vector. The bin size is 16.


### vehicle detection
To detect vehicles in each frame, a two stage approach was taken:
1. defining and restricting the search space in the frame which is done by means of a sliding approach
2. train a classifier to decide if something is a vehicle or not
Below a more thorough explanation of both stages.

### sliding windows
With a sliding window approach an image is split into a number of smaller images, each overlapping the others by a factor. In this project four window sizes are used, going from `(60, 60)` to `(160, 160)` each restricted to an area of interest in the original image as shown below.
```
window_definition = [
    ((60, 60),  (380, 560)), # size and the region of interest in the y coordinates as tuples.
    ((80, 80),  (400, 640)),
    ((120, 120),(400, 660)),
    ((160, 160),(400, 720)),
]
```
The smaller windows are used for objects farther in the distance, whereas the bigger images will detect objects close to the source of the image. Due to the overlap, in between windows of the same size as well as those of different sizes the approach works well to find individual objects in an image without having to move pixel by pixel. The overlap factor chosen for this project is 0.5 and detection only starts beyond 300 on the x-axis, which leads to a total of 348 windows to check.

### classifier
Based on the extracted features, I trained a `LinearSVC` as provided by scikit learn. Performance (accuracy) on the test set was a bit higher than 98%. Before settling on the `LinearSVC` I tried a `GradientBoostingClassifier` and a `SGDClassifier` both of which took longer to train and fared not significantly better.
I experimented a bit with GridSearchCV to find better parameters than the standard set, but decided to not use any special configuration apart from the class balancing. Before training (and later predicting) the feature vector is scaled to zero mean and unit variance so that larger numerical values do not get a greater weight. This is done using the `StandardScaler` provided by scikit-learn.

Below the output of the sliding windows and the classifier applied to the six test images.
![classified images](img/classifier.png)
When applying this setup to the video, I used a heatmap to eliminate duplicates and extract bounding boxes for the detected vehicles. For a bounding box to be displayed across multiple frames the car has to appear in several consecutive frames as tracked by merging the heatmaps.


### the video
Here's a [link to my video result](./project_video_out_.mp4)


### outlook and discussion
Looking at the performance in the video, there are a number of things that could be improved
- The bounding boxes could be more stable and at times do not accurately track the whole car
- scikit-learn provides a number of methods to do feature selection which could help prune the feature space, either enabling the use of additional features without slowing things down or improving performance
- In the video cars are always coming from the right hand side moving into the picture, there's no indication of how the classifier would work e.g. with cars coming towards us or passing on the left hand side etc.
- using all channels of the colour images has a significant impact on performance of the video processing pipeline. I've seen that OpenCV provides hog feature extraction functions as well and it might be worth checking if those are faster.
-
