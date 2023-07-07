import websocket, json
from secret_constants import ESP_IP
from get_train_seq_percs import *

ws = websocket.WebSocket()

ws.connect("ws://" + ESP_IP+ "/")

line_pos_dict = json_helper(get_curr_train_seq_percs())
print(line_pos_dict)

ws.send(json.dumps(line_pos_dict))
print("Sent!!!!!!!!!!!!!!")
result = ws.recv()
print(result)

ws.close()