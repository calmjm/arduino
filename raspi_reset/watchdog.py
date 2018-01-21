from time import sleep
import RPi.GPIO as GPIO

pin = 40
delay = 0.1

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(pin, GPIO.OUT, initial=GPIO.LOW)
GPIO.output(pin, GPIO.HIGH)
sleep(delay)
GPIO.output(pin, GPIO.LOW)
