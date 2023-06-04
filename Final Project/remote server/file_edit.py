#Changing the log file

def flip_sensor(sensor_name : str):
    '''
    Given the name of the sensor, activate it in the log file
    '''
    text_list = ''
    with open('options.txt') as f:
        file_text = f.read()
        index = file_text.find(sensor_name)
        #print(index)
        index = index + len(sensor_name) + 2

        #print(index)
        text_list = list(file_text)
        #print(text_list)
        text_list[index] = "0" if file_text[index]==("1") else "1"

        text_list = (''.join(text_list))

    with open('options.txt', 'w') as f:
        f.write(text_list)

def get_sensor_status(sensor_name: str):
    '''
    Get the status of a sensor given its name
    '''
    with open('options.txt') as f:
        file_text = f.read()
        index = file_text.find(sensor_name)
        index = index + len(sensor_name) + 2
        return file_text[index]




def set_alarm(mode: int):
    if(mode == 1):
        if(get_sensor_status('alarm') == "0"):
            flip_sensor('alarm')
    elif(mode == 2):
        if(get_sensor_status('alarm') == "1"):
            flip_sensor('alarm')

def select_security_mode(mode: int):
    '''
    1 for low security
    2 for high security
    '''
    text_list = ''
    with open('options.txt') as f:
        file_text = f.read()
        index = file_text.find("mode")
        index = index + len("mode") + 2
        text_list = list(file_text)
        text_list[index] = str(mode-1)

        text_list = ''.join(text_list)
        
    with open('options.txt', 'w') as f:
        f.write(text_list)
