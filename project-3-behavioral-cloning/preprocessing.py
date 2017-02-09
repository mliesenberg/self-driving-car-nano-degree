import json
import cv2
import numpy as np
import pandas as pd
from keras.preprocessing.image import img_to_array
from scipy.stats import bernoulli


DRIVING_LOG_FILE = '../../Downloads/behavioral_cloning-master/data/driving_log.csv'
IMG_PATH = '../../Downloads/behavioral_cloning-master/data/'
STEERING_COEFFICIENT = 0.229


def random_flip(image, steering_angle, flipping_prob=0.5):
    """
        flip a coin to see if the image will be flipped. if so,
        steering_angle will be inverted
    """
    head = bernoulli.rvs(flipping_prob)
    if head:
        return np.fliplr(image), -1 * steering_angle
    else:
        return image, steering_angle

def random_shear(image, steering_angle, shear_range=200):
    """
        Source: https://medium.com/@ksakmann/behavioral-cloning-make-a-car-drive-like-yourself-dc6021152713#.7k8vfppvk
    """
    rows, cols, ch = image.shape
    dx = np.random.randint(-shear_range, shear_range + 1)
    random_point = [cols / 2 + dx, rows / 2]
    pts1 = np.float32([[0, rows], [cols, rows], [cols / 2, rows / 2]])
    pts2 = np.float32([[0, rows], [cols, rows], random_point])
    dsteering = dx / (rows / 2) * 360 / (2 * np.pi * 25.0) / 6.0
    M = cv2.getAffineTransform(pts1, pts2)
    image = cv2.warpAffine(image, M, (cols, rows), borderMode=1)
    steering_angle += dsteering

    return image, steering_angle


def apply_transformations(image, steering_angle, top_crop_percent=0.35, bottom_crop_percent=0.1,
                          resize_dim=(64, 64), do_shear_prob=0.9):

    head = bernoulli.rvs(do_shear_prob)
    if head == 1:
        image, steering_angle = random_shear(image, steering_angle)

    # constrain to region of interest in y direction
    top = int(np.ceil(image.shape[0] * top_crop_percent))
    bottom = image.shape[0] - int(np.ceil(image.shape[0] * bottom_crop_percent))
    image = image[top:bottom, :]

    image, steering_angle = random_flip(image, steering_angle)

    image = cv2.resize(image, resize_dim)

    return image, steering_angle


def get_next_image_files(batch_size=64):
    """
        randomly take one of the input images - left, right or center - for training.
    """
    log = pd.read_csv(DRIVING_LOG_FILE)
    indices = np.random.randint(0, len(log), batch_size)

    image_files_and_angles = []
    for index in indices:
        rnd_image = np.random.randint(0, 3)
        if rnd_image == 0:
            img = log.iloc[index]['left'].strip()
            angle = log.iloc[index]['steering'] + STEERING_COEFFICIENT
            image_files_and_angles.append((img, angle))

        elif rnd_image == 1:
            img = log.iloc[index]['center'].strip()
            angle = log.iloc[index]['steering']
            image_files_and_angles.append((img, angle))
        else:
            img = log.iloc[index]['right'].strip()
            angle = log.iloc[index]['steering'] - STEERING_COEFFICIENT
            image_files_and_angles.append((img, angle))

    return image_files_and_angles

def read_image(image_path):
    """
        read image from file and convert to expected color space.
    """
    image = cv2.imread(image_path)
    image = cv2.cvtColor(img_to_array(image), cv2.COLOR_BGR2RGB) # cv2.imread returns BGR not RGB
    return image

def generate_next_batch(batch_size=64):
    """
        generate a training batch as per the instructions.
    """
    while True:
        X_batch = []
        y_batch = []
        images = get_next_image_files(batch_size)
        for img_file, angle in images:
            raw_image = read_image(IMG_PATH + img_file)
            raw_angle = angle
            new_image, new_angle = apply_transformations(raw_image, raw_angle)
            X_batch.append(new_image)
            y_batch.append(new_angle)

        yield np.array(X_batch), np.array(y_batch)
