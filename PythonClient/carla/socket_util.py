#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import binascii
import socket
import struct
import logging
import time
import select

def int2bytes(i):
    hex_string = '%x' % i
    n = len(hex_string)
    return binascii.unhexlify(hex_string.zfill(n + (n & 1)))

def bytes2int(str):
	return int(str.encode('hex'), 16)
	

def connect(host,port):

	try:
		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		sock.settimeout(100)
	except socket.error:
		logginge.exception("Error Creating Socket World")
	
	sock.connect((host,port))
	logging.debug("connected to %s port %d" %(host,port))

	return sock

def pers_connect(host,port):
	for attempt in range(10):
		try:
			sock = connect(host,port)
			return sock
		except Exception as e:
			logging.exception("Failure on connection")
			time.sleep(1)
			continue
		else:			
			break
	raise socket.error
	

def disconnect(sock):

	sock.shutdown(socket.SHUT_RDWR)
	sock.close()


def send_message(sock, message):
	""" Send a serialized message (protobuf Message interface)
	    to a socket, prepended by its length packed in 4
	    bytes (big endian).
	"""

	s = message.SerializeToString()
	packed_len =struct.pack('<L', len(s)) #int2bytes(len(s))
	logging.debug( "SOCKET Send: %d bytes" % int(len(s)))

	sock.sendall(packed_len + s)




def get_message(sock):
	""" Read a message from a socket.
	"""


	len_buf = socket_read_n(sock, 4)
	
	msg_len = struct.unpack('<L', len_buf)[0]
	logging.debug( "SOCKET RECEIVED: %d bytes" % msg_len)

	msg_buf = socket_read_n(sock, msg_len)

	return msg_buf


def socket_read_n(sock,n):
	""" Read exactly n bytes from the socket.
	    Raise RuntimeError if the connection closed before
	    n bytes were read.
	"""

	buf = b''
	while n > 0:
	
		sock.setblocking(0)
		ready = select.select([sock], [], [], 3)
		if ready[0]:
			try:
				data = sock.recv(n)
				if data == b'':
					raise RuntimeError('unexpected connection close')
				buf += data
				n -= len(data)
			except socket.error:
				raise socket.error

	sock.setblocking(1)


	return buf


