from secret_constants import header

import json, requests

api_url = 'https://api.wmata.com'

def get_std_routes():
    std_routes = requests.get(api_url + "/TrainPositions/StandardRoutes?contentType=json", header)
    std_routes = std_routes.json()['StandardRoutes']
    return std_routes

def get_train_pos():
    train_pos = requests.get(api_url + "/TrainPositions/TrainPositions?contentType=json", header)
    train_pos = train_pos.json()['TrainPositions']
    return train_pos

def get_circuits():
    line_cir = requests.get(api_url + "/TrainPositions/TrackCircuits?contentType=json", header)
    line_cir = line_cir.json()['TrackCircuits']
    return line_cir
