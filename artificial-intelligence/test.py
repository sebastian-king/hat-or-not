#!/bin/python

import cv2

img = cv2.imread('latest.jpg')
img2 = letter = img[100:500,100:200]
cv2.imwrite('masked.jpg', img2);
