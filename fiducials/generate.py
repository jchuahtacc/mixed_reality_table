import cv2
import math
import numpy as np

scale = 50
fiducial_size = 7
fiducials = [ ]
min_hot = 3
data_offset = 2

bit_positions = [ (row + data_offset, col + data_offset) for row in range(fiducial_size - 2 - data_offset) for col in range(fiducial_size - 2 - data_offset) ]

bits = len(bit_positions)

def make_fiducial(num):
	bitmask = np.zeros((fiducial_size, fiducial_size), dtype=np.uint8)
	bitmask[1][1] = 255
	num_hot = 0
	for i in range(fiducial_size):
		bitmask[i][0] = 255
		bitmask[i][fiducial_size - 1] = 255
		bitmask[0][i] = 255
		bitmask[fiducial_size - 1][i] = 255
	for index in range(bits):
		bit_position = bit_positions[index]
		value = ((num >> index) & 1) * 255
		if value > 0:
			num_hot += 1
		bitmask[bit_position[0], bit_position[1]] = value
	if num_hot >= min_hot:
		return bitmask
	else:
		return None

def make_convolution(fiducial):
	convolution = np.zeros((fiducial_size - 2 - data_offset, fiducial_size - 2 - data_offset))
	for row in range(0, 3):
		for col in range(0, 3):
			value = 0
			for row_offset in range(-1, 1):
				for col_offset in range(-1, 1):
					value += fiducial[row + row_offset + data_offset][col + col_offset + data_offset]
			convolution[row][col] = value
	return convolution

def make_maxpool(fiducial):
	maxpool = np.zeros((fiducial_size - 2 - data_offset, fiducial_size - 2 - data_offset))
	for row in range(0, 3):
		for col in range(0, 3):
			value = 0
			for row_offset in range(-1, 1):
				for col_offset in range(-1, 1):
					if fiducial[row + row_offset + data_offset][col + col_offset + data_offset] > value:
						value = fiducial[row + row_offset + data_offset][col + col_offset + data_offset]
			maxpool[row][col] = value
	return maxpool

def rotationally_equal(a1, a2):
	r90 = np.rot90(a2)
	r180 = np.rot90(a2)
	r270 = np.rot90(a2)
	return np.array_equal(a1, a2) or np.array_equal(a1, r90) or np.array_equal(a1, r180) or np.array_equal(a1, r270)

def find_match(a1, ordinal):
	for fiducial_tuple in fiducials:
		if rotationally_equal(a1, fiducial_tuple[ordinal]):
			return True
	return False

for num in range( int(math.pow(2, bits)) ):
	if num % 100 == 0:
		print num, ": num unique fiducials", len(fiducials)
	fiducial = make_fiducial(num)
	if fiducial is not None:
		maxpool = make_maxpool(fiducial)
		convolution = make_convolution(fiducial)
		if not find_match(convolution, 1) and not find_match(maxpool, 2):
			fiducials.append((fiducial, convolution, maxpool, num))

print "Num unique fiducials:", len(fiducials)
cv2.namedWindow('image')

for (fiducial, convolution, maxpool, num) in fiducials:
	img = cv2.resize(fiducial, (0, 0), fx=scale, fy=scale, interpolation=cv2.INTER_NEAREST)
	half_scale = scale / 2
	subimg = img[half_scale:(fiducial_size - half_scale), half_scale:(fiducial_size - half_scale)]
	cv2.imwrite(str(num) + ".png", subimg)
	cv2.imshow('image', subimg)
	cv2.waitKey(1)

cv2.destroyAllWindows()
