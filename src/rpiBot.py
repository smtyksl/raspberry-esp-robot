import imutils
import cv2
import math
from time import sleep      # Import sleep from time
import RPi.GPIO as GPIO     # Import Standard GPIO Module

GPIO.setmode(GPIO.BOARD)      # Set GPIO mode to BCM
GPIO.setwarnings(False);

# PWM Frequency
pwmFreq = 100

# Setup Pins for motor controller
GPIO.setup(12, GPIO.OUT)    # PWMA
GPIO.setup(16, GPIO.OUT)    # AIN2
GPIO.setup(18, GPIO.OUT)    # AIN1
GPIO.setup(22, GPIO.OUT)    # STBY
GPIO.setup(15, GPIO.OUT)    # BIN1
GPIO.setup(13, GPIO.OUT)    # BIN2
GPIO.setup(11, GPIO.OUT)    # PWMB

pwma = GPIO.PWM(12, pwmFreq)    # pin 18 to PWM  
pwmb = GPIO.PWM(11, pwmFreq)    # pin 13 to PWM
pwma.start(100)
pwmb.start(100)
def forward(spd):
    print("ileri")
    runMotor(0, spd, 0)
    runMotor(1, spd, 0)
    

def reverse(spd):
    print("geri")
    runMotor(0, spd, 1)
    runMotor(1, spd, 1)
    

def turnLeft(spd):
    print("sol")
    runMotor(0, spd, 0)
    runMotor(1, spd, 1)
    

def turnRight(spd):
    print("sag")
    runMotor(0, spd, 1)
    runMotor(1, spd, 0)
    

def runMotor(motor, spd, direction):
    GPIO.output(22, GPIO.HIGH);
    in1 = GPIO.HIGH
    in2 = GPIO.LOW

    if(direction == 1):
        in1 = GPIO.LOW
        in2 = GPIO.HIGH

    if(motor == 0):
        GPIO.output(18, in1)
        GPIO.output(16, in2)
        pwma.ChangeDutyCycle(spd)
    elif(motor == 1):
        GPIO.output(15, in1)
        GPIO.output(13, in2)
        pwmb.ChangeDutyCycle(spd)


def motorStop():
    print("dur")
    GPIO.output(22, GPIO.LOW)
def decideToDirection(avg, distance):
    
    if distance < 40:
        motorStop()
        reverse(50)
    else:
        if 40 < distance and distance < 60:
            spd = 50
        else:
            spd = 100
        if 220 <= avg and avg <= 420:
                forward(spd)
        elif avg < 220:
            turnLeft(spd)   
        elif 420 < avg: 
            turnRight(spd)

DFOV_DEGREES = 54  # such as average laptop webcam horizontal field of view
KNOWN_ROI_MM = 240  # say, height of a human head
def calcDistanceByNightmare(detectedObject,image):
 
    # get image dimensions:
    image = image
    width = image.shape[1]
    height = image.shape[0]
    x,y,w,h = detectedObject
    focal_value = (height / 2) / math.tan(math.radians(DFOV_DEGREES / 2))
    dist = KNOWN_ROI_MM * focal_value / h
    dist_in = dist / 25.4
    dist_in = dist_in * 2.54
   
    cv2.putText(image, 'Distance:' + str(round(dist_in)) + ' cm',
                    (5, 300), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
    return dist_in



# My HSV Values
hsvLower = (77, 50, 132)
hsvUpper = (94, 93 ,218)


camera=cv2.VideoCapture(0)
while True:
        (grabbed, frame) = camera.read()
        
        frame = imutils.resize(frame, width=600)
        blurred = cv2.GaussianBlur(frame, (7, 7), 0)
        hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

        mask = cv2.inRange(hsv, hsvLower, hsvUpper)
        mask = cv2.erode(mask, None, iterations=2)
        mask = cv2.dilate(mask, None, iterations=2)


        cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,
                cv2.CHAIN_APPROX_SIMPLE)[-2]
        center = None
        text=""
        if len(cnts) > 0:
                bbox = cv2.boundingRect(mask)
                distance = calcDistanceByNightmare(bbox,frame)
                c = max(cnts, key=cv2.contourArea)
                ((x, y), radius) = cv2.minEnclosingCircle(c)
                M = cv2.moments(c)
                center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
                if radius > 10:
                        cv2.circle(frame, (int(x), int(y)), int(radius),
                                (0, 255, 255), 2)
                        cv2.circle(frame, center, 5, (0, 0, 255), -1)
                        print(center,radius,"\ndistance ==>> ",distance)
                        if radius > 250:
                                print("stop")
                        else:
                                if(center[0]<150):
                                        print("Left")
                                        text="Left"
                                elif(center[0]>450):
                                        print("Right")
                                        text="Right"
                                elif(radius<250):
                                        print("Front")
                                        text="Front"
                                else:
                                        print("Stop")
                        decideToDirection(center[0],distance)
                else:
                    motorStop()
                
        cv2.putText(frame,text,(50,100),cv2.FONT_HERSHEY_SIMPLEX,1.0,(255, 0, 0),2 )
        # cv2.imshow("Frame", frame)
        # key = cv2.waitKey(1) & 0xFF
        # if key == ord("q"):
        #         break

# camera.release()
# cv2.destroyAllWindows()
