#!/usr/bin/env python3

#
# Example for Python3 binding
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

# This example assumes tweak-mock-server being run as a parallel process

from tweak2 import Client
from time import sleep

client = Client("nng", "role=client", "tcp://0.0.0.0:7777/")
[jira_153_1_id, jira_153_2_id, test_id, test1_id, test2_id, test3_id,\
 tweak_jira_153_4_id, tweak_jira_153_5_id, tweak_jira_153_6_id ] = \
                  client.collect(["/test/jira_153_1", \
                                  "/test/jira_153_2", \
                                  "/test/test", \
                                  "/test/test1", \
                                  "/test/test2", \
                                  "/test/test3", \
                                  "/test/tweak_jira_153_4", \
                                  "/test/tweak_jira_153_5", \
                                  "/test/tweak_jira_153_6"])

print("/test/jira_153_1 = {}".format(client[jira_153_1_id]))
print("/test/jira_153_2 = {}".format(client[jira_153_2_id]))
print("/test/test = {}".format(client[test_id]))
print("/test/test1 = {}".format(client[test1_id]))
print("/test/test2 = {}".format(client[test2_id]))
print("/test/test3 = {}".format(client[test3_id]))
print("/test/tweak_jira_153_4 = {}".format(client[tweak_jira_153_4_id]))
print("/test/tweak_jira_153_5 = {}".format(client[tweak_jira_153_5_id]))
print("/test/tweak_jira_153_6 = {}".format(client[tweak_jira_153_6_id]))

client[test_id] = 24
print("/test/test = {}".format(client[test_id]))

print("/test/test = {}".format(client[test_id]))
client.set_item_callback(test1_id, lambda _, arg: print("/test/test1 = {}".format(arg)))

sleep(5)
