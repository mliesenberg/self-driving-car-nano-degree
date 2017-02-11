import cv2
import numpy as np

def add_heat(heatmap, bbox_list):
    '''Add "heat" to a map for a list of bounding boxes'''
    for box in bbox_list:
        # Add += 1 for all pixels inside each bbox
        # Assuming each "box" takes the form ((x1, y1), (x2, y2))
        heatmap[box[0][1]:box[1][1], box[0][0]:box[1][0]] += 1

    # Return updated heatmap
    return heatmap


def apply_threshold(heatmap, threshold):
    '''Zero out heatmap pixels below the threshold and return thresholded heatmap'''
    heatmap[heatmap <= threshold] = 0
    return heatmap


def draw_labeled_bboxes(image, labels):
    '''Draw rectangles around labeled regions'''
    # Iterate through all detected vehicles
    for vehicle_id in range(1, labels[1] + 1):
        # Find pixels with each vehicle_id label value
        nonzero = (labels[0] == vehicle_id).nonzero()

        # Identify x and y values of those pixels
        nonzeroy = np.array(nonzero[0])
        nonzerox = np.array(nonzero[1])

        # Define a bounding box based on min/max x and y
        bbox = ((np.min(nonzerox), np.min(nonzeroy)), (np.max(nonzerox), np.max(nonzeroy)))

        # Draw the box on the image
        cv2.rectangle(image, bbox[0], bbox[1], (0, 0, 255), 6)

    # Return the image
    return image
