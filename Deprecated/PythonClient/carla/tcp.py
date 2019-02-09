# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
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
    """
    Basic networking client for TCP connections. Errors occurred during
    networking operations are raised as TCPConnectionError.

    Received messages are expected to be prepended by a int32 defining the
    message size. Messages are sent following this convention.
    """

    def __init__(self, host, port, timeout):
        self._host = host
        self._port = port
        self._timeout = timeout
        self._socket = None
        self._logprefix = '(%s:%s) ' % (self._host, self._port)

    def connect(self, connection_attempts=10):
        """Try to establish a connection to the given host:port."""
        connection_attempts = max(1, connection_attempts)
        error = None
        for attempt in range(1, connection_attempts + 1):
            try:
                self._socket = socket.create_connection(address=(self._host, self._port), timeout=self._timeout)
                self._socket.settimeout(self._timeout)
                logging.debug('%sconnected', self._logprefix)
                return
            except socket.error as exception:
                error = exception
                logging.debug('%sconnection attempt %d: %s', self._logprefix, attempt, error)
                time.sleep(1)
        self._reraise_exception_as_tcp_error('failed to connect', error)

    def disconnect(self):
        """Disconnect any active connection."""
        if self._socket is not None:
            logging.debug('%sdisconnecting', self._logprefix)
            self._socket.close()
            self._socket = None

    def connected(self):
        """Return whether there is an active connection."""
        return self._socket is not None

    def write(self, message):
        """Send message to the server."""
        if self._socket is None:
            raise TCPConnectionError(self._logprefix + 'not connected')
        header = struct.pack('<L', len(message))
        try:
            self._socket.sendall(header + message)
        except socket.error as exception:
            self._reraise_exception_as_tcp_error('failed to write data', exception)

    def read(self):
        """Read a message from the server."""
        header = self._read_n(4)
        if not header:
            raise TCPConnectionError(self._logprefix + 'connection closed')
        length = struct.unpack('<L', header)[0]
        data = self._read_n(length)
        return data

    def _read_n(self, length):
        """Read n bytes from the socket."""
        if self._socket is None:
            raise TCPConnectionError(self._logprefix + 'not connected')
        buf = bytes()
        while length > 0:
            try:
                data = self._socket.recv(length)
            except socket.error as exception:
                self._reraise_exception_as_tcp_error('failed to read data', exception)
            if not data:
                raise TCPConnectionError(self._logprefix + 'connection closed')
            buf += data
            length -= len(data)
        return buf

    def _reraise_exception_as_tcp_error(self, message, exception):
        raise TCPConnectionError('%s%s: %s' % (self._logprefix, message, exception))
