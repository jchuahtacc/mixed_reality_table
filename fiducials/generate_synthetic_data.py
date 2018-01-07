import cv2
from os import listdir
from os.path import isfile, join
import numpy as np
import math

fiducial_source_dir = "./fiducial_sources"
output_dir = "./synthetic_data"
source_fiducial_files = [f for f in listdir(fiducial_source_dir) if isfile(join(fiducial_source_dir, f))]

fiducials = { source_file[:-4] : cv2.imread(fiducial_source_dir + "/" + source_file, cv2.IMREAD_UNCHANGED) for source_file in source_fiducial_files }
fiducial_names = fiducials.keys()


rows, cols, depth = fiducials[fiducial_names[0]].shape

img1 = fiducials[fiducial_names[0]]

"""
From: https://stackoverflow.com/questions/22041699/rotate-an-image-without-cropping-in-opencv-in-c/33564950#33564950
"""
def rotate_image(mat, angle):
    height, width = mat.shape[:2]
    image_center = (width / 2, height / 2)

    rotation_mat = cv2.getRotationMatrix2D(image_center, angle, 1)

    radians = math.radians(angle)
    sin = math.sin(radians)
    cos = math.cos(radians)
    bound_w = int((height * abs(sin)) + (width * abs(cos)))
    bound_h = int((height * abs(cos)) + (width * abs(sin)))

    rotation_mat[0, 2] += ((bound_w / 2) - image_center[0])
    rotation_mat[1, 2] += ((bound_h / 2) - image_center[1])

    rotated_mat = cv2.warpAffine(mat, rotation_mat, (bound_w, bound_h))
    return rotated_mat

def _do_compose(base, overlay):
	source_mask = overlay[:, :, 3]
	base_mask = base[:, :, 3]
	base_mask = cv2.bitwise_not(source_mask)
	bg = cv2.bitwise_and(base, base, mask=base_mask)
	return cv2.add(bg, overlay)

def compose(base, overlay, row = 0, col = 0):
	base_width, base_height, _ = base.shape
	overlay_width, overlay_height, _ = overlay.shape
	overlay_row = 0
	overlay_col = 0
	if row < 0:
		overlay_row = row * -1
	if col < 0:
		overlay_col = col * -1
	overlay_height -= abs(row)
	overlay_width -= abs(col)

	height_diff = max(0, row) + overlay_height - base_height
	if height_diff > 0:
		overlay_height -= height_diff

	width_diff = max(0, col) + overlay_width - base_width
	if width_diff > 0:
		overlay_width -= width_diff

	overlay_roi = overlay[overlay_row:overlay_row + overlay_height, overlay_col:overlay_col + overlay_width]
	base_row = max(0, row)
	base_col = max(0, col)
	base_roi = base[base_row:base_row + overlay_height, base_col:base_col + overlay_width]
	composite = base
	composite[base_row:base_row + overlay_height, base_col:base_col + overlay_width] = _do_compose(base_roi, overlay_roi)
	return composite
			
		

background = np.zeros((rows, cols, depth), dtype=np.uint8)
background[:] = (255, 0, 0, 255)

rot = rotate_image(img1, 45)

cv2.namedWindow('image')

composite = compose(background, rot, 50, 50)
cv2.imshow('image', composite)
cv2.waitKey(0)
