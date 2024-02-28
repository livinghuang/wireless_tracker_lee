#!/usr/bin/env python3

import socket
import paho.mqtt.client as mqtt
import asyncio
from datetime import datetime, timedelta
from pymongo import MongoClient, results
import motor.motor_asyncio
import orjson
import aioredis
import logging
import math

import sys, time
import os 

DB_ADDRESS = 'mongo'
MONGO_ID = os.getenv('MONGO_ID')
MONGO_PW = os.getenv('MONGO_PW')

client_id = '/gw/#'
topic = client_id
#print("Using client_id / topic: " + client_id)

async def send_to_redis(data):
    # await asyncio.sleep(2)
    # data = {time_stamp:time_now.strftime("%Y-%m-%d %H:%M:%S"), bracelet_mac:data[0],device_data:data[1:]}
    redis = await aioredis.create_redis_pool('redis://redis')  # 新版本aioredis 的寫法
    for i in data:
        # if i.get('spo2'):
        mac = i['mac']
        # print('data:', data)
        try:
            i['rawData'] = None
            i['_id'] = None
            i['time_stamp'] = None
            i['timestamp'] = None
            i['type'] = None
            del i['rawData'] 
            del i['_id'] 
            del i['time_stamp'] 
            del i['timestamp'] 
            del i['type'] 
            logging.warning(f'send to redis i: {i}')
            await redis.hmset_dict(mac, **i)
            await redis.expire(mac, 28800)
        except:
            pass
    redis.close()
    await redis.wait_closed()

async def send_to_db(data):
    # data = {time_stamp:time_now.strftime("%Y-%m-%d %H:%M:%S"), bracelet_mac:data[0],device_data:data[1:]}
    # 這個方法要改成存入mongodb

    client = motor.motor_asyncio.AsyncIOMotorClient(f'mongodb://{MONGO_ID}:{MONGO_PW}@{DB_ADDRESS}', 27017)
    db = client['h05']
    collection = db['mqtt']
    # print('data:', data)
    try:
        del data['rawData']
    except:
        pass
    result = await collection.insert_many(data)
    options = {'async': True}
    client.fsync(**options)
    client.close()
    # print('result %s' % len(result.inserted_ids))


def h03_parse(string):
    string = string.split('107803E8')[1]
    data = bytes.fromhex(string)
    time_stamp = datetime.utcnow() + timedelta(hours=8)
    diastolic = int.from_bytes(data[0:1], byteorder='big')
    systolic = int.from_bytes(data[1:2], byteorder='big')
    steps = int.from_bytes(data[2:4], byteorder='little', signed=False)
    kms = round(steps * 0.66, 2)
    cals = int.from_bytes(data[6:8], byteorder='little', signed=False)
    battery = int.from_bytes(data[8:9], byteorder='big')
    heart_rate = int.from_bytes(data[9:10], byteorder='big')
    sleep_data = int.from_bytes(data[12:13], byteorder='big')
    temp = round(int(f'{string[20:22]}',16)+int(f'{string[22:24]}',16)/100,1)
    spo2 = 1

    # times =    int.from_bytes(data[17:19], byteorder='big')
    # diastolic = int.from_bytes(data[19:20], byteorder='big')
    # systolic = int.from_bytes(data[20:21], byteorder='big')
    device_data = locals()
    # print(f"yuntu_parse result: {steps}, {kms}, {cals}, {heart_beat}, {diastolic}, {systolic}")
    del device_data["data"]
    del device_data["string"]
    return device_data

def t05_parse(string):
    string = string.split('107803E8')[1]
    data = bytes.fromhex(string)
    time_stamp = datetime.utcnow() + timedelta(hours=8)
    diastolic = int.from_bytes(data[0:1], byteorder='big', signed=False)
    systolic = int.from_bytes(data[1:2], byteorder='big', signed=False)
    steps = int.from_bytes(data[2:4], byteorder='little', signed=False)
    heart_rate = int.from_bytes(data[4:5], byteorder='big')
    spo2 = int.from_bytes(data[5:6], byteorder='big')
    cals = int.from_bytes(data[6:8], byteorder='little', signed=False)
    battery = int.from_bytes(data[8:9], byteorder='big')
    temp = round(int(f'{string[20:22]}',16)+int(f'{string[22:24]}',16)/100,2)
    sleep_data = int.from_bytes(data[12:13], byteorder='big')
    kms = round(steps * 0.66, 2)
    # times =    int.from_bytes(data[17:19], byteorder='big')
    # diastolic = int.from_bytes(data[19:20], byteorder='big')
    # systolic = int.from_bytes(data[20:21], byteorder='big')
    device_data = locals()
    # print(f"yuntu_parse result: {steps}, {kms}, {cals}, {heart_beat}, {diastolic}, {systolic}")
    del device_data["data"]
    del device_data["string"]
    return device_data

def yt9_parse(string):
    string = string.split('543916FF')[1]
    data = bytes.fromhex(string)
    time_stamp = datetime.utcnow() + timedelta(hours=8)    
    steps = int.from_bytes(data[5:7], byteorder='big', signed=False)
    cals = int.from_bytes(data[7:9], byteorder='big', signed=False)
    heart_rate = int.from_bytes(data[9:10], byteorder='big')
    diastolic = int.from_bytes(data[10:11], byteorder='big', signed=False)
    systolic = int.from_bytes(data[11:12], byteorder='big', signed=False)
    battery = int.from_bytes(data[12:13], byteorder='big')
    #ysos = int.from_bytes(data[13:14], byteorder='big')
    #ykey = int.from_bytes(data[14:15], byteorder='big')   
    sleep_data = int.from_bytes(data[17:19], byteorder='big')
    temp =  round(int.from_bytes(data[15:17], byteorder='big') / 10,2)    
    spo2 = int.from_bytes(data[19:20], byteorder='big')            
    kms = round(steps * 0.66, 2)
    battery = int.from_bytes(data[12:13], byteorder='big')
    ysos = int.from_bytes(data[13:14], byteorder='big')
    ykey = int.from_bytes(data[14:15], byteorder='big')   
    device_data = locals()
    # print(f"yuntu_parse result: {steps}, {kms}, {cals}, {heart_beat}, {diastolic}, {systolic}")
    del device_data["data"]
    del device_data["string"]
    return device_data  

async def dejson(msg, topic):
    logging.warning(f'def dejson: {msg}')
    list_obj = orjson.loads(msg)
    j = []
    for i in list_obj:
        i['gateway_mac'] = topic
        i['distance'] = round(math.pow(10, (abs(i['rssi'])-49)/(10*3.5)),2)

        if i['bleName']:
            if i['bleName'][:3] == "YT9":
                if i['rawData'] [:4] == "07FF":
                    continue     
                # print('jciplus_parse', i['rawData'])
                else: 
                    i.update(yt9_parse(i['rawData']))
                    i['bleName'] = "YT9"
                    del i["timestamp"]
                    del i["type"]
                    del i['rawData']
                    j.append(i)

            if i['bleName'][:3] == "H05":
                if '107803E8' in i['rawData']:
                    # print('jciplus_parse', i['rawData'])
                    i.update(t05_parse(i['rawData']))
                    i['bleName'] = "H05"
                    del i["timestamp"]
                    del i['rawData']
                    del i["type"]

                    # print('i', i)

                    j.append(i)
                else:
                    j.append(i)
    return j


class AsyncioHelper:
    def __init__(self, loop, client):
        self.loop = loop
        self.client = client
        self.client.on_socket_open = self.on_socket_open
        self.client.on_socket_close = self.on_socket_close
        self.client.on_socket_register_write = self.on_socket_register_write
        self.client.on_socket_unregister_write = self.on_socket_unregister_write

    def on_socket_open(self, client, userdata, sock):
#        print("Socket opened")

        def cb():
            # print("Socket is readable, calling loop_read")
            client.loop_read()

        self.loop.add_reader(sock, cb)
        self.misc = self.loop.create_task(self.misc_loop())

    def on_socket_close(self, client, userdata, sock):
#        print("Socket closed")
        self.loop.remove_reader(sock)
        self.misc.cancel()

    def on_socket_register_write(self, client, userdata, sock):
#        print("Watching socket for writability.")

        def cb():
            # print("Socket is writable, calling loop_write")
            client.loop_write()

        self.loop.add_writer(sock, cb)

    def on_socket_unregister_write(self, client, userdata, sock):
#        print("Stop watching socket for writability.")
        self.loop.remove_writer(sock)

    async def misc_loop(self):
#        print("misc_loop started")
        while self.client.loop_misc() == mqtt.MQTT_ERR_SUCCESS:
            try:
                await asyncio.sleep(1)
            except asyncio.CancelledError:
                break
#        print("misc_loop finished")


class AsyncMqttExample:
    def __init__(self, loop):
        self.loop = loop

    def on_connect(self, client, userdata, flags, rc):
#        print("Subscribing")
        client.subscribe(topic)

    def on_message(self, client, userdata, msg):
        # print("Received message '" + str(msg.payload) + "' on topic '" + msg.topic + "' with QoS " + str(msg.qos))
        if not self.got_message:
            pass
#            print("Got unexpected message: {}".format(msg.decode()))
        else:
            try:
                self.got_message.set_result((msg.payload, msg.topic))
            except:
                pass

    def on_disconnect(self, client, userdata, rc):
        self.disconnected.set_result(rc)

    async def main(self):
        self.disconnected = self.loop.create_future()
        self.got_message = None

        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.on_disconnect = self.on_disconnect

        aioh = AsyncioHelper(self.loop, self.client)

        #self.client.connect('18.180.174.234', 1883, 60)
        self.client.connect('mosquitto', 1883, 60)
        self.client.socket().setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF,
                                        2048)
        while True:
            self.got_message = self.loop.create_future()
            msg = await self.got_message
            print('msg[1] ',msg[1])
            topic = msg[1].split('/')
            try:
                device_data_list = await dejson(msg[0], topic[2])
            except:
                continue
            # logging.warning('orjson ', device_data_list)
            # for i in device_data_list:
            #     print(i['mac'])
            if device_data_list != []:
                await send_to_db(device_data_list)
                await send_to_redis(device_data_list)
            self.got_message = None

        self.client.disconnect()
        print("Disconnected: {}".format(await self.disconnected))

# class MyDaemon(Daemon):
#     def run(self):
#         loop = asyncio.get_event_loop()
#         asyncio.ensure_future(AsyncMqttExample(loop).main())
#         loop.run_forever()


if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    asyncio.ensure_future(AsyncMqttExample(loop).main())
    loop.run_forever()

    # daemon = MyDaemon('/tmp/daemon-mqtt.pid')
    # if len(sys.argv) == 2:
    #     if 'start' == sys.argv[1]:
    #         print("Starting")
    #         daemon.start()
    #     elif 'stop' == sys.argv[1]:
    #         print("Stop")
    #         daemon.stop()
    #     elif 'restart' == sys.argv[1]:
    #         print("Restart")
    #         daemon.restart()
    #     else:
    #         print("Unknown command")
    #         sys.exit(2)
    #     sys.exit(0)
    # else:
    #     print("usage: %s start|stop|restart" % sys.argv[0])
    #     print("Finished")
    #     sys.exit(2)
