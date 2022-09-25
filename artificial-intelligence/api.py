import argparse
import cv2

from fastapi import FastAPI, Request
from fastapi.responses import FileResponse
import uvicorn

from colour_detection.colour_detection import colorDetection
from hat_detection.hat_detection import hatDetection
from clothing_classification.clothing_classification import clothingClassification

import numpy as np

from PIL import Image
import io

ap = argparse.ArgumentParser()
ap.add_argument('-live', '--live', action='store_true')
args = vars(ap.parse_args())

print('Starting API server on port 8080')
app = FastAPI()

@app.post("/ingest/outfit")
async def ingest_clothing_image(request: Request):
	type = request.headers.get('clothing-type')
	print('type', type);
	data: bytes = await request.body()
	#print('data', data.hex());
	bytes_as_np_array = np.frombuffer(data, dtype=np.uint8)
	img = cv2.imdecode(bytes_as_np_array, cv2.IMREAD_ANYCOLOR)
	print('shape', img.shape)

	filename = 'latest.jpg';
	cv2.imwrite(filename, img);

	resized_image = cv2.resize(img, (224, 224))
	# mask sections?

	args['topwear'] = False;
	args['bottomwear'] = True;

	# hat detection
	hat = hatDetection(args, resized_image)
	# clothing detection
	clothing = clothingClassification(args, resized_image)
	# colour detection
	colours = colorDetection(args, img)

	print ('hat', hat)
	#print ('clothing', clothing)
	print ('colours', colours)
	return colours

@app.get("/latest")
async def latest(request: Request):
	return FileResponse("latest.jpg")

uvicorn.run(app, host="0.0.0.0", port=8080)
