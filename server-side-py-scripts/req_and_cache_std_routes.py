import json
from api_handler import *

lines = {}

std_routes = get_std_routes()

ROUND_DEC = 4

for route in std_routes:
    line_dict = {}
    line_dict['route'] = route
    line_dict['max_seq'] = route['TrackCircuits'][-1]['SeqNum']
    lines[route['LineCode'] + '_' + str(route['TrackNum'])] = line_dict

circuit_arr = [None] * 9999
for k, v in lines.items():
    for circuit in v['route']['TrackCircuits']:
        cir_dict = {}
        cir_dict['seq_perc'] = round((circuit['SeqNum'] / v['max_seq']), ROUND_DEC)
        circuit_arr[circuit['CircuitId']] = cir_dict

f = open("circuit_cache", "w")

out= ""
for x in range(len(circuit_arr)):
    if circuit_arr[x] is not None:
        f.write(json.dumps(circuit_arr[x]))
    else:
        f.write("NONE")
    f.write("\n")

f.close()