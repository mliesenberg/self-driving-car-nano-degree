import json
from keras.models import Sequential, model_from_json
from keras.layers import Dense, Flatten, Lambda, Dropout
from keras.layers import Convolution2D, MaxPooling2D
from keras.optimizers import Adam

import preprocessing

model_json = '../../keras/model-6.json'
model_weights = '../../keras/model-6.h5'
number_of_epochs = 8
number_of_samples_per_epoch = 20032 # needs to be multiple of batch size, otherwise warning appears.
number_of_validation_samples = 6400
learning_rate = 1e-4
keep_prob = 0.5

input_shape = (64, 64, 3)

# define the model, see README.md for details.
model = Sequential()
model.add(Lambda(lambda x: x/127.5 - 1., input_shape=input_shape))
model.add(Convolution2D(24, 5, 5, subsample=(2,2), activation='relu', name='Conv1'))
model.add(Dropout(keep_prob))
model.add(Convolution2D(36, 5, 5, subsample=(2,2), activation='relu', name='Conv2'))
model.add(Dropout(keep_prob))
model.add(Convolution2D(48, 5, 5, subsample=(2,2), activation='relu', name='Conv3'))
model.add(Dropout(keep_prob))
model.add(Convolution2D(64, 3, 3, activation='relu', name='Conv4'))
model.add(Dropout(keep_prob))
model.add(Convolution2D(64, 3, 3, activation='relu', name='Conv5'))
model.add(Flatten())
model.add(Dense(1164, activation='relu', name='FC1'))
model.add(Dropout(0.4))
model.add(Dense(100, activation='relu', name='FC2'))
model.add(Dropout(0.3))
model.add(Dense(50, activation='relu', name='FC3'))
model.add(Dropout(0.2))
model.add(Dense(10, activation='relu', name='FC4'))
model.add(Dense(1, name='output'))
adam = Adam(lr=learning_rate)
model.compile(optimizer=adam, loss="mse")
model.summary()

# create generators for training and validation
train_gen = preprocessing.generate_next_batch()
validation_gen = preprocessing.generate_next_batch()

history = model.fit_generator(
                    train_gen,
                    samples_per_epoch=number_of_samples_per_epoch,
                    nb_epoch=number_of_epochs,
                    validation_data=validation_gen,
                    nb_val_samples=number_of_validation_samples,
                    verbose=1)

print("Saving model to disk: ", model_json, "and", model_weights)

json_string = model.to_json()
with open(model_json, 'w') as f:
   json.dump(json_string, f)

model.save_weights(model_weights)
