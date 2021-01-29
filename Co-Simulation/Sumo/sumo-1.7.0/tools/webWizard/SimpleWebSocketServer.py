# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# The MIT License (MIT) Copyright (c) 2013 Dave P.

# @file    SimpleWebSocketServer.py
# @author  Dave Pallot
# @date    2013

"""
A web socket server implementation to be used by the osm server.py
Originally distributed at
https://github.com/dpallot/simple-websocket-server/tree/master/SimpleWebSocketServer.
"""
import sys
import hashlib
import base64
import socket
import struct
import ssl
import errno
import codecs
from collections import deque
from select import select

VER = sys.version_info[0]
if VER >= 3:
    import socketserver  # noqa
    from http.server import BaseHTTPRequestHandler
    from io import StringIO, BytesIO
else:
    import SocketServer  # noqa
    from BaseHTTPServer import BaseHTTPRequestHandler
    from StringIO import StringIO


__all__ = ['WebSocket',
           'SimpleWebSocketServer',
           'SimpleSSLWebSocketServer']


def _check_unicode(val):
    if VER >= 3:
        return isinstance(val, str)
    else:
        # python 2.7
        return isinstance(val, unicode)  # noqa


class HTTPRequest(BaseHTTPRequestHandler):

    def __init__(self, request_text):
        if VER >= 3:
            self.rfile = BytesIO(request_text)
        else:
            self.rfile = StringIO(request_text)
        self.raw_requestline = self.rfile.readline()
        self.error_code = self.error_message = None
        self.parse_request()


_VALID_STATUS_CODES = [1000, 1001, 1002, 1003, 1007, 1008,
                       1009, 1010, 1011, 3000, 3999, 4000, 4999]

HANDSHAKE_STR = (
    "HTTP/1.1 101 Switching Protocols\r\n"
    "Upgrade: WebSocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Accept: %(acceptstr)s\r\n\r\n"
)

GUID_STR = '258EAFA5-E914-47DA-95CA-C5AB0DC85B11'

STREAM = 0x0
TEXT = 0x1
BINARY = 0x2
CLOSE = 0x8
PING = 0x9
PONG = 0xA

HEADERB1 = 1
HEADERB2 = 3
LENGTHSHORT = 4
LENGTHLONG = 5
MASK = 6
PAYLOAD = 7

MAXHEADER = 65536
MAXPAYLOAD = 33554432


class WebSocket(object):

    def __init__(self, server, sock, address):
        self.server = server
        self.client = sock
        self.address = address

        self.handshaked = False
        self.headerbuffer = bytearray()
        self.headertoread = 2048

        self.fin = 0
        self.data = bytearray()
        self.opcode = 0
        self.hasmask = 0
        self.maskarray = None
        self.length = 0
        self.lengtharray = None
        self.index = 0
        self.request = None
        self.usingssl = False

        self.frag_start = False
        self.frag_type = BINARY
        self.frag_buffer = None
        self.frag_decoder = codecs.getincrementaldecoder(
            'utf-8')(errors='strict')
        self.closed = False
        self.sendq = deque()

        self.state = HEADERB1

        # restrict the size of header and payload for security reasons
        self.maxheader = MAXHEADER
        self.maxpayload = MAXPAYLOAD

    def handleMessage(self):
        """
            Called when websocket frame is received.
            To access the frame data call self.data.

            If the frame is Text then self.data is a unicode object.
            If the frame is Binary then self.data is a bytearray object.
        """
        pass

    def handleConnected(self):
        """
            Called when a websocket client connects to the server.
        """
        pass

    def handleClose(self):
        """
            Called when a websocket server gets a Close frame from a client.
        """
        pass

    def _handlePacket(self):
        if self.opcode == CLOSE:
            pass
        elif self.opcode == STREAM:
            pass
        elif self.opcode == TEXT:
            pass
        elif self.opcode == BINARY:
            pass
        elif self.opcode == PONG or self.opcode == PING:
            if len(self.data) > 125:
                raise Exception('control frame length can not be > 125')
        else:
            # unknown or reserved opcode so just close
            raise Exception('unknown opcode')

        if self.opcode == CLOSE:
            status = 1000
            reason = u''
            length = len(self.data)

            if length == 0:
                pass
            elif length >= 2:
                status = struct.unpack_from('!H', self.data[:2])[0]
                reason = self.data[2:]

                if status not in _VALID_STATUS_CODES:
                    status = 1002

                if len(reason) > 0:
                    try:
                        reason = reason.decode('utf8', errors='strict')
                    except Exception:
                        status = 1002
            else:
                status = 1002

            self.close(status, reason)
            return

        elif self.fin == 0:
            if self.opcode != STREAM:
                if self.opcode == PING or self.opcode == PONG:
                    raise Exception('control messages can not be fragmented')

                self.frag_type = self.opcode
                self.frag_start = True
                self.frag_decoder.reset()

                if self.frag_type == TEXT:
                    self.frag_buffer = []
                    utf_str = self.frag_decoder.decode(self.data, final=False)
                    if utf_str:
                        self.frag_buffer.append(utf_str)
                else:
                    self.frag_buffer = bytearray()
                    self.frag_buffer.extend(self.data)

            else:
                if self.frag_start is False:
                    raise Exception('fragmentation protocol error')

                if self.frag_type == TEXT:
                    utf_str = self.frag_decoder.decode(self.data, final=False)
                    if utf_str:
                        self.frag_buffer.append(utf_str)
                else:
                    self.frag_buffer.extend(self.data)

        else:
            if self.opcode == STREAM:
                if self.frag_start is False:
                    raise Exception('fragmentation protocol error')

                if self.frag_type == TEXT:
                    utf_str = self.frag_decoder.decode(self.data, final=True)
                    self.frag_buffer.append(utf_str)
                    self.data = u''.join(self.frag_buffer)
                else:
                    self.frag_buffer.extend(self.data)
                    self.data = self.frag_buffer

                self.handleMessage()

                self.frag_decoder.reset()
                self.frag_type = BINARY
                self.frag_start = False
                self.frag_buffer = None

            elif self.opcode == PING:
                self._sendMessage(False, PONG, self.data)

            elif self.opcode == PONG:
                pass

            else:
                if self.frag_start is True:
                    raise Exception('fragmentation protocol error')

                if self.opcode == TEXT:
                    try:
                        self.data = self.data.decode('utf8', errors='strict')
                    except Exception:
                        raise Exception('invalid utf-8 payload')

                self.handleMessage()

    def _handleData(self):
        # do the HTTP header and handshake
        if self.handshaked is False:

            data = self.client.recv(self.headertoread)
            if not data:
                raise Exception('remote socket closed')

            else:
                # accumulate
                self.headerbuffer.extend(data)

                if len(self.headerbuffer) >= self.maxheader:
                    raise Exception('header exceeded allowable size')

                # indicates end of HTTP header
                if b'\r\n\r\n' in self.headerbuffer:
                    self.request = HTTPRequest(self.headerbuffer)

                    # handshake rfc 6455
                    try:
                        key = self.request.headers['Sec-WebSocket-Key']
                        k = key.encode('ascii') + GUID_STR.encode('ascii')
                        k_s = base64.b64encode(
                            hashlib.sha1(k).digest()).decode('ascii')
                        hStr = HANDSHAKE_STR % {'acceptstr': k_s}
                        self.sendq.append((BINARY, hStr.encode('ascii')))
                        self.handshaked = True
                        self.handleConnected()
                    except Exception as e:
                        raise Exception('handshake failed: %s', str(e))

        # else do normal data
        else:
            data = self.client.recv(16384)
            if not data:
                raise Exception("remote socket closed")

            if VER >= 3:
                for d in data:
                    self._parseMessage(d)
            else:
                for d in data:
                    self._parseMessage(ord(d))

    def close(self, status=1000, reason=u''):
        """
           Send Close frame to the client. The underlying socket is only closed
           when the client acknowledges the Close frame.

           status is the closing identifier.
           reason is the reason for the close.
         """
        try:
            if self.closed is False:
                close_msg = bytearray()
                close_msg.extend(struct.pack("!H", status))
                if _check_unicode(reason):
                    close_msg.extend(reason.encode('utf-8'))
                else:
                    close_msg.extend(reason)

                self._sendMessage(False, CLOSE, close_msg)

        finally:
            self.closed = True

    def _sendBuffer(self, buff, send_all=False):
        size = len(buff)
        tosend = size
        already_sent = 0

        while tosend > 0:
            try:
                # i should be able to send a bytearray
                sent = self.client.send(buff[already_sent:])
                if sent == 0:
                    raise RuntimeError('socket connection broken')

                already_sent += sent
                tosend -= sent

            except socket.error as e:
                # if we have full buffers then wait for them to drain and try
                # again
                if e.errno in [errno.EAGAIN, errno.EWOULDBLOCK]:
                    if send_all:
                        continue
                    return buff[already_sent:]
                else:
                    raise e

        return None

    def sendFragmentStart(self, data):
        """
            Send the start of a data fragment stream to a websocket client.
            Subsequent data should be sent using sendFragment().
            A fragment stream is completed when sendFragmentEnd() is called.

            If data is a unicode object then the frame is sent as Text.
            If the data is a bytearray object then the frame is sent as Binary.
        """
        opcode = BINARY
        if _check_unicode(data):
            opcode = TEXT
        self._sendMessage(True, opcode, data)

    def sendFragment(self, data):
        """
            see sendFragmentStart()

            If data is a unicode object then the frame is sent as Text.
            If the data is a bytearray object then the frame is sent as Binary.
        """
        self._sendMessage(True, STREAM, data)

    def sendFragmentEnd(self, data):
        """
            see sendFragmentEnd()

            If data is a unicode object then the frame is sent as Text.
            If the data is a bytearray object then the frame is sent as Binary.
        """
        self._sendMessage(False, STREAM, data)

    def sendMessage(self, data):
        """
            Send websocket data frame to the client.

            If data is a unicode object then the frame is sent as Text.
            If the data is a bytearray object then the frame is sent as Binary.
        """
        opcode = BINARY
        if _check_unicode(data):
            opcode = TEXT
        self._sendMessage(False, opcode, data)

    def _sendMessage(self, fin, opcode, data):

        payload = bytearray()

        b1 = 0
        b2 = 0
        if fin is False:
            b1 |= 0x80
        b1 |= opcode

        if _check_unicode(data):
            data = data.encode('utf-8')

        length = len(data)
        payload.append(b1)

        if length <= 125:
            b2 |= length
            payload.append(b2)

        elif length >= 126 and length <= 65535:
            b2 |= 126
            payload.append(b2)
            payload.extend(struct.pack("!H", length))

        else:
            b2 |= 127
            payload.append(b2)
            payload.extend(struct.pack("!Q", length))

        if length > 0:
            payload.extend(data)

        self.sendq.append((opcode, payload))

    def _parseMessage(self, byte):
        # read in the header
        if self.state == HEADERB1:

            self.fin = byte & 0x80
            self.opcode = byte & 0x0F
            self.state = HEADERB2

            self.index = 0
            self.length = 0
            self.lengtharray = bytearray()
            self.data = bytearray()

            rsv = byte & 0x70
            if rsv != 0:
                raise Exception('RSV bit must be 0')

        elif self.state == HEADERB2:
            mask = byte & 0x80
            length = byte & 0x7F

            if self.opcode == PING and length > 125:
                raise Exception('ping packet is too large')

            if mask == 128:
                self.hasmask = True
            else:
                self.hasmask = False

            if length <= 125:
                self.length = length

                # if we have a mask we must read it
                if self.hasmask is True:
                    self.maskarray = bytearray()
                    self.state = MASK
                else:
                    # if there is no mask and no payload we are done
                    if self.length <= 0:
                        try:
                            self._handlePacket()
                        finally:
                            self.state = self.HEADERB1
                            self.data = bytearray()

                    # we have no mask and some payload
                    else:
                        # self.index = 0
                        self.data = bytearray()
                        self.state = PAYLOAD

            elif length == 126:
                self.lengtharray = bytearray()
                self.state = LENGTHSHORT

            elif length == 127:
                self.lengtharray = bytearray()
                self.state = LENGTHLONG

        elif self.state == LENGTHSHORT:
            self.lengtharray.append(byte)

            if len(self.lengtharray) > 2:
                raise Exception('short length exceeded allowable size')

            if len(self.lengtharray) == 2:
                self.length = struct.unpack_from('!H', self.lengtharray)[0]

                if self.hasmask is True:
                    self.maskarray = bytearray()
                    self.state = MASK
                else:
                    # if there is no mask and no payload we are done
                    if self.length <= 0:
                        try:
                            self._handlePacket()
                        finally:
                            self.state = HEADERB1
                            self.data = bytearray()

                    # we have no mask and some payload
                    else:
                        # self.index = 0
                        self.data = bytearray()
                        self.state = PAYLOAD

        elif self.state == LENGTHLONG:

            self.lengtharray.append(byte)

            if len(self.lengtharray) > 8:
                raise Exception('long length exceeded allowable size')

            if len(self.lengtharray) == 8:
                self.length = struct.unpack_from('!Q', self.lengtharray)[0]

                if self.hasmask is True:
                    self.maskarray = bytearray()
                    self.state = MASK
                else:
                    # if there is no mask and no payload we are done
                    if self.length <= 0:
                        try:
                            self._handlePacket()
                        finally:
                            self.state = HEADERB1
                            self.data = bytearray()

                    # we have no mask and some payload
                    else:
                        # self.index = 0
                        self.data = bytearray()
                        self.state = PAYLOAD

        # MASK STATE
        elif self.state == MASK:
            self.maskarray.append(byte)

            if len(self.maskarray) > 4:
                raise Exception('mask exceeded allowable size')

            if len(self.maskarray) == 4:
                # if there is no mask and no payload we are done
                if self.length <= 0:
                    try:
                        self._handlePacket()
                    finally:
                        self.state = HEADERB1
                        self.data = bytearray()

                # we have no mask and some payload
                else:
                    # self.index = 0
                    self.data = bytearray()
                    self.state = PAYLOAD

        # PAYLOAD STATE
        elif self.state == PAYLOAD:
            if self.hasmask is True:
                self.data.append(byte ^ self.maskarray[self.index % 4])
            else:
                self.data.append(byte)

            # if length exceeds allowable size then we except and remove the
            # connection
            if len(self.data) >= self.maxpayload:
                raise Exception('payload exceeded allowable size')

            # check if we have processed length bytes; if so we are done
            if (self.index + 1) == self.length:
                try:
                    self._handlePacket()
                finally:
                    # self.index = 0
                    self.state = HEADERB1
                    self.data = bytearray()
            else:
                self.index += 1


class SimpleWebSocketServer(object):

    def __init__(self, host, port, websocketclass, selectInterval=0.1):
        self.websocketclass = websocketclass
        self.serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.serversocket.bind((host, port))
        self.serversocket.listen(5)
        self.selectInterval = selectInterval
        self.connections = {}
        self.listeners = [self.serversocket]

    def _decorateSocket(self, sock):
        return sock

    def _constructWebSocket(self, sock, address):
        return self.websocketclass(self, sock, address)

    def close(self):
        self.serversocket.close()

        for desc, conn in self.connections.items():
            conn.close()
            conn.handleClose()

    def serveforever(self):
        while True:
            writers = []
            for fileno in self.listeners:
                if fileno == self.serversocket:
                    continue
                client = self.connections[fileno]
                if client.sendq:
                    writers.append(fileno)

            if self.selectInterval:
                rList, wList, xList = select(
                    self.listeners, writers, self.listeners, self.selectInterval)
            else:
                rList, wList, xList = select(
                    self.listeners, writers, self.listeners)

            for ready in wList:
                client = self.connections[ready]
                try:
                    while client.sendq:
                        opcode, payload = client.sendq.popleft()
                        remaining = client._sendBuffer(payload)
                        if remaining is not None:
                            client.sendq.appendleft((opcode, remaining))
                            break
                        else:
                            if opcode == CLOSE:
                                raise Exception('received client close')

                except Exception:
                    client.client.close()
                    client.handleClose()
                    del self.connections[ready]
                    self.listeners.remove(ready)

            for ready in rList:
                if ready == self.serversocket:
                    try:
                        sock, address = self.serversocket.accept()
                        newsock = self._decorateSocket(sock)
                        newsock.setblocking(0)
                        fileno = newsock.fileno()
                        self.connections[fileno] = self._constructWebSocket(
                            newsock, address)
                        self.listeners.append(fileno)
                    except Exception:
                        if sock is not None:
                            sock.close()
                else:
                    if ready not in self.connections:
                        continue
                    client = self.connections[ready]
                    try:
                        client._handleData()
                    except Exception:
                        client.client.close()
                        client.handleClose()
                        del self.connections[ready]
                        self.listeners.remove(ready)

            for failed in xList:
                if failed == self.serversocket:
                    self.close()
                    raise Exception('server socket failed')
                else:
                    if failed not in self.connections:
                        continue
                    client = self.connections[failed]
                    client.client.close()
                    client.handleClose()
                    del self.connections[failed]
                    self.listeners.remove(failed)


class SimpleSSLWebSocketServer(SimpleWebSocketServer):

    def __init__(self, host, port, websocketclass, certfile,
                 keyfile, version=ssl.PROTOCOL_TLSv1, selectInterval=0.1):

        SimpleWebSocketServer.__init__(self, host, port,
                                       websocketclass, selectInterval)

        self.context = ssl.SSLContext(version)
        self.context.load_cert_chain(certfile, keyfile)

    def close(self):
        super(SimpleSSLWebSocketServer, self).close()

    def _decorateSocket(self, sock):
        sslsock = self.context.wrap_socket(sock, server_side=True)
        return sslsock

    def _constructWebSocket(self, sock, address):
        ws = self.websocketclass(self, sock, address)
        ws.usingssl = True
        return ws

    def serveforever(self):
        super(SimpleSSLWebSocketServer, self).serveforever()
