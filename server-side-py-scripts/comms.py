import websocket, json
from time import sleep
from secret_constants import ESP_IP
from get_train_seq_percs import *

ws = websocket.WebSocket()

# todo keep track of prev pos, if same dont other sending anything 
def update_and_send():
    try:
        ws.connect("ws://" + ESP_IP+ "/") # connect each run to make sure ESP still up
        print("Connected!")
    except:
        print("Error Connecting")
        return

    line_pos_dict = json_helper(get_curr_train_seq_percs())
    print(line_pos_dict)

    ws.send(json.dumps(line_pos_dict))
    print("Sent!")
    result = ws.recv()
    print(result)

    ws.close()

while True:
    update_and_send()
    sleep(30)
