import cv2
import numpy as np

from sklearn.model_selection import train_test_split
from sklearn.model_selection import GridSearchCV
from sklearn.preprocessing import StandardScaler
from sklearn.svm import LinearSVC

from feature_extraction import spatial_binning_features, color_histogram_features, hog_features

# Define a function to extract features from a list of images
def extract_features(images, spatial_size=(32, 32), hist_bins=32, hist_range=(0, 256)):
    """

    """
    features = []
    for image in images:
        # resize for conformity across input
        if (image.shape != (64, 64, 3)):
            image = cv2.resize(image, (64, 64))
        # Apply spatial_binning_features() to get spatial color features
        spatial_features = spatial_binning_features(image, size=spatial_size)
        # Apply color_hist() also with a color space option now
        hist_features = color_histogram_features(image, nbins=hist_bins, bins_range=hist_range)
        # hog features
        hog = hog_features(image)

        # Append the new feature vector to the features list
        features.append(np.concatenate((spatial_features, hist_features, hog)))

    # Return list of feature vectors
    return features

def create_train_test_set(cars, not_cars, spatial=32, hist_bin_size=32):
    car_features = extract_features(cars, spatial_size=(spatial, spatial), hist_bins=hist_bin_size, hist_range=(0, 256))
    notcar_features = extract_features(not_cars, spatial_size=(spatial, spatial), hist_bins=hist_bin_size, hist_range=(0, 256))

    # Create an array stack of feature vectors
    X = np.vstack((car_features, notcar_features)).astype(np.float64)
    # Fit a per-column scaler
    X_scaler = StandardScaler().fit(X)
    # Apply the scaler to X
    scaled_X = X_scaler.transform(X)

    # Define the labels vector
    y = np.hstack((np.ones(len(car_features)), np.zeros(len(notcar_features))))

    # Split up data into randomized training and test sets
    rand_state = np.random.randint(0, 100)
    X_train, X_test, y_train, y_test = train_test_split(scaled_X, y, test_size=0.2, random_state=rand_state)

    return X_train, X_test, y_train, y_test, X_scaler
