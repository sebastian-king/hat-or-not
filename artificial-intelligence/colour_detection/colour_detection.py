import argparse
import pandas as pd
import cv2
import os
import matplotlib.pyplot as plt
import numpy as np

from fastapi import FastAPI, Request
from fastapi.responses import FileResponse
import uvicorn

def histogram_equalization(img):
    img_y_cr_cb = cv2.cvtColor(img, cv2.COLOR_BGR2YCrCb)
    y, cr, cb = cv2.split(img_y_cr_cb)

    # Applying equalize Hist operation on Y channel.
    y_eq = cv2.equalizeHist(y)

    img_y_cr_cb_eq = cv2.merge((y_eq, cr, cb))
    img_rgb_eq = cv2.cvtColor(img_y_cr_cb_eq, cv2.COLOR_YCR_CB2BGR)
    return img_rgb_eq

def getColorName(R,G,B,csv):
    minimum = 10000
    for i in range(len(csv)):
        d = abs(R- int(csv.loc[i,"R"])) + abs(G- int(csv.loc[i,"G"]))+ abs(B- int(csv.loc[i,"B"]))
        if(d<=minimum):
            minimum = d
            cname = csv.loc[i,"color_name"]
            chex = csv.loc[i,"hex"]

    return cname, chex

def colorDetection(args, img):
    if args['live']:
        img = histogram_equalization(img)

    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    plt.imshow(img)
    plt.show()
    #Reading csv file with pandas and giving names to each column
    index=["color","color_name","hex","R","G","B"]
    csv = pd.read_csv(os.path.join(os.path.dirname(os.path.abspath(__file__)),'colors.csv'), names=index, header=None)

    width = img.shape[1]
    height = img.shape[0]
    ypos = int(height/2)
    xpos = int(width/2)
    r,g,b = img[ypos,xpos]
    color, hex = getColorName(r,g,b,csv) #+ ' R='+ str(r) + ' G='+ str(g) + ' B='+ str(b)
    print("Text colour: ", color)
    return hex

if __name__ == "__main__":
	ap = argparse.ArgumentParser()
	ap.add_argument('-i', '--image', help="Image Path")
	ap.add_argument('-live', '--live', action='store_true')
	ap.add_argument('-serve', '--serve', action='store_true')
	args = vars(ap.parse_args())

	if args['image'] is not True and args['serve'] is not True:
		ap.error("requires one of --image or --serve.")

		img_path = args['image']

		if args['serve']:
			print('Starting API server on port 8080')
		app = FastAPI()

		@app.post("/ingest/outfit")
		async def ingest_clothing_image(request: Request):
			type = request.headers.get('clothing-type')
			print('type', type);
			data: bytes = await request.body()
			print('data', data.hex());
			bytes_as_np_array = np.frombuffer(data, dtype=np.uint8)
			img = cv2.imdecode(bytes_as_np_array, cv2.IMREAD_ANYCOLOR)

			filename = 'latest.jpg';
			cv2.imwrite(filename, img);

			return colorDetection(args, img)

		@app.get("/latest")
		async def latest(request: Request):
			return FileResponse("latest.jpg")

		uvicorn.run(app, host="0.0.0.0", port=8080)
	else:
		#Reading image with opencv
		img = cv2.imread(img_path)
		colorDetection(args, img);
