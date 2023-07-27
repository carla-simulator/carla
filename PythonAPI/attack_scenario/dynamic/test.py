import cv2
import imutils
import numpy as np

x = np.ones((3,4))

for px in x:
    print(px)


# img_orig = cv2.imread("./attack.png",cv2.IMREAD_UNCHANGED)
# # img_orig = cv2.imread("./self-portrait.png",cv2.IMREAD_UNCHANGED)
# img = img_orig.copy()
# cv2.imshow("img", img)
# cv2.waitKey()

# # print(img.shape)
# # dim = (7,7)
# # # imutils.resize(img, width=128)
# # img = cv2.resize(img, dim, interpolation = cv2.INTER_AREA)
# # print(img.shape)
# img[0:20,0:20,:] = 0

# cv2.imshow("img", img)
# cv2.waitKey()