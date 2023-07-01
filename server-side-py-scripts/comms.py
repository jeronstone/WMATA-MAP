import websocket, json
from secret_constants import ESP_IP
from get_train_seq_percs import *

ws = websocket.WebSocket()

ws.connect(ESP_IP)

line_pos_dict= get_curr_train_seq_percs()

ws.send(json.dumps(line_pos_dict))
result = ws.recv()
print(result)
 
ws.close()