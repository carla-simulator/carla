# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Basic TCP client."""

import logging
import socket
import struct
import time

class TCPConnectionError(Exception):
    pass


class TCPClient(object):
    def __init__(self, host, port, timeout):
        self._host = host
        self._port = port
        self._timeout = timeout
        self._socket = None
        self._logprefix = '(%s:%s) ' % (self._host, self._port)

    def connect(self):
        for attempt in range(10):
            try:
                self._socket = socket.create_connection(address=(self._host, self._port), timeout=self._timeout)
                self._socket.settimeout(self._timeout)
                logging.debug(self._logprefix + 'connected')
                return
            except Exception as exception:
                logging.debug(self._logprefix + 'connection attempt %d: %s', attempt, exception)
                time.sleep(1)
                continue
        self._reraise_exception_as_tcp_error('failed to connect', exception)

    def disconnect(self):
        if self._socket is not None:
            logging.debug(self._logprefix + 'disconnecting')
            self._socket.close()
            self._socket = None

    def connected(self):
        return self._socket is not None

    def write(self, message):
        if self._socket is None:
            raise TCPConnectionError(self._logprefix + 'not connected')
        header = struct.pack('<L', len(message))
        try:
            self._socket.sendall(header + message)
        except Exception as exception:
            self._reraise_exception_as_tcp_error('failed to write data', exception)

    def read(self):
        header = self._read_n(4)
        if not header:
            raise TCPConnectionError(self._logprefix + 'connection closed')
        length = struct.unpack('<L', header)[0]
        data = self._read_n(length)
        return data

    def _read_n(self, length):
        if self._socket is None:
            raise TCPConnectionError(self._logprefix + 'not connected')
        buf = bytes()
        while length > 0:
            try:
                data = self._socket.recv(length)
            except Exception as exception:
                self._reraise_exception_as_tcp_error('failed to read data', exception)
            if not data:
                raise TCPConnectionError(self._logprefix + 'connection closed')
            buf += data
            length -= len(data)
        return buf

    def _reraise_exception_as_tcp_error(self, message, exception):
        raise TCPConnectionError('%s%s: %s' % (self._logprefix, message, exception))
