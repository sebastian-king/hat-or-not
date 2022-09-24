import argparse
import pandas as pd
import cv2
import os
import matplotlib.pyplot as plt

def histogram_equalization(image):
    img_y_cr_cb = cv2.cvtColor(image, cv2.COLOR_BGR2YCrCb)
    y, cr, cb = cv2.split(img_y_cr_cb)

    # Applying equalize Hist operation on Y channel.
    y_eq = cv2.equalizeHist(y)

    img_y_cr_cb_eq = cv2.merge((y_eq, cr, cb))
    img_rgb_eq = cv2.cvtColor(img_y_cr_cb_eq, cv2.COLOR_YCR_CB2BGR)
    return img_rgb_eq
    
def getColorName(R,G,B):
    minimum = 10000
    for i in range(len(csv)):
        d = abs(R- int(csv.loc[i,"R"])) + abs(G- int(csv.loc[i,"G"]))+ abs(B- int(csv.loc[i,"B"]))
        if(d<=minimum):
            minimum = d
            cname = csv.loc[i,"color_name"]
            chex = csv.loc[i,"hex"]
            
    return cname, chex

ap = argparse.ArgumentParser()
ap.add_argument('-i', '--image', required=True, help="Image Path")
ap.add_argument('-live', '--live', action='store_true')

args = vars(ap.parse_args())
img_path = args['image']
#Reading image with opencv
img = cv2.imread(img_path)
if args['live']:
    img = histogram_equalization(img)
    
img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
plt.imshow(img)
plt.show()
#Reading csv file with pandas and giving names to each column
index=["color","color_name","hex","R","G","B"]
csv = pd.read_csv(os.path.join(os.getcwd(),'colors.csv'), names=index, header=None)

width = img.shape[1]
height = img.shape[0]
ypos = int(height/2)
xpos = int(width/2)
r,g,b = img[ypos,xpos]
color, hex = getColorName(r,g,b) #+ ' R='+ str(r) + ' G='+ str(g) + ' B='+ str(b)
print(color, hex)
    