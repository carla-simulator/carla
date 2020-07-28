/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004-2007 by the SwarmNet (www.swarmnet.de) project  **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the BSD License. Refer to the shawn-licence.txt **
 ** file in the root of the Shawn source tree for further details.     **
 ************************************************************************/

#ifdef SHAWN
	#include <apps/tcpip/socket.h>
	#include <sys/simulation/simulation_controller.h>
#else
	#include "socket.h"
#endif

#ifdef BUILD_TCPIP


#ifndef WIN32
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <errno.h>
	#include <fcntl.h>
	#include <unistd.h>
#else
	#ifdef ERROR
		#undef ERROR
	#endif

	#include <winsock2.h>
	#include <ws2tcpip.h>

	#ifndef vsnprintf
		#define vsnprintf _vsnprintf
	#endif

#endif

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cassert>
#include <string>
#include <vector>
#include <string>
#include <algorithm>
#include <string.h>


#ifdef SHAWN
    extern "C" void init_tcpip( shawn::SimulationController& sc )
    {
            // std::cout << "tcpip init" << std::endl;
    }
#endif

namespace tcpip
{
	const int Socket::lengthLen = 4;

#ifdef WIN32
	bool Socket::init_windows_sockets_ = true;
	bool Socket::windows_sockets_initialized_ = false;
	int Socket::instance_count_ = 0;
#endif

	// ----------------------------------------------------------------------
	Socket::
		Socket(std::string host, int port) 
		: host_( host ),
		port_( port ),
		socket_(-1),
		server_socket_(-1),
		blocking_(true),
		verbose_(false)
	{
		init();
	}

	// ----------------------------------------------------------------------
	Socket::
		Socket(int port) 
		: host_(""),
		port_( port ),
		socket_(-1),
		server_socket_(-1),
		blocking_(true),
		verbose_(false)
	{
		init();
	}

	// ----------------------------------------------------------------------
	void
		Socket::
		init()
	{
#ifdef WIN32
		instance_count_++;

		if( init_windows_sockets_ && !windows_sockets_initialized_ )
		{
			WSAData wsaData;
			if( WSAStartup(MAKEWORD(1, 1), &wsaData) != 0 )
				BailOnSocketError("Unable to init WSA Sockets");
			windows_sockets_initialized_ = true;
		}
#endif
	}


    int
        Socket::
        getFreeSocketPort()
    {
        // Create socket to find a random free port that can be handed to the app
        int sock = static_cast<int>(socket( AF_INET, SOCK_STREAM, 0 ));
        struct sockaddr_in self;
        memset(&self, 0, sizeof(self));
        self.sin_family = AF_INET;
        self.sin_port = htons(0);
        self.sin_addr.s_addr = htonl(INADDR_ANY);

        socklen_t address_len = sizeof(self);
        // bind with port==0 assigns free port
        if ( bind(sock, (struct sockaddr*) &self, address_len) < 0)
            BailOnSocketError("tcpip::Socket::getFreeSocketPort() Unable to bind socket");
        // get the assigned port with getsockname
        if ( getsockname(sock, (struct sockaddr*) &self, &address_len) < 0)
            BailOnSocketError("tcpip::Socket::getFreeSocketPort() Unable to get socket name");
        const int port = ntohs(self.sin_port);
#ifdef WIN32
        ::closesocket( sock );
#else
        ::close( sock );
#endif
        return port;
    }


	// ----------------------------------------------------------------------
	Socket::
		~Socket()
	{
		// Close first an existing client connection ...
		close();
#ifdef WIN32
		instance_count_--;
#endif

		// ... then the server socket
		if( server_socket_ >= 0 )
		{
#ifdef WIN32
			::closesocket( server_socket_ );
#else
			::close( server_socket_ );
#endif
			server_socket_ = -1;
		}

#ifdef WIN32
		if( server_socket_ == -1 && socket_ == -1 
		    && init_windows_sockets_ && instance_count_ == 0 )
				WSACleanup();
                windows_sockets_initialized_ = false;
#endif
	}

	// ----------------------------------------------------------------------
	void 
		Socket::
		BailOnSocketError( std::string context)
	{
#ifdef WIN32
		int e = WSAGetLastError();
		std::string msg = GetWinsockErrorString( e );
#else
		std::string msg = strerror( errno );
#endif
		throw SocketException( context + ": " + msg );
	}

	// ----------------------------------------------------------------------
	int  
		Socket::
		port()
	{
		return port_;
	}


	// ----------------------------------------------------------------------
	bool 
		Socket::
		datawaiting(int sock) 
		const
	{
		fd_set fds;
		FD_ZERO( &fds );
		FD_SET( (unsigned int)sock, &fds );

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		int r = select( sock+1, &fds, nullptr, nullptr, &tv);

		if (r < 0)
			BailOnSocketError("tcpip::Socket::datawaiting @ select");

		if( FD_ISSET( sock, &fds ) )
			return true;
		else
			return false;
	}

	// ----------------------------------------------------------------------
	bool
		Socket::
		atoaddr( std::string address, struct sockaddr_in& addr)
	{
        int status;
        struct addrinfo *servinfo; // will point to the results

        struct addrinfo hints;
        memset(&hints, 0, sizeof hints); // make sure the struct is empty
        hints.ai_family = AF_INET; // restrict to IPv4?
        hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
        hints.ai_flags = AI_PASSIVE; // fill in my IP for me

        if ((status = getaddrinfo(address.c_str(), nullptr, &hints, &servinfo)) != 0) {
            return false;
        }
        
        bool valid = false;

        for (struct addrinfo *p = servinfo; p != nullptr; p = p->ai_next) {
            if (p->ai_family == AF_INET) { // IPv4
                addr = *(struct sockaddr_in *)p->ai_addr;
                addr.sin_port = htons((unsigned short)port_);
                valid = true;
                break;
            }
        }

        freeaddrinfo(servinfo); // free the linked list

		return valid;
	}


	// ----------------------------------------------------------------------
	Socket*
		Socket::
		accept(const bool create)
	{
		if( socket_ >= 0 )
			return nullptr;

		struct sockaddr_in client_addr;
#ifdef WIN32
		int addrlen = sizeof(client_addr);
#else
		socklen_t addrlen = sizeof(client_addr);
#endif

		if( server_socket_ < 0 )
		{
			struct sockaddr_in self;

			//Create the server socket
			server_socket_ = static_cast<int>(socket( AF_INET, SOCK_STREAM, 0 ));
			if( server_socket_ < 0 )
				BailOnSocketError("tcpip::Socket::accept() @ socket");
			
			//"Address already in use" error protection
			{

				#ifdef WIN32
					//setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr, sizeof(reuseaddr));
					// No address reuse in Windows!!!
				#else
    				int reuseaddr = 1;
					setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
				#endif
			}

			// Initialize address/port structure
			memset(&self, 0, sizeof(self));
			self.sin_family = AF_INET;
			self.sin_port = htons((unsigned short)port_);
			self.sin_addr.s_addr = htonl(INADDR_ANY);

			// Assign a port number to the socket
			if ( bind(server_socket_, (struct sockaddr*)&self, sizeof(self)) != 0 )
				BailOnSocketError("tcpip::Socket::accept() Unable to create listening socket");


			// Make it a "listening socket"
			if ( listen(server_socket_, 10) == -1 )
				BailOnSocketError("tcpip::Socket::accept() Unable to listen on server socket");

			// Make the newly created socket blocking or not
			set_blocking(blocking_);
		}

		socket_ = static_cast<int>(::accept(server_socket_, (struct sockaddr*)&client_addr, &addrlen));

		if( socket_ >= 0 )
		{
			int x = 1;
			setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (const char*)&x, sizeof(x));
            if (create) {
                Socket* result = new Socket(0);
                result->socket_ = socket_;
                socket_ = -1;
                return result;
            }
		}
        return nullptr;
	}

	// ----------------------------------------------------------------------
	void 
		Socket::
		set_blocking(bool blocking) 
	{
		blocking_ = blocking;

		if( server_socket_ > 0 )
		{
#ifdef WIN32
			ULONG NonBlock = blocking_ ? 0 : 1;
		    if (ioctlsocket(server_socket_, FIONBIO, &NonBlock) == SOCKET_ERROR)
				BailOnSocketError("tcpip::Socket::set_blocking() Unable to initialize non blocking I/O");
#else
			long arg = fcntl(server_socket_, F_GETFL, NULL);
			if (blocking_)
			{
				arg &= ~O_NONBLOCK;
			} else {
				arg |= O_NONBLOCK;
			}
			fcntl(server_socket_, F_SETFL, arg);
#endif
		}
	
	}

	// ----------------------------------------------------------------------
	void 
		Socket::
		connect()
	{
		sockaddr_in address;

		if( !atoaddr( host_.c_str(), address) )
			BailOnSocketError("tcpip::Socket::connect() @ Invalid network address");

		socket_ = static_cast<int>(socket( PF_INET, SOCK_STREAM, 0 ));
		if( socket_ < 0 )
			BailOnSocketError("tcpip::Socket::connect() @ socket");

		if( ::connect( socket_, (sockaddr const*)&address, sizeof(address) ) < 0 )
			BailOnSocketError("tcpip::Socket::connect() @ connect");

		if( socket_ >= 0 )
		{
			int x = 1;
			setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (const char*)&x, sizeof(x));
		}
    }

	// ----------------------------------------------------------------------
	void 
		Socket::
		close()
	{
		// Close client-connection 
		if( socket_ >= 0 )
		{
#ifdef WIN32
			::closesocket( socket_ );
#else
			::close( socket_ );
#endif

			socket_ = -1;
		}
	}

	// ----------------------------------------------------------------------
	void 
		Socket::
		send( const std::vector<unsigned char> &buffer)
	{
		if( socket_ < 0 )
			return;

		printBufferOnVerbose(buffer, "Send");

		size_t numbytes = buffer.size();
		unsigned char const *bufPtr = &buffer[0];
		while( numbytes > 0 )
		{
#ifdef WIN32
			int bytesSent = ::send( socket_, (const char*)bufPtr, static_cast<int>(numbytes), 0 );
#else
			int bytesSent = ::send( socket_, bufPtr, numbytes, 0 );
#endif
			if( bytesSent < 0 )
				BailOnSocketError( "send failed" );

			numbytes -= bytesSent;
			bufPtr += bytesSent;
		}
	}



	// ----------------------------------------------------------------------

	void
		Socket::
		sendExact( const Storage &b)
	{
		int length = static_cast<int>(b.size());
		Storage length_storage;
		length_storage.writeInt(lengthLen + length);

		// Sending length_storage and b independently would probably be possible and
		// avoid some copying here, but both parts would have to go through the
		// TCP/IP stack on their own which probably would cost more performance.
		std::vector<unsigned char> msg;
		msg.insert(msg.end(), length_storage.begin(), length_storage.end());
		msg.insert(msg.end(), b.begin(), b.end());
		send(msg);
	}


	// ----------------------------------------------------------------------
	size_t
		Socket::
		recvAndCheck(unsigned char * const buffer, std::size_t len)
		const
	{
#ifdef WIN32
		const int bytesReceived = recv( socket_, (char*)buffer, static_cast<int>(len), 0 );
#else
		const int bytesReceived = static_cast<int>(recv( socket_, buffer, len, 0 ));
#endif
		if( bytesReceived == 0 )
			throw SocketException( "tcpip::Socket::recvAndCheck @ recv: peer shutdown" );
		if( bytesReceived < 0 )
			BailOnSocketError( "tcpip::Socket::recvAndCheck @ recv" );

		return static_cast<size_t>(bytesReceived);
	}


	// ----------------------------------------------------------------------
	void
		Socket::
		receiveComplete(unsigned char * buffer, size_t len)
		const
	{
		while (len > 0)
		{
			const size_t bytesReceived = recvAndCheck(buffer, len);
			len -= bytesReceived;
			buffer += bytesReceived;
		}
	}


	// ----------------------------------------------------------------------
	void
		Socket::
		printBufferOnVerbose(const std::vector<unsigned char> buffer, const std::string &label)
		const
	{
		if (verbose_)
		{
			std::cerr << label << " " << buffer.size() <<  " bytes via tcpip::Socket: [";
			// cache end iterator for performance
			const std::vector<unsigned char>::const_iterator end = buffer.end();
			for (std::vector<unsigned char>::const_iterator it = buffer.begin(); end != it; ++it)
				std::cerr << " " << static_cast<int>(*it) << " ";
			std::cerr << "]" << std::endl;
		}
	}


	// ----------------------------------------------------------------------
	std::vector<unsigned char> 
		Socket::
		receive(int bufSize)
	{
		std::vector<unsigned char> buffer;

		if( socket_ < 0 )
			connect();

		if( !datawaiting( socket_) )
			return buffer;

		buffer.resize(bufSize);
		const size_t bytesReceived = recvAndCheck(&buffer[0], bufSize);

		buffer.resize(bytesReceived);

		printBufferOnVerbose(buffer, "Rcvd");

		return buffer;
	}

	// ----------------------------------------------------------------------
	

	bool
		Socket::
		receiveExact( Storage &msg )
	{
		// buffer for received bytes
		// According to the C++ standard elements of a std::vector are stored
		// contiguously. Explicitly &buffer[n] == &buffer[0] + n for 0 <= n < buffer.size().
		std::vector<unsigned char> buffer(lengthLen);

		// receive length of TraCI message
		receiveComplete(&buffer[0], lengthLen);
		Storage length_storage(&buffer[0], lengthLen);
		const int totalLen = length_storage.readInt();
		assert(totalLen > lengthLen);

		// extent buffer
		buffer.resize(totalLen);

		// receive remaining TraCI message
		receiveComplete(&buffer[lengthLen], totalLen - lengthLen);

		// copy message content into passed Storage
		msg.reset();
		msg.writePacket(&buffer[lengthLen], totalLen - lengthLen);
		
		printBufferOnVerbose(buffer, "Rcvd Storage with");

		return true;
	}
	
	
	// ----------------------------------------------------------------------
	bool 
		Socket::
		has_client_connection() 
		const
	{
		return socket_ >= 0;
	}

	// ----------------------------------------------------------------------
	bool 
		Socket::
		is_blocking() 
	{
		return blocking_;
	}


#ifdef WIN32
	// ----------------------------------------------------------------------
	std::string 
		Socket::
		GetWinsockErrorString(int err) 
	{

		switch( err)
		{
		case 0:					return "No error";
		case WSAEINTR:			return "Interrupted system call";
		case WSAEBADF:			return "Bad file number";
		case WSAEACCES:			return "Permission denied";
		case WSAEFAULT:			return "Bad address";
		case WSAEINVAL:			return "Invalid argument";
		case WSAEMFILE:			return "Too many open sockets";
		case WSAEWOULDBLOCK:	return "Operation would block";
		case WSAEINPROGRESS:	return "Operation now in progress";
		case WSAEALREADY:		return "Operation already in progress";
		case WSAENOTSOCK:		return "Socket operation on non-socket";
		case WSAEDESTADDRREQ:	return "Destination address required";
		case WSAEMSGSIZE:		return "Message too long";
		case WSAEPROTOTYPE:		return "Protocol wrong type for socket";
		case WSAENOPROTOOPT:	return "Bad protocol option";
		case WSAEPROTONOSUPPORT:	return "Protocol not supported";
		case WSAESOCKTNOSUPPORT:	return "Socket type not supported";
		case WSAEOPNOTSUPP:		return "Operation not supported on socket";
		case WSAEPFNOSUPPORT:	return "Protocol family not supported";
		case WSAEAFNOSUPPORT:	return "Address family not supported";
		case WSAEADDRINUSE:		return "Address already in use";
		case WSAEADDRNOTAVAIL:	return "Can't assign requested address";
		case WSAENETDOWN:		return "Network is down";
		case WSAENETUNREACH:	return "Network is unreachable";
		case WSAENETRESET:		return "Net Socket reset";
		case WSAECONNABORTED:	return "Software caused tcpip::Socket abort";
		case WSAECONNRESET:		return "Socket reset by peer";
		case WSAENOBUFS:		return "No buffer space available";
		case WSAEISCONN:		return "Socket is already connected";
		case WSAENOTCONN:		return "Socket is not connected";
		case WSAESHUTDOWN:		return "Can't send after socket shutdown";
		case WSAETOOMANYREFS:	return "Too many references, can't splice";
		case WSAETIMEDOUT:		return "Socket timed out";
		case WSAECONNREFUSED:	return "Socket refused";
		case WSAELOOP:			return "Too many levels of symbolic links";
		case WSAENAMETOOLONG:	return "File name too long";
		case WSAEHOSTDOWN:		return "Host is down";
		case WSAEHOSTUNREACH:	return "No route to host";
		case WSAENOTEMPTY:		return "Directory not empty";
		case WSAEPROCLIM:		return "Too many processes";
		case WSAEUSERS:			return "Too many users";
		case WSAEDQUOT:			return "Disc quota exceeded";
		case WSAESTALE:			return "Stale NFS file handle";
		case WSAEREMOTE:		return "Too many levels of remote in path";
		case WSASYSNOTREADY:	return "Network system is unavailable";
		case WSAVERNOTSUPPORTED:	return "Winsock version out of range";
		case WSANOTINITIALISED:	return "WSAStartup not yet called";
		case WSAEDISCON:		return "Graceful shutdown in progress";
		case WSAHOST_NOT_FOUND:	return "Host not found";
		case WSANO_DATA:		return "No host data of that type was found";
		}

		return "unknown";
	}

#endif // WIN32

} // namespace tcpip

#endif // BUILD_TCPIP

/*-----------------------------------------------------------------------
* Source  $Source: $
* Version $Revision: 645 $
* Date    $Date: 2012-04-27 14:03:33 +0200 (Fri, 27 Apr 2012) $
*-----------------------------------------------------------------------
* $Log: $
*-----------------------------------------------------------------------*/
