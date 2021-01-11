# coding: utf-8
import json

from typing import Optional
from fastapi import FastAPI

# ---- Class -----
class DataHandler:
    def __init__(self):
        self.time = 0
        self.sendable_messages = {}
        self.recieved_messages = {}

    def get_sendable_messages(self, veh_id):
        if str(veh_id) not in self.sendable_messages.keys():
            return []
        else:
            target_messages = self.sendable_messages[str(veh_id)]
            self.sendable_messages[str(veh_id)] = []

            return target_messages

    def get_recieved_messages(self, veh_id):
        if str(veh_id) not in self.recieved_messages.keys():
            return []
        else:
            target_messages = self.recieved_messages[str(veh_id)]
            self.recieved_messages[str(veh_id)] = []

            return target_messages

    def save_sendable_messages(self, veh_id, message):
        if str(veh_id) not in self.sendable_messages.keys():
            self.sendable_messages[str(veh_id)] = []
        self.sendable_messages[str(veh_id)].append(message)

    def save_recieved_messages(self, veh_id, message):
        if str(veh_id) not in self.recieved_messages.keys():
            self.recieved_messages[str(veh_id)] = []
        self.recieved_messages[str(veh_id)].append(message)

# ----- vars -----
app = FastAPI()
dh = DataHandler()

# ----- get -----
@app.get("/carla_time")
def get_carla_time():
    global dh
    return json.dumps(dh.time)

@app.get("/sendable_messages/{veh_id}")
def get_sendable_messages(veh_id):
    global dh
    return json.dumps(dh.get_sendable_messages(veh_id))

@app.get("/recieved_messages/{veh_id}")
def get_recieved_messages(veh_id):
    global dh
    return json.dumps(dh.get_recieved_messages(veh_id))

# ----- post -----
@app.post("/carla_time/{save_time}")
def save_carla_time(save_time):
    global dh
    dh.time = save_time

@app.post("/sendable_messages/{veh_id}/{message}")
def save_sendable_messages(veh_id, message):
    global dh
    dh.save_sendable_messages(veh_id, message)

@app.post("/recieved_messages/{veh_id}/{message}")
def save_recieved_messages(veh_id, message):
    global dh
    dh.save_recieved_messages(veh_id, message)
