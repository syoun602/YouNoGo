import cv2
import numpy as np
import os
from PIL import Image
 
face_cascade = cv2.CascadeClassifier('haarcascades/haarcascade_frontalface_default.xml')
recognizer = cv2.face.LBPHFaceRecognizer_create()

Face_ID = -1 
pev_person_name = ""
y_ID = []
x_train = []

Face_Images = os.path.join(os.getcwd(), "Face_Images")
print (Face_Images)

for root, dirs, files in os.walk(Face_Images) :
    for file in files :
        if file.endswith("jpeg") or file.endswith("jpg") or file.endswith("png") :
            path = os.path.join(root, file)
            person_name = os.path.basename(root)
            print(path, person_name)
 
            if pev_person_name != person_name :
                Face_ID=Face_ID+1
                pev_person_name = person_name
            
            img = cv2.imread(path) 
            gray_image = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            faces = face_cascade.detectMultiScale(gray_image, scaleFactor=1.3, minNeighbors=5)

            print (Face_ID, faces)
            
            for (x,y,w,h) in faces:
                roi = gray_image[y:y+h, x:x+w] 
                x_train.append(roi)
                y_ID.append(Face_ID)
 
                recognizer.train(x_train, np.array(y_ID)) 
                recognizer.save("face-trainner.yml")
