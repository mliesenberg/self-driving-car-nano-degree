import os
from pathlib import Path
import numpy as np
from numpy.random import random
import cv2
import pandas as pd
import json
from keras.models import Sequential, model_from_json
from keras.layers import Dense, Flatten, Lambda
from keras.layers import Convolution2D, MaxPooling2D
from keras.preprocessing.image import load_img, img_to_array
from keras.utils import np_utils
from keras.optimizers import Adam
from sklearn.cross_validation import train_test_split

## model input files for initial model save and retrain
training_data = '../../keras/IMG'
training_data_log = '../../keras/driving_log.csv'
model_json = '../../keras/model-6.json'
model_weights = '../../keras/model-6.h5'

training_data_udacity = '../../Downloads/data/'
training_data_log_udacity = '../../Downloads/data/driving_log.csv'

## model parameters defined here
ch, img_rows, img_cols = 3, 66, 200  # relevant camera region
nb_classes = 1

def load_and_preprocess_image(image_path):
    image_path = image_path.replace(' ', '') # some file names have spaces in front
    image = cv2.imread(training_data_udacity + image_path)
    image = image[60:140,40:280] # restrict to region of interest
    image = cv2.resize(image, (img_rows, img_cols), interpolation=cv2.INTER_AREA)
    image = cv2.cvtColor(img_to_array(image), cv2.COLOR_BGR2YUV) # cv2.imread returns BGR not RGB
    return image

# generator to produce the data for the model.
def batch_generator_(X, Y):
    while True:
        for i in range(len(X)):
            y = Y[i]
            if y < -0.01:
                chance = random()
                if chance > 0.75:
                    imagepath = X[i].split(separator)[1]
                    y *= 3.0
                else:
                    if chance > 0.5:
                        imagepath = X[i].split(separator)[1]
                        y *= 2.0
                    else:
                        if chance > 0.25:
                           imagepath = X[i].split(separator)[0]
                           y *= 1.5
                        else:
                           imagepath = X[i].split(separator)[0]
            else:
                if y > 0.01:
                    chance = random()
                    if chance > 0.75:
                        imagepath = X[i].split(separator)[2]
                        y *= 3.0
                    else:
                        if chance > 0.5:
                            imagepath = X[i].split(separator)[2]
                            y *= 2.0
                        else:
                            if chance > 0.25:
                                imagepath = X[i].split(separator)[0]
                                y *= 1.5
                            else:
                                imagepath = X[i].split(separator)[0]
                else:
                    imagepath = X[i].split(separator)[0]
            image = load_and_preprocess_image(imagepath)
            y = np.array([[y]])
            image = image.reshape(1, img_rows, img_cols, ch)
            yield image, y

# label the data for better access
data = pd.read_csv(training_data_log_udacity)
data = data.sample(frac=1).reset_index(drop=True) # shuffle the data.

print(len(data), "read from: ", training_data_log_udacity)
print(data.describe())
print(data.head())

# build a model
separator = 'SEP'
# filter for non brake and only moving frames
data = data[(data['brake'] == 0.0) & (data['speed'] > 0.0)]
# put all the images in the training data, to later use them in the generator
X_train = np.copy(data['center'] + separator + data['left'] + separator + data['right'])

# the element to be trained for
Y_train = np.copy(data['steering'])
Y_train = Y_train.astype(np.float32)

# split the data into training and validation
X_train, X_val, Y_train, Y_val = train_test_split(X_train, Y_train, test_size=0.15, random_state=42)

batch_size = 32
samples_per_epoch = int(len(X_train) / batch_size)
val_size = int(samples_per_epoch / 10.0)
nb_epoch = 10

input_shape = (img_rows, img_cols, ch)

# define the model, see README.md for details.
pool_size = (2, 3)
model = Sequential()
#model.add(MaxPooling2D(pool_size=pool_size, input_shape=input_shape))
model.add(Lambda(lambda x: x/127.5 - 1., input_shape=input_shape))
model.add(Convolution2D(24, 5, 5, subsample=(2,2), activation='relu', name='Conv1'))
model.add(Convolution2D(36, 5, 5, subsample=(2,2), activation='relu', name='Conv2'))
model.add(Convolution2D(48, 5, 5, subsample=(2,2), activation='relu', name='Conv3'))
model.add(Convolution2D(64, 3, 3, activation='relu', name='Conv4'))
model.add(Convolution2D(64, 3, 3, activation='relu', name='Conv5'))
model.add(Flatten())
model.add(Dense(1164, activation='relu', name='FC1'))
model.add(Dense(100, activation='relu', name='FC2'))
model.add(Dense(50, activation='relu', name='FC3'))
model.add(Dense(10, activation='relu', name='FC4'))
model.add(Dense(1, name='output'))
adam = Adam(lr=0.0001)
model.compile(optimizer=adam, loss="mse", metrics=['accuracy'])
model.summary()

history = model.fit_generator(
                    batch_generator_(X_train, Y_train),
                    samples_per_epoch=4800,
                    nb_epoch=nb_epoch,
                    validation_data=batch_generator_(X_val, Y_val),
                    nb_val_samples=480,
                    verbose=1)

print("Saving model to disk: ", model_json, "and", model_weights)

json_string = model.to_json()
with open(model_json, 'w') as f:
   json.dump(json_string, f)

model.save_weights(model_weights)
