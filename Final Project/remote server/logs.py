from datetime import datetime

def log(alarm_status: int, alarm_cause: int):
    alarm_status_list = ['Alarm Deactivated', 'Alarm Activated', 'Low Security Trigger']
    alarm_cause_list_a = ['Door', 'Light', 'Motion', 'Remote']
    alarm_cause_list_d = ['Button Code', 'Remote']
    #print(type(alarm_status))
    #print(type(alarm_cause))
   
    now = datetime.now()
    current_time = now.strftime("%Y-%m-%d, %H:%M:%S")
    alarm_status = int(alarm_status)
    alarm_cause = int(alarm_cause)
    #print(alarm_status, alarm_cause)
    #print('testing log') 
    with open("logging.txt", 'a') as f:
        if(alarm_status == 0): #if alarm deactivated
            message = current_time + " " + alarm_status_list[alarm_status] + " " + alarm_cause_list_d[alarm_cause] + "\n"
            f.write(message)
            #print(message)
            return 'success'
        elif(alarm_status == 1 or alarm_status == 2):
            message = current_time + " " + alarm_status_list[alarm_status] + " " + alarm_cause_list_a[alarm_cause] + "\n"
            f.write(message)
            #print(message)
            return 'success'

    return 'failure'

