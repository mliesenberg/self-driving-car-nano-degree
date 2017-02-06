**Vehicle Detection Project**

The goals / steps of this project are the following:

* Perform a Histogram of Oriented Gradients (HOG) feature extraction on a labeled training set of images and train a classifier Linear SVM classifier
* Optionally, you can also apply a color transform and append binned color features, as well as histograms of color, to your HOG feature vector.
* Note: for those first two steps don't forget to normalize your features and randomize a selection for training and testing.
* Implement a sliding-window technique and use your trained classifier to search for vehicles in images.
* Run your pipeline on a video stream and create a heat map of recurring detections frame by frame to reject outliers and follow detected vehicles.
* Estimate a bounding box for vehicles detected.
---

###Histogram of Oriented Gradients (HOG)

####1. Explain how (and identify where in your code) you extracted HOG features from the training images.

I then explored different color spaces and different `skimage.hog()` parameters (`orientations`, `pixels_per_cell`, and `cells_per_block`).  I grabbed random images from each of the two classes and displayed them to get a feel for what the `skimage.hog()` output looks like.

Here is an example using the `HLS` color space and HOG parameters of `orientations=10`, `pixels_per_cell=(16, 16)` and `cells_per_block=(2, 2)` on the L chanel which seemed to perform best (judging from several classifier training sessions):


![hog features][img/hog_features.png]

The code to generate the image can be found in cell 4 of the jupyter notebook. Based on
several classifier training sessions, I stuck with the above mentioned parameters as
that performed best in terms of training and test accuracy.


####2. Describe how (and identify where in your code) you trained a classifier using your selected HOG features (and color features if you used them).

Cell 5 contains the code to extract features and normalize them. Cell 6 contains
the code training the classifier. I tried both a LinearSVC and a SGDClassifier.

I experimented a bit with GridSearchCV to find better parameters than the standard set,
but decided to not use any special configuration apart from the class balancing.

Below the 6 test images with the windows that each classifier identified as car objects.
The LinearSVC has notably lower false positives.

![classified images][img/classifier.png]
---

### Video Implementation

####1. Provide a link to your final video output.  Your pipeline should perform reasonably well on the entire project video (somewhat wobbly or unstable bounding boxes are ok as long as you are identifying the vehicles most of the time with minimal false positives.)
Here's a [link to my video result](./project_video.mp4)


####2. Describe how (and identify where in your code) you implemented some kind of filter for false positives and some method for combining overlapping bounding boxes.

I recorded the positions of positive detections in each frame of the video.  From the positive detections I created a heatmap and then thresholded that map to identify vehicle positions.  I then used blob detection in Sci-kit Image (Determinant of a Hessian [`skimage.feature.blob_doh()`](http://scikit-image.org/docs/dev/auto_examples/plot_blob.html) worked best for me) to identify individual blobs in the heatmap and then determined the extent of each blob using [`skimage.morphology.watershed()`](http://scikit-image.org/docs/dev/auto_examples/plot_watershed.html). I then assumed each blob corresponded to a vehicle.  I constructed bounding boxes to cover the area of each blob detected.  

Here's an example result showing the heatmap and bounding boxes overlaid on a frame of video:

![alt text][image5]

---

###Discussion

####1. Briefly discuss any problems / issues you faced in your implementation of this project.  Where will your pipeline likely fail?  What could you do to make it more robust?
