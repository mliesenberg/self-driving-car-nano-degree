import cv2
import numpy as np
from skimage.feature import hog

# create features based on the histograms of the individual color channels.
def color_histogram_features(image, nbins=16, bins_range=(0, 256), color_space='RGB'):
    if color_space != 'RGB':
        image = cv2.cvtColor(image, cv2.RGB2HLS)
    # Compute the histogram of the RGB channels separately
    rhist = np.histogram(image[:,:,0], bins=nbins, range=bins_range)
    ghist = np.histogram(image[:,:,1], bins=nbins, range=bins_range)
    bhist = np.histogram(image[:,:,2], bins=nbins, range=bins_range)
    # Generating bin centers
    bin_edges = rhist[1]
    bin_centers = (bin_edges[1:]  + bin_edges[0:len(bin_edges)-1])/2
    # Concatenate the histograms into a single feature vector
    hist_features = np.concatenate((rhist[0], ghist[0], bhist[0]))
    # Return the individual histograms, bin_centers and feature vector
    return hist_features

# spatial binning as discussed in the lectures.
def spatial_binning_features(image, size=(16, 16)):
    features = cv2.resize(image, size).ravel()
    return features

# create HOG features of an image as discussd in class
def hog_features(image, orient=9, pix_per_cell=8, cell_per_block=2, visualize=False):
    image = cv2.cvtColor(image, cv2.COLOR_RGB2YCrCb)

    return hog(image[:,:,0], orientations=orient, pixels_per_cell=(pix_per_cell, pix_per_cell),
                    cells_per_block=(cell_per_block, cell_per_block), transform_sqrt=True,
                    visualise=visualize, feature_vector=True)
