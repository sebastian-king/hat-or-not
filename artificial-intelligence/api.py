import argparse
import cv2

from fastapi import FastAPI, Request
from fastapi.responses import FileResponse
import uvicorn

from colour_detection.colour_detection import colorDetection
import numpy as np

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

	filename = 'latest.jpg';
	cv2.imwrite(filename, img);

	# mask the image sections
	# hat detection
	# clothing detection
	# colour detection
	colours = colorDetection(args, img)

	return colorDetection(args, img)

@app.get("/latest")
async def latest(request: Request):
	return FileResponse("latest.jpg")

uvicorn.run(app, host="0.0.0.0", port=8080)
