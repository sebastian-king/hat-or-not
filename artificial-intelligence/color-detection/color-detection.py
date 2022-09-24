import argparse
import pandas as pd
import cv2
import os
import math 
import numpy as np

clicked = False
r = g= b = xpos = ypos = 0

def histogram_equalization(image):
    img_y_cr_cb = cv2.cvtColor(image, cv2.COLOR_BGR2YCrCb)
    y, cr, cb = cv2.split(img_y_cr_cb)

    # Applying equalize Hist operation on Y channel.
    y_eq = cv2.equalizeHist(y)

    img_y_cr_cb_eq = cv2.merge((y_eq, cr, cb))
    img_rgb_eq = cv2.cvtColor(img_y_cr_cb_eq, cv2.COLOR_YCR_CB2BGR)
    return img_rgb_eq
    
def draw_function(event, x,y,flags,param):
    if event == cv2.EVENT_LBUTTONDOWN:
        global b,g,r,xpos,ypos, clicked
        clicked = True
        xpos = x
        ypos = y
        b,g,r = img[y,x]
        b = int(b)
        g = int(g)
        r = int(r)
        
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


cv2.namedWindow('image')
cv2.setMouseCallback('image',draw_function)
while(1):   
    cv2.imshow("image",img)
    if (clicked):
        cv2.rectangle(img,(20,20), (img.shape[1]-20,60), (b,g,r), -1)
        #Creating text string to display ( Color name and RGB values )
    
        text = getColorName(r,g,b) + ' Color'#+ ' R='+ str(r) + ' G='+ str(g) + ' B='+ str(b)
        print(text)
        scale = math.ceil(min(img.shape[1], img.shape[0])*0.5) # 1 # this value can be from 0 to 1 (0,1] to change the size of the text relative to the image
        fontScale = min(img.shape[1],img.shape[0]) * 2e-3
        cv2.putText(img, text,(20,50),fontScale = fontScale,color = (255,255,255),thickness = 1,fontFace = cv2.FONT_HERSHEY_TRIPLEX)
  
        #For very light colours we will display text in black colour
        if(r+g+b>=600):
            scale = math.ceil(min(img.shape[1], img.shape[0])*0.2) # 1 # this value can be from 0 to 1 (0,1] to change the size of the text relative to the image
            fontScale = min(img.shape[1],img.shape[0])/(25/scale)
            cv2.putText(img, text,(10,50),fontScale = fontScale,color = (0,0,0),thickness = scale,fontFace = cv2.FONT_HERSHEY_TRIPLEX)
        clicked=False
        #Break the loop when user hits 'esc' key 

    if cv2.waitKey(20) & 0xFF ==27:
        break
cv2.imwrite("color.png",img)

cv2.destroyAllWindows()