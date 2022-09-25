from threading import activeCount
from keras.layers import GlobalMaxPooling2D
import cv2
import os
import tensorflow as tf
import keras
import argparse
import sys
from tensorflow.keras.applications.resnet50 import ResNet50, preprocess_input, decode_predictions
import numpy as np
import pandas as pd

# Input Shape
img_width, img_height, _ = 224, 224, 3 
DATASET_PATH = os.path.join(os.getcwd(),"fashion-dataset/")

def img_path(img):
    return DATASET_PATH+"/images/"+img

def load_image(img, resized_fac = 0.1):
    img     = cv2.imread(img_path(img))
    w, h, _ = img.shape
    resized = cv2.resize(img, (int(h*resized_fac), int(w*resized_fac)), interpolation = cv2.INTER_AREA)
    return resized

def recommendation(df):
    from tensorflow.keras.models import load_model
    base_model=load_model('weights.h5')

    # Add Layer Embedding
    model = keras.Sequential([
        base_model,
        GlobalMaxPooling2D()
    ])

    # model.summary()
    
    return model

# Function that get movie recommendations based on the cosine similarity score of movie genres
def get_recommender(idx, indices, top_n):
    sim_idx    = indices[idx]
    sim_scores = list(enumerate(cosine_sim[sim_idx]))
    sim_scores = sorted(sim_scores, key=lambda x: x[1], reverse=True)
    
    new_score = []
    for i in sim_scores:
        if i[1] > top_n:
            new_score.append(i)
    
    idx_rec    = [i[0] for i in new_score]
    idx_sim    = [i[1] for i in new_score]
    
    return indices.iloc[idx_rec].index, idx_sim

def get_embedding(model, img_name):
    # Reshape
    img = tf.keras.utils.load_img(img_name, target_size=(img_width, img_height))

    # img to Array
    x   = tf.keras.utils.img_to_array(img)
    # Expand Dim (1, w, h)
    x   = np.expand_dims(x, axis=0)
    # Pre process Input
    x   = preprocess_input(x)
    return model.predict(x).reshape(-1)

def create_embeddings(image ):
    from tensorflow.keras.models import load_model
    base_model=load_model(os.path.join(os.getcwd(),'weights.h5'))

    # Add Layer Embedding
    model = keras.Sequential([
            base_model,
            GlobalMaxPooling2D()
            ])

    # model.summary()
    
    emb = get_embedding(model, image)
        
    return emb
    # ### recommendation
    

ap = argparse.ArgumentParser()
ap.add_argument('-i', '--testimage', help="Image Path")
ap.add_argument('-t','--topwear',action='store_true')
ap.add_argument('-b','--bottomwear', action='store_true')
args = vars(ap.parse_args())

# generation of a dictionary of (title, images)
df_topwear = pd.read_csv(os.path.join(os.getcwd(), 'data_clothes_topwear.csv'))
df_bottomwear = pd.read_csv(os.path.join(os.getcwd(), 'data_clothes_bottomwear.csv'))

# # https://scikit-learn.org/stable/modules/generated/sklearn.metrics.pairwise_distances.html
from sklearn.metrics.pairwise import pairwise_distances

df_embs_topwear = pd.read_csv(os.path.join(os.getcwd(), 'embeddings_topwear.csv'))
df_embs_bottomwear = pd.read_csv(os.path.join(os.getcwd(), 'embeddings_bottomwear.csv'))

emb = create_embeddings(image=args['testimage'])
emb = pd.DataFrame(emb.reshape(-1, len(emb)))
if args['topwear']:
    df_embs = df_embs_topwear
    df = df_topwear
elif args['bottomwear']:
    df_embs = df_embs_bottomwear
    df = df_bottomwear
else:
    print("wrong type .. use --topwear for shirt and --bottomwear for bottom")
    sys.exit()

n_images = len(df_embs.index)
emb = emb.loc[emb.index.repeat(n_images)].reset_index(drop=True)

df_embs = df_embs.set_index(df_embs.columns[0])

cosine_sim = 1 - pairwise_distances(X=emb, Y=df_embs, metric='cosine')
indices = pd.Series(range(len(df)), index=df.index)
idx_ref = 0
idx_rec, idx_sim = get_recommender(idx_ref, indices, top_n = 0.6)
    
output_data = {'Type': [], 'Season': [], 'Product Name': [], 'Usage': []}
for i, row in df.loc[idx_rec].iterrows():
    # print(row)
    output_data['Season'].append(row.season)
    output_data['Type'].append(row.articleType)
    output_data['Product Name'].append(row.productDisplayName.replace(str(row.baseColour),''))
    output_data['Usage'].append(row.usage)
# print(output_data)
final = {}
if output_data['Type']:
    final['Type'] = max(set(output_data['Type']), key=output_data['Type'].count)
    index = output_data['Type'].index(final['Type'])
    final['Product Name'] = output_data['Product Name'][index]
    final['Season'] = output_data['Season'][index]
    final['Usage'] = output_data['Usage'][index]
    print(final)
else:
    print(output_data)
