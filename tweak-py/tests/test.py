#!/usr/bin/env python3

#
# Test for Python3 binding
#
# Copyright 2022 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

import numpy as np

from tweak2 import server, Server, Client
from time import sleep
from threading import Condition, Lock

CallbacksCallCount = 0

class KeyTracker:
    def __init__(self):
        self.callbacks = set()
        self.lock = Lock()
        self.cond = Condition(self.lock)

    def markDirty(self, key):
        with self.lock:
            self.callbacks.discard(key)

    def markClean(self, key):
        with self.lock:
            self.callbacks.add(key)
            self.cond.notify_all()

    def waitClean(self, key):
        with self.lock:
            self.cond.wait_for(lambda: key in self.callbacks, timeout=5)

serverTracker = KeyTracker()

def serverTrackerMarkClean(item, arg):
    global serverTracker
    serverTracker.markClean(item)

def printValueBool(item, arg):
    global CallbacksCallCount
    serverTrackerMarkClean(item, arg)
    CallbacksCallCount += 1
    print("/a/testBool1 = {}".format(arg))

def printValueInt(item, arg):
    global CallbacksCallCount
    serverTrackerMarkClean(item, arg)
    CallbacksCallCount += 1
    print("/a/testInt1 = {}".format(arg))

def printValueFloat(item, arg):
    global CallbacksCallCount
    serverTrackerMarkClean(item, arg)
    CallbacksCallCount += 1
    print("/a/testFloat1 = {}".format(arg))

clientTracker = KeyTracker()

exceptionThrown = False

def exceptionThrower(id, value):
    global exceptionThrown
    global clientTracker
    print("Throwing expection")
    exceptionThrown = True
    clientTracker.markClean(id)
    raise ValueError("Test exception, shouldn't crash thread\n Invalid value: {}".format(value))

client = None
server.initialize_library(context_type="nng", params="role=server", uri="tcp://0.0.0.0:7777/")

num_iterations = 10000
def checkServerValue(func, tweak_id, value):
    global num_iterations
    for i in range(num_iterations):
        if func(tweak_id) == value:
            break
        sleep(0.01)
    assert(func(tweak_id) == value)

def checkArraysEqual(a, b):
    if len(a) != len(b):
        return False
    for ix, val in enumerate(a):
        if (b[ix] != val):
            return False
    return True

def checkClientValue(client, tweak_id, value):
    for i in range(num_iterations):
        if client[tweak_id] == value:
            break
        sleep(0.01)
    assert(client[tweak_id] == value)

callbackThreadAlive = False
def callbackThreadChecker(id, value):
    global callbackThreadAlive
    print("Callback thread is alive after exception being thrown")
    callbackThreadAlive = True
    clientTracker.markClean(id)

def test(clientFactory, server):
    global exceptionThrown
    global clientTracker
    global serverTracker
    global callbackThreadAlive
    global CallbacksCallCount

    CallbacksCallCount = 0

    itemBool1 = server.add_bool(
        "/a/testBool1", initial_value=False, meta='{}', callback=printValueBool)
    itemBool2 = server.add_bool(
        "/a/testBool2", initial_value=False, callback=serverTrackerMarkClean)
    itemBool3 = server.add_bool(
        "/a/testBool3", initial_value=False, meta='{}', callback=serverTrackerMarkClean)
    itemBool4 = server.add_bool(
        "/a/testBool4", initial_value=False, meta='{}',
        description='desc', callback=serverTrackerMarkClean)
    itemInt1 = server.add_int(
        "/a/testInt1", initial_value=1, meta='{}', callback=printValueInt)
    itemInt2 = server.add_int(
        "/a/testInt2", initial_value=1, callback=serverTrackerMarkClean)
    itemFloat1 = server.add_float(
        "/a/testFloat1", initial_value=1., meta='{}', callback=printValueFloat)
    itemFloat2 = server.add_float(
        "/a/testFloat2", initial_value=1., callback=serverTrackerMarkClean)

    foo = server.add("/a/testMatrix12", initial_value=1,
        callback=serverTrackerMarkClean)

    bufferB = np.array([[1, 2, 3], [4, 5, 6]], dtype="int8", order="F")
    itemByteMatrix = server.add("/a/testMatrix1", initial_value=bufferB,
        meta='{\"layout\":{\"dimensions\": [2, 3]}}',
        callback=serverTrackerMarkClean)

    bufferI = np.array([[1, 2], [3, 4]], dtype="float")
    itemDoubleMatrix = server.add("/a/testMatrix2", initial_value=bufferI,
        meta='{\"layout\":{\"dimensions\": [2, 2]}}',
        callback=serverTrackerMarkClean)

    bufferA = np.array([[1, 2], [3, 4], [5, 6]], dtype=np.int64, order="F")
    itemLMatrix = server.add("/a/testMatrix3", initial_value=bufferA,
        meta='{\"layout\":{\"dimensions\": [3, 2]}}',
        callback=serverTrackerMarkClean)

    itemString = server.add("/a/testString", initial_value="test string",
        meta='{}', callback=serverTrackerMarkClean)

    # TODO Exercise all argument combinations

    # Check that items were created successfully
    itemBool_find1 = server.find("/a/testBool1")
    itemBool_find2 = server.find("/a/testBool2")
    itemBool_find3 = server.find("/a/testBool3")
    itemBool_find4 = server.find("/a/testBool4")
    itemInt_find1 = server.find("/a/testInt1")
    itemInt_find2 = server.find("/a/testInt2")
    itemFloat_find1 = server.find("/a/testFloat1")
    itemFloat_find2 = server.find("/a/testFloat2")
    itemByteMatrix_find1 = server.find("/a/testMatrix1")
    itemDoubleMatrix_find2 = server.find("/a/testMatrix2")
    itemLMatrix_find2 = server.find("/a/testMatrix3")

    assert(itemBool1 == itemBool_find1)
    assert(itemBool2 == itemBool_find2)
    assert(itemBool3 == itemBool_find3)
    assert(itemBool4 == itemBool_find4)
    assert(itemInt1 == itemInt_find1)
    assert(itemInt2 == itemInt_find2)
    assert(itemFloat1 == itemFloat_find1)
    assert(itemFloat2 == itemFloat_find2)
    assert(itemByteMatrix == itemByteMatrix_find1)
    assert(itemDoubleMatrix == itemDoubleMatrix_find2)
    assert(itemLMatrix == itemLMatrix_find2)

    assert(server.get_bool(itemBool1) == False)
    assert(server.get_int(itemInt1) == 1)
    assert(server.get_float(itemFloat1) == 1.)
    assert(server.get_bool("/a/testBool1") == False)
    assert(server.get_int("/a/testInt1") == 1)
    assert(server.get_float("/a/testFloat1") == 1.)
    server.set_bool(itemBool1, True)
    server.set_int(itemInt1, 2)
    server.set_float(itemFloat1, 2.)

    bufferB1 = np.array(server.get("/a/testMatrix1"), copy=False)
    print(bufferB)
    print(bufferB1)

    assert(bufferB[0, 0] == bufferB1[0, 0])
    assert(bufferB[0, 1] == bufferB1[0, 1])
    assert(bufferB[0, 2] == bufferB1[0, 2])
    assert(bufferB[1, 0] == bufferB1[1, 0])
    assert(bufferB[1, 1] == bufferB1[1, 1])
    assert(bufferB[1, 2] == bufferB1[1, 2])

    bufferI1 = np.array(server.get("/a/testMatrix2"), copy=False)
    assert(bufferI[0, 0] == bufferI1[0, 0])
    assert(bufferI[0, 1] == bufferI1[0, 1])
    assert(bufferI[1, 0] == bufferI1[1, 0])
    assert(bufferI[1, 1] == bufferI1[1, 1])

    bufferA1 = np.array(server.get("/a/testMatrix3"), copy=False)
    assert(bufferA[0, 0] == bufferA1[0, 0])
    assert(bufferA[0, 1] == bufferA1[0, 1])
    assert(bufferA[1, 0] == bufferA1[1, 0])
    assert(bufferA[1, 1] == bufferA1[1, 1])
    assert(bufferA[2, 0] == bufferA1[2, 0])
    assert(bufferA[2, 1] == bufferA1[2, 1])

    bufferA1 = np.array(server.get("/a/testMatrix3"), copy=False)
    print(bufferA)
    print(bufferA1)

    assert(server.get_bool("/a/testBool1") == True)
    assert(server.get_int("/a/testInt1") == 2)
    assert(server.get_float("/a/testFloat1") == 2.)

    server.set_bool("/a/testBool1", False)
    server.set_int("/a/testInt1", 3)
    server.set_float("/a/testFloat1", 3.)

    assert(server.get_bool(itemBool1) == False)
    assert(server.get_int(itemInt1) == 3)
    assert(server.get_float(itemFloat1) == 3.)

    server.remove(itemFloat1)
    wasException = False
    try:
        server.get_float("/a/testFloat1")
    except IndexError:
        wasException = True
    assert(wasException)

    assert(CallbacksCallCount == 0)

    client = clientFactory()
    [ client_testBool1, client_testBool2, client_testBool3, client_testBool4, client_testInt1, client_testInt2, client_testFloat2 ] = \
        client.collect( [ "/a/testBool1", "/a/testBool2", "/a/testBool3", "/a/testBool4", "/a/testInt1", "/a/testInt2", "/a/testFloat2"] )

    assert(itemBool1 == client_testBool1)
    assert(itemBool2 == client_testBool2)
    assert(itemBool3 == client_testBool3)
    assert(itemBool4 == client_testBool4)
    assert(itemInt1 == client_testInt1)
    assert(itemInt2 == client_testInt2)
    assert(itemFloat2 == client_testFloat2)

    itemUris = dict()
    for item in client.list(lambda uri: not uri.endswith("vector")): # Vectors aren't supported yet.
        _, uri, _, _, _, _ = item                                    # Presense of them shall cause typecast exception and
        itemUris[uri] = True                                         # eternal sleep because server.finalize_library()
                                                                     # isn't called and thread pools aren't destroyed.
                                                                     # So let's filter them by simple predicate.

    assert("/a/testBool1" in itemUris)
    assert("/a/testBool2" in itemUris)
    assert("/a/testBool3" in itemUris)
    assert("/a/testBool4" in itemUris)
    assert("/a/testInt1" in itemUris)
    assert("/a/testInt2" in itemUris)
    assert("/a/testFloat2" in itemUris)

    clientB1 = np.array(client["/a/testMatrix1"], copy=False)
    assert(bufferB[0, 0] == clientB1[0, 0])
    assert(bufferB[0, 1] == clientB1[0, 1])
    assert(bufferB[0, 2] == clientB1[0, 2])
    assert(bufferB[1, 0] == clientB1[1, 0])
    assert(bufferB[1, 1] == clientB1[1, 1])
    assert(bufferB[1, 2] == clientB1[1, 2])

    serverTracker.markDirty(itemByteMatrix)
    clientB1[0, 0] = 21
    clientB1[0, 1] = 22
    clientB1[0, 2] = 23
    clientB1[1, 0] = 24
    clientB1[1, 1] = 25
    clientB1[1, 2] = 26
    client["/a/testMatrix1"] = clientB1
    serverTracker.waitClean(itemByteMatrix)

    bufferB1 = np.array(server.get("/a/testMatrix1"), copy=False)
    assert(clientB1[0, 0] == bufferB1[0, 0])
    assert(clientB1[0, 1] == bufferB1[0, 1])
    assert(clientB1[0, 2] == bufferB1[0, 2])
    assert(clientB1[1, 0] == bufferB1[1, 0])
    assert(clientB1[1, 1] == bufferB1[1, 1])
    assert(clientB1[1, 2] == bufferB1[1, 2])

    clientI1 = np.array(client["/a/testMatrix2"], copy=False)
    assert(bufferI[0, 0] == clientI1[0, 0])
    assert(bufferI[0, 1] == clientI1[0, 1])
    assert(bufferI[1, 0] == clientI1[1, 0])
    assert(bufferI[1, 1] == clientI1[1, 1])

    clientA1 = np.array(client["/a/testMatrix3"], copy=False)
    assert(bufferA[0, 0] == clientA1[0, 0])
    assert(bufferA[0, 1] == clientA1[0, 1])
    assert(bufferA[1, 0] == clientA1[1, 0])
    assert(bufferA[1, 1] == clientA1[1, 1])
    assert(bufferA[2, 0] == clientA1[2, 0])
    assert(bufferA[2, 1] == clientA1[2, 1])

    assert(client["/a/testString"] == "test string")
    print(client["/a/testString"])

    server.get_bool(itemBool1)
    client[client_testBool1] = True
    checkServerValue(server.get_bool, itemBool1, True)

    server.set_int(itemInt1, 0)
    client[client_testInt1] = 42
    checkServerValue(server.get_int, itemInt1, 42)

    server.set_float(itemFloat2, 0)
    client[client_testFloat2] = 42.24
    checkServerValue(server.get_float, itemFloat2, 42.24)

    client.set_item_callback(client_testBool1, lambda item, _: clientTracker.markClean(item))
    client.set_item_callback(client_testInt1, lambda item, _: clientTracker.markClean(item))
    client.set_item_callback(client_testFloat2, lambda item, _: clientTracker.markClean(item))

    server.set_bool(itemBool1, False)
    checkClientValue(client, client_testBool1, False)
    print('client[client_testBool1] == False: SUCCESS')

    server.set_bool(itemBool1, True)
    checkClientValue(client, client_testBool1, True)
    print('client[client_testBool1] == True: SUCCESS')

    server.set_int(itemInt1, 13)
    checkClientValue(client, client_testInt1, 13)
    print('client[client_testInt1] == 13: SUCCESS')

    server.set_int(itemInt1, 42)
    checkClientValue(client, client_testInt1, 42)
    print('client[client_testInt1] == 42: SUCCESS')

    server.set_float(itemFloat2, 13.31)
    checkClientValue(client, client_testFloat2, 13.31)
    print('client[client_testFloat2] == 13.31: SUCCESS')

    clientTracker.markDirty(client_testBool1)
    server.set_bool(itemBool1, False)
    clientTracker.waitClean(client_testBool1)
    assert(client[client_testBool1] == False)
    print('via callback client[client_testFloat2] == 13.31: SUCCESS')

    clientTracker.markDirty(client_testBool1)
    server.set_bool(itemBool1, True)
    clientTracker.waitClean(client_testBool1)
    assert(client[client_testBool1] == True)
    print('via callback client[client_testBool1] == True: SUCCESS')

    clientTracker.markDirty(client_testInt1)
    server.set_int(itemInt1, 13)
    clientTracker.waitClean(client_testInt1)
    assert(client[client_testInt1] == 13)
    print('via callback client[client_testInt1] == 13: SUCCESS')

    clientTracker.markDirty(client_testInt1)
    server.set_int(itemInt1, 42)
    clientTracker.waitClean(client_testInt1)
    assert(client[client_testInt1] == 42)
    print('via callback client[client_testInt1] == 42: SUCCESS')

    print("set thrower")
    clientTracker.markDirty(client_testFloat2)
    client.set_item_callback(client_testFloat2, exceptionThrower)
    server.set_float(itemFloat2, 31.13)
    clientTracker.waitClean(client_testFloat2)
    assert(exceptionThrown)
    assert(client[client_testFloat2] == 31.13)

    clientTracker.markDirty(client_testFloat2)
    client.set_item_callback(client_testFloat2, callbackThreadChecker)
    server.set_float(itemFloat2, 13.31)
    clientTracker.waitClean(client_testFloat2)
    assert(callbackThreadAlive)

test(lambda: Client("nng", "role=client", "tcp://0.0.0.0:7777/"), server)
test(lambda: Client("nng", "role=client", "tcp://0.0.0.0:8888/"),
    Server(context_type="nng", params="role=server", uri="tcp://0.0.0.0:8888/"))
