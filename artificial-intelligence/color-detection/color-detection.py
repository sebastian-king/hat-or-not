import argparse
import pandas as pd
import cv2
import os

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
    return cname

ap = argparse.ArgumentParser()
ap.add_argument('-i', '--image', required=True, help="Image Path")
args = vars(ap.parse_args())
img_path = args['image']
#Reading image with opencv
img = cv2.imread(img_path)
img = histogram_equalization(img)
#Reading csv file with pandas and giving names to each column
index=["color","color_name","hex","R","G","B"]
csv = pd.read_csv(os.path.join(os.getcwd(),'colors.csv'), names=index, header=None)

width = img.shape[1]
height = img.shape[0]
ypos = int(height/2)
xpos = int(width/2)
b,g,r = img[ypos,xpos]
text = getColorName(r,g,b) + ' Color'#+ ' R='+ str(r) + ' G='+ str(g) + ' B='+ str(b)
print(text)
    