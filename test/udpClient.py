#!/usr/bin/python3
# -*- coding: utf-8 -*-

from time import sleep
import socket
import signal
import json



def main():
    print("start udp-client...")
    server_addr = "localhost"
    server_port = 26106
    # socket machen
    sock = socket.socket( socket.AF_INET, socket.SOCK_DGRAM)
    addr = (server_addr,server_port)

    try:
        #print("get config-id...")
        #req = dict()
        #req['get'] = [ 'config-id' ]
        #msgb = json.dumps(req).encode(encoding='utf-8')
        #print("send data to server: <%s>..." % msgb.decode('utf-8'))
        #sent = sock.sendto(msgb, addr)
        #print("await response...")
        #data, server = sock.recvfrom(4094)
        #rec_msg = data.decode('utf-8')
        #print("recived \n{}".format(rec_msg))
        #print("get config-id...OK")

        #print("get alert-01 states...")
        #req = dict()
        #req['get'] = [ 'alert-01' ]
        #msgb = json.dumps(req).encode(encoding='utf-8')
        #print("send data to server: <%s>..." % msgb.decode('utf-8'))
        #sent = sock.sendto(msgb, addr)
        #print("await response...")
        #data, server = sock.recvfrom(4094)
        #rec_msg = data.decode('utf-8')
        #print("recived {}".format(rec_msg))
        #print("get alert-01 states...OK")

        #print("get all states...")
        #req = dict()
        #req['get'] = [ 'all' ]
        #msgb = json.dumps(req).encode(encoding='utf-8')
        #print("send data to server: <%s>..." % msgb.decode('utf-8'))
        #sent = sock.sendto(msgb, addr)
        #print("await response...")
        #data, server = sock.recvfrom(4094)
        #rec_msg = data.decode('utf-8')
        #print("recived {}".format(rec_msg))
        #print("get all states...OK")

        #print("get devices...")
        #req = dict()
        #req['get'] = [ 'devices' ]
        #msgb = json.dumps(req).encode(encoding='utf-8')
        #print("send data to server: <%s>..." % msgb.decode('utf-8'))
        #sent = sock.sendto(msgb, addr)
        #print("await response...")
        #data, server = sock.recvfrom(4094)
        #rec_msg = data.decode('utf-8')
        #print("recived {}".format(rec_msg))
        #print("get devices...OK")

        print("get new...")
        req = dict()
        req['get'] = [ 'new' ]
        msgb = json.dumps(req).encode(encoding='utf-8')
        print("send data to server: <%s>..." % msgb.decode('utf-8'))
        sent = sock.sendto(msgb, addr)
        print("await response...")
        data, server = sock.recvfrom(4094)
        rec_msg = data.decode('utf-8')
        print("recived {}".format(rec_msg))
        print("get new...OK")

        print("del alert-01")
        req = dict()
        req['del'] = [ 'alert-01' ]
        msgb = json.dumps(req).encode(encoding='utf-8')
        print("send data to server: <%s>..." % msgb.decode('utf-8'))
        sent = sock.sendto(msgb, addr)
        print("await response...")
        data, server = sock.recvfrom(4094)
        rec_msg = data.decode('utf-8')
        print("recived {}".format(rec_msg))
        print("del-alert-01...OK")

        # sleep(10)

    finally:
        print("close socket...")
        sock.close()
    print("END...")


if __name__ == '__main__':
    main()
