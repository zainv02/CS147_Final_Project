import file_edit
import logs as logging
def select_action():
    inp = None
    while(not inp in [1, 2, 3]):
        print('Enter number for action:')
        print('1 - Select security mode')
        print('2 - Enable/disable sensors')
        print('3 - Enable/disable alarm ringing')
        inp = input()
        inp = int(inp)
    return inp

def select_security_mode():
    inp = None 
    while(not inp in [1, 2]):
        print("Enter number for security mode:")
        print("1 - Low Security Mode")
        print("2 - High Security Mode")

        inp = int(input())
        file_edit.select_security_mode(inp)

def flip_sensors():

    inp = None
    while(inp != 4):
        door_status = file_edit.get_sensor_status("door")
        motion_status = file_edit.get_sensor_status("motion")
        light_status = file_edit.get_sensor_status("light")

        print("Enter number for enabling/disabling sensor")
        print("1 - Door [" + ("disabled" if door_status=="0" else "enabled") + "]")
        print("2 - Motion [" + ("disabled" if motion_status=="0" else "enabled") + "]")
        print("3 - Light [" + ("disabled" if light_status=="0" else "enabled") + "]")
        print("4 - quit")

        inp = input('Enter Input:')
        inp = int(inp)
        if(inp == 1):
            file_edit.flip_sensor("door")
        elif(inp == 2):
            file_edit.flip_sensor("motion")
        elif(inp == 3):
            file_edit.flip_sensor("light")

def flip_alarm():
    inp = None
    while(not inp in [1, 2]):
        print("Enter number for turning on/off alarm")
        print("1 - Turn on alarm")
        print("2 - Turn off alarm")

        inp = int(input())
        file_edit.set_alarm(inp)
        
        if(inp == 1):
            logging.log(1,3)
        elif(inp == 2):
            logging.log(0, 1)
    

if __name__ == '__main__':

    while(True):
        inp = select_action()

        if(inp == 1):
            select_security_mode()
        elif(inp == 2):
            flip_sensors()
        elif(inp == 3):
            flip_alarm()
        else:
            print('Invalid Input')
    


