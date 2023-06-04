from flask import Flask
from flask import request
from datetime import datetime
import file_edit
import logs as logging

app = Flask(__name__)

#receive a trigger that activates the alarm
@app.route("/")
def receive_update():
    '''
    Receive cause for trigger and change options to reflect it.
    '''

    trigger = request.args.get("trigger")
   
    alarm_status = trigger[0]
    alarm_cause = trigger[1]
    
    
    x = logging.log(alarm_status, alarm_cause)
   


    #if alarm is off
    if(alarm_status == "0"):
        print('deactivating alarm')
        file_edit.set_alarm(2)
    #else if alarm is on turn it on in options
    elif(alarm_status == "1"):
        print('activating alarm')
        file_edit.set_alarm(1);
    elif(alarm_status == "2"):
        print('Device triggered in low security mode')
    return trigger

@app.route("/flipsensor")
def flip_sensor():
    '''
    Activate the sensor depending on user input
    '''
    door = request.args.get("door")
    motion = request.args.get("motion")
    light = request.args.get("light")

    if(door and door.equals("true")):
        file_edit.flip_sensor("door")

    if(motion):
        file_edit.flip_sensor("motion")

    if(light):
        file_edit.flip_sensor("light")
    
    return door, motion, light



@app.route("/options")
def get_options():
    response = []
    response.append(file_edit.get_sensor_status("alarm"))
    response.append(file_edit.get_sensor_status("mode"))
    response.append(file_edit.get_sensor_status("door"))
    response.append(file_edit.get_sensor_status("motion"))
    response.append(file_edit.get_sensor_status("light"))

    #returns 5 digit number where each digit is representative of an option
    return ''.join(response)

'''
@app.route("/securitymode")
def change_security_mode():

    174.5435.3424.3434/flipsensor?door=true

'''
