/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004-2007 by the SwarmNet (www.swarmnet.de) project  **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the BSD License. Refer to the shawn-licence.txt **
 ** file in the root of the Shawn source tree for further details.     **
 ************************************************************************/

#ifndef __SHAWN_APPS_TCPIP_SOCKET_H
#define __SHAWN_APPS_TCPIP_SOCKET_H

#ifdef SHAWN
     #include <shawn_config.h>
     #include "_apps_enable_cmake.h"
     #ifdef ENABLE_TCPIP
            #define BUILD_TCPIP
     #endif
#else
     #define BUILD_TCPIP
#endif


#ifdef BUILD_TCPIP

// Get Storage
#ifdef SHAWN
	#include <apps/tcpip/storage.h>
#else
	#include "storage.h"
#endif

#ifdef SHAWN
     namespace shawn
      { class SimulationController; }

     // Dummy function is called when Shawn Simulation starts. Does nothing up to now.
     extern "C" void init_tcpip( shawn::SimulationController& );
#endif

#include <string>
#include <map>
#include <vector>
#include <list>
#include <deque>
#include <iostream>
#include <cstddef>


struct sockaddr_in;

namespace tcpip
{

	class SocketException: public std::runtime_error
	{
	public:
        SocketException(std::string what) : std::runtime_error(what.c_str()) {}
	};

	class Socket
	{
		friend class Response;
	public:
		/// Constructor that prepare to connect to host:port 
		Socket(std::string host, int port);
		
		/// Constructor that prepare for accepting a connection on given port
		Socket(int port);

		/// Destructor
		~Socket();

		/// @brief Returns an free port on the system
		/// @note This is done by binding a socket with port=0, getting the assigned port, and closing the socket again
		static int getFreeSocketPort();

		/// Connects to host_:port_
		void connect();

		/// Wait for a incoming connection to port_
        Socket* accept(const bool create = false);

		void send( const std::vector<unsigned char> &buffer);
		void sendExact( const Storage & );
		/// Receive up to \p bufSize available bytes from Socket::socket_
		std::vector<unsigned char> receive( int bufSize = 2048 );
		/// Receive a complete TraCI message from Socket::socket_
		bool receiveExact( Storage &);
		void close();
		int port();
		void set_blocking(bool);
		bool is_blocking();
		bool has_client_connection() const;

		// If verbose, each send and received data is written to stderr
		bool verbose() { return verbose_; }
		void set_verbose(bool newVerbose) { verbose_ = newVerbose; }

	protected:
		/// Length of the message length part of a TraCI message
		static const int lengthLen;

		/// Receive \p len bytes from Socket::socket_
		void receiveComplete(unsigned char * const buffer, std::size_t len) const;
		/// Receive up to \p len available bytes from Socket::socket_
		size_t recvAndCheck(unsigned char * const buffer, std::size_t len) const;
		/// Print \p label and \p buffer to stderr if Socket::verbose_ is set
		void printBufferOnVerbose(const std::vector<unsigned char> buffer, const std::string &label) const;

	private:
		void init();
		static void BailOnSocketError(std::string context);
#ifdef WIN32
		static std::string GetWinsockErrorString(int err);
#endif
		bool atoaddr(std::string, struct sockaddr_in& addr);
		bool datawaiting(int sock) const;

		std::string host_;
		int port_;
		int socket_;
		int server_socket_;
		bool blocking_;

		bool verbose_;
#ifdef WIN32
		static bool init_windows_sockets_;
		static bool windows_sockets_initialized_;
		static int instance_count_;
#endif
	};

}	// namespace tcpip

#endif // BUILD_TCPIP

#endif

/*-----------------------------------------------------------------------
* Source  $Source: $
* Version $Revision: 612 $
* Date    $Date: 2011-06-14 15:16:52 +0200 (Tue, 14 Jun 2011) $
*-----------------------------------------------------------------------
* $Log:$
*-----------------------------------------------------------------------*/
