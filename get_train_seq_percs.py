import json
from api_handler import *


circuit_arr = []
with open('circuit_cache','r') as f:
   for cir in f:
        if cir != 'NONE\n':
            circuit_arr.append(json.loads(cir[:-1]))
        else:
            circuit_arr.append(None)

f.close()
    
def get_curr_train_seq_percs():
    train_pos = get_train_pos()
    trains_per_line = {}
    for train in train_pos:
        if (train['LineCode'] != None and circuit_arr[train['CircuitId']] != None):
            try:
                trains_per_line[train['LineCode']].append(circuit_arr[train['CircuitId']]['seq_perc'])
            except:
                trains_per_line[train['LineCode']] = []
                trains_per_line[train['LineCode']].append(circuit_arr[train['CircuitId']]['seq_perc'])
    
    return trains_per_line

