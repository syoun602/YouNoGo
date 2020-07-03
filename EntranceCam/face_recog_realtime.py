import cv2
import numpy as np
import os
from PIL import Image
 
labels = ["boyoung", "one", "yong", "nayoung"]
 
face_cascade = cv2.CascadeClassifier('haarcascades/haarcascade_frontalface_default.xml')
recognizer = cv2.face.LBPHFaceRecognizer_create()
recognizer.read("face-trainner.yml")
 
cap = cv2.VideoCapture(0)
cap.set(3,320)
cap.set(4,240)

if cap.isOpened() == False :
    exit()

while True :
    ret, img = cap.read()
    gray  = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    faces = face_cascade.detectMultiScale(gray, scaleFactor=1.5, minNeighbors=5)

    for (x, y, w, h) in faces :
        roi_gray = gray[y:y+h, x:x+w]

        id_, conf = recognizer.predict(roi_gray) 
        print(id_, conf)
        
        if conf>=50:
            font = cv2.FONT_HERSHEY_SIMPLEX 
            name = labels[id_] 
            cv2.putText(img, name, (x,y), font, 1, (0,0,255), 2)
            cv2.rectangle(img,(x,y),(x+w,y+h),(0,255,0),2)
    
    cv2.imshow('Preview',img)
    if cv2.waitKey(10) >= 0: 
        break
 
cap.release()
cv2.destroyAllWindows()

