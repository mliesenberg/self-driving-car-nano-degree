import numpy as np
from scipy.ndimage.measurements import label

from window_detection import search_windows
from heatmap import add_heat, apply_threshold, draw_labeled_bboxes


class VideoProcessor2():

    def __init__(self, classifier, scaler, windows, threshold_single=1, threshold_combined=8, smoothing=0.0):
        self.classifier = classifier
        self.scaler = scaler
        self.windows = windows
        self.heatmap = None
        self.threshold_single = threshold_single
        self.threshold_combined = threshold_combined
        self.smoothing = smoothing

    def process_single_image(self, image):
            # 1. extract all potential hits
            hot_windows = search_windows(image, self.windows, self.classifier, self.scaler)

            # 2. combine duplicate detections by creating a heatmap
            current_heatmap = np.zeros_like(image[:,:,0]).astype(np.float)
            current_heatmap = add_heat(current_heatmap, hot_windows)

            # 3. threshold the heatmap to remove false positives and duplicate detections
            current_heatmap_thresh = apply_threshold(current_heatmap, self.threshold_single)

            # 4. Determine the number of vehicles and their position by identifying the positions and regions in the heatmap
            if self.heatmap is None:
                # There is no previous frame heat map so just use blank images
                current_heatmap_combined = np.zeros_like(image[:, :, 0]).astype(np.float)
                current_heatmap_combined_thresh = current_heatmap_combined

                labels = label(current_heatmap_thresh)
                self.heatmap = current_heatmap_thresh
            else:
                # use a smoothing factor to combine the current and previous frame heat map
                current_heatmap_combined = self.heatmap * self.smoothing +\
                                           current_heatmap_thresh * (1 - self.smoothing)

                # apply a different threshold to the combined heatmap
                current_heatmap_combined_thresh = apply_threshold(current_heatmap_combined, self.threshold_combined)

                labels = label(current_heatmap_combined_thresh)
                self.heatmap = current_heatmap_combined_thresh

            # 5. draw the bounding boxes of the detected regions in the original image/frame
            window_hot = draw_labeled_bboxes(np.copy(image), labels)

            return window_hot
