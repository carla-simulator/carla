/************************************************************************
 ** This file is part of the network simulator Shawn.                  **
 ** Copyright (C) 2004-2007 by the SwarmNet (www.swarmnet.de) project  **
 ** Shawn is free software; you can redistribute it and/or modify it   **
 ** under the terms of the BSD License. Refer to the shawn-licence.txt **
 ** file in the root of the Shawn source tree for further details.     **
 ************************************************************************
 **                                                                    **
 ** \author Axel Wegener <wegener@itm.uni-luebeck.de>                  **
 ** \author Bjoern Hendriks <hendriks@ibr.cs.tu-bs.de>                 **
 **                                                                    **
 ************************************************************************/
#ifndef __SHAWN_APPS_TCPIP_STORAGE_H
#define __SHAWN_APPS_TCPIP_STORAGE_H

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

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>

namespace tcpip
{

class Storage
{

public:
	typedef std::vector<unsigned char> StorageType;

private:
	StorageType store;
	StorageType::const_iterator iter_;

	// sortation of bytes forwards or backwards?
	bool bigEndian_;

	/// Used in constructors to initialize local variables
	void init();

	/// Check if the next \p num bytes can be read safely
	void checkReadSafe(unsigned int num) const;
	/// Read a byte \em without validity check
	unsigned char readCharUnsafe();
	/// Write \p size elements of array \p begin according to endianess
	void writeByEndianess(const unsigned char * begin, unsigned int size);
	/// Read \p size elements into \p array according to endianess
	void readByEndianess(unsigned char * array, int size);


public:

	/// Standard Constructor
	Storage();

	/// Constructor, that fills the storage with an char array. If length is -1, the whole array is handed over
	Storage(const unsigned char[], int length=-1);

	// Destructor
	virtual ~Storage();

	virtual bool valid_pos();
	virtual unsigned int position() const;

	void reset();
	/// Dump storage content as series of hex values
	std::string hexDump() const;

	virtual unsigned char readChar();
	virtual void writeChar(unsigned char);

	virtual int readByte();
	virtual void writeByte(int);
//	virtual void writeByte(unsigned char);

	virtual int readUnsignedByte();
	virtual void writeUnsignedByte(int);

	virtual std::string readString();
	virtual void writeString(const std::string& s);

    virtual std::vector<std::string> readStringList();
    virtual void writeStringList(const std::vector<std::string> &s);

    virtual std::vector<double> readDoubleList();
    virtual void writeDoubleList(const std::vector<double> &s);

	virtual int readShort();
	virtual void writeShort(int);

	virtual int readInt();
	virtual void writeInt(int);

	virtual float readFloat();
	virtual void writeFloat( float );

	virtual double readDouble();
	virtual void writeDouble( double );

	virtual void writePacket(unsigned char* packet, int length);
    virtual void writePacket(const std::vector<unsigned char> &packet);

	virtual void writeStorage(tcpip::Storage& store);

	// Some enabled functions of the underlying std::list
	StorageType::size_type size() const { return store.size(); }

	StorageType::const_iterator begin() const { return store.begin(); }
	StorageType::const_iterator end() const { return store.end(); }

};

} // namespace tcpip

#endif // BUILD_TCPIP

#endif
/*-----------------------------------------------------------------------
 * Source  $Source: $
 * Version $Revision: 620 $
 * Date    $Date: 2011-07-08 17:39:10 +0200 (Fri, 08 Jul 2011) $
 *-----------------------------------------------------------------------
 * $Log: $
 *-----------------------------------------------------------------------*/
