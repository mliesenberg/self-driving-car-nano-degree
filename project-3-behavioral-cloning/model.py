import os
from pathlib import Path
import numpy as np
from numpy.random import random
import cv2
import pandas as pd
import json
from keras.models import Sequential, model_from_json
from keras.layers import Dense, Dropout, Activation, Flatten, Lambda, ELU
from keras.layers import Convolution2D, MaxPooling2D
from keras.utils import np_utils
from keras.optimizers import Adam
from sklearn.model_selection import train_test_split

## model input files for initial model save and retrain
training_data = '../keras/IMG'
training_data_log = '../keras/driving_log.csv'
model_json = '../keras/model-3.json'
model_weights = '../keras/model-3.h5'

## model parameters defined here
ch, img_rows, img_cols = 3, 66, 200  # relevant camera region
nb_classes = 1

# label the data for better access
data = pd.read_csv(training_data_log, names=['center', 'left', 'right', 'steering', 'throttle', 'break', 'speed'])
print(len(data), "read from: ", training_data_log)
print(data.describe())
print(data.head())

# build a model
separator = 'SEP'
# filter for non brake and only moving frames
data = data[(data['break'] == 0.0) & (data['speed'] > 0.0)]
# put all the images in the training data, to later use them in the generator
X_train = np.copy(data['center'] + separator + data['left'] + separator + data['right'])

# the element to be trained for
Y_train = np.copy(data['steering'])
Y_train = Y_train.astype(np.float32)

# split the data into training and validation
X_train, X_val, Y_train, Y_val = train_test_split(X_train, Y_train, test_size=0.15, random_state=42)

batch_size = 20
samples_per_epoch = len(X_train) / batch_size
val_size = int(samples_per_epoch / 10.0)
nb_epoch = 50

def load_and_perprocess_image(imagepath):
    imagepath = imagepath.replace(' ', '')
    image = cv2.imread(imagepath, 1)
    image = cv2.resize(image, (img_rows, img_cols), interpolation=cv2.INTER_AREA)
    image = cv2.cvtColor(image, cv2.COLOR_RGB2YUV)
    return image

# generator to produce the data for the model.
def batchgen(X, Y):
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
            image = load_and_perprocess_image(imagepath)
            y = np.array([[y]])
            image = image.reshape(1, img_rows, img_cols, ch)
            yield image, y

input_shape = (img_rows, img_cols, ch)

# define the model, see README.md for details.
pool_size = (2, 3)
model = Sequential()
model.add(MaxPooling2D(pool_size=pool_size, input_shape=input_shape))
model.add(Lambda(lambda x: x / 127.5 - 1.))
model.add(Convolution2D(5, 5, 24, subsample=(4, 4), border_mode="same"))
model.add(ELU())
model.add(Convolution2D(5, 5, 36, subsample=(2, 2), border_mode="same"))
model.add(ELU())
model.add(Convolution2D(5, 5, 48, subsample=(2, 2), border_mode="same"))
model.add(ELU())
model.add(Convolution2D(3, 3, 64, subsample=(2, 2), border_mode="same"))
model.add(ELU())
model.add(Convolution2D(3, 3, 64, subsample=(2, 2), border_mode="same"))
model.add(Flatten())
model.add(Dropout(.2))
model.add(ELU())
model.add(Dense(1164))
model.add(ELU())
model.add(Dense(100))
model.add(ELU())
model.add(Dense(50))
model.add(ELU())
model.add(Dense(10))
model.add(ELU())
model.add(Dense(1))
adam = Adam(lr=0.001, beta_1=0.9, beta_2=0.999, epsilon=1e-07, decay=0.0)
model.compile(optimizer=adam, loss="mse", metrics=['accuracy'])
model.summary()

print(val_size)
history = model.fit_generator(
                    batchgen(X_train, Y_train),
                    samples_per_epoch=samples_per_epoch,
                    nb_epoch=nb_epoch,
                    validation_data=batchgen(X_val, Y_val),
                    nb_val_samples=val_size,
                    verbose=1)

print("Saving model to disk: ", model_json, "and", model_weights)

json_string = model.to_json()
with open(model_json, 'w') as f:
   json.dump(json_string, f)

model.save_weights(model_weights)
