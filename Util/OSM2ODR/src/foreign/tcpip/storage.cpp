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

#include "storage.h"

#ifdef BUILD_TCPIP

#include <iostream>
#include <iterator>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <iomanip>


//#define NULLITER static_cast<list<unsigned char>::iterator>(0)

namespace tcpip
{

	// ----------------------------------------------------------------------
	Storage::Storage()
	{
		init();
	}


	// ----------------------------------------------------------------------
	Storage::Storage(const unsigned char packet[], int length)
	{
		assert(length >= 0); // fixed MB, 2015-04-21

		store.reserve(length);
		// Get the content
		for(int i = 0; i < length; ++i) store.push_back(packet[i]);

		init();
	}


	// ----------------------------------------------------------------------
	void Storage::init()
	{
		// Initialize local variables
		iter_ = store.begin();

		short a = 0x0102;
		unsigned char *p_a = reinterpret_cast<unsigned char*>(&a);
		bigEndian_ = (p_a[0] == 0x01); // big endian?
	}


	// ----------------------------------------------------------------------
	Storage::~Storage()
	{}


	// ----------------------------------------------------------------------
	bool Storage::valid_pos()
	{
		return (iter_ != store.end());   // this implies !store.empty()
	}


	// ----------------------------------------------------------------------
	unsigned int Storage::position() const
	{
		// According to C++ standard std::distance will simply compute the iterators
		// difference for random access iterators as std::vector provides.
		return static_cast<unsigned int>(std::distance(store.begin(), iter_));
	}


	// ----------------------------------------------------------------------
	void Storage::reset()
	{
		store.clear();
		iter_ = store.begin();
	}


	// ----------------------------------------------------------------------
	/**
	* Reads a char form the array
	* @return The read char (between 0 and 255)
	*/
	unsigned char Storage::readChar()
	{
		if ( !valid_pos() )
		{
			throw std::invalid_argument("Storage::readChar(): invalid position");
		}
		return readCharUnsafe();
	}


	// ----------------------------------------------------------------------
	/**
	*
	*/
	void Storage::writeChar(unsigned char value)
	{
		store.push_back(value);
		iter_ = store.begin();
	}


	// ----------------------------------------------------------------------
	/**
	* Reads a byte form the array
	* @return The read byte (between -128 and 127)
	*/
	int Storage::readByte()
	{
		int i = static_cast<int>(readChar());
		if (i < 128) return i;
		else return (i - 256);
	}


	// ----------------------------------------------------------------------
	/**
	*
	*/
	void Storage::writeByte(int value)
	{
		if (value < -128 || value > 127)
		{
			throw std::invalid_argument("Storage::writeByte(): Invalid value, not in [-128, 127]");
		}
		writeChar( static_cast<unsigned char>( (value+256) % 256 ) );
	}


	// ----------------------------------------------------------------------
	/**
	* Reads an unsigned byte form the array
	* @return The read byte (between 0 and 255)
	*/
	int Storage::readUnsignedByte()
	{
		return static_cast<int>(readChar());
	}


	// ----------------------------------------------------------------------
	/**
	*
	*/
	void Storage::writeUnsignedByte(int value)
	{
		if (value < 0 || value > 255)
		{
			throw std::invalid_argument("Storage::writeUnsignedByte(): Invalid value, not in [0, 255]");
		}
		writeChar( static_cast<unsigned char>( value ));
	}


	// -----------------------------------------------------------------------
	/**
	* Reads a string form the array
	* @return The read string
	*/
	std::string Storage::readString()
	{
		int len = readInt();
		checkReadSafe(len);
		StorageType::const_iterator end = iter_;
		std::advance(end, len);
		const std::string tmp(iter_, end);
		iter_ = end;
		return tmp;
	}


	// ----------------------------------------------------------------------
	/**
	* Writes a string into the array;
	* @param s		The string to be written
	*/
	void Storage::writeString(const std::string &s)
	{
		writeInt(static_cast<int>(s.length()));

		store.insert(store.end(), s.begin(), s.end());
		iter_ = store.begin();
	}


    // -----------------------------------------------------------------------
    /**
    * Reads a string list form the array
    * @return The read string
    */
    std::vector<std::string> Storage::readStringList()
    {
        std::vector<std::string> tmp;
        const int len = readInt();
        tmp.reserve(len);
        for (int i = 0; i < len; i++)
        {
            tmp.push_back(readString());
        }
        return tmp;
    }


    // -----------------------------------------------------------------------
    /**
    * Reads a double list from the array
    * @return The read double list
    */
    std::vector<double> Storage::readDoubleList()
    {
        std::vector<double> tmp;
        const int len = readInt();
        tmp.reserve(len);
        for (int i = 0; i < len; i++)
        {
            tmp.push_back(readDouble());
        }
        return tmp;
    }


    // ----------------------------------------------------------------------
    /**
    * Writes a string into the array;
    * @param s      The string to be written
    */
    void Storage::writeStringList(const std::vector<std::string> &s)
    {
        writeInt(static_cast<int>(s.size()));
        for (std::vector<std::string>::const_iterator it = s.begin(); it!=s.end() ; it++)
        {
            writeString(*it);
        }
    }


    // ----------------------------------------------------------------------
    /**
    * Writes a double list into the array;
    * @param s      The double list  to be written
    */
    void Storage::writeDoubleList(const std::vector<double> &s)
    {
        writeInt(static_cast<int>(s.size()));
        for (std::vector<double>::const_iterator it = s.begin(); it!=s.end() ; it++)
        {
            writeDouble(*it);
        }
    }


	// ----------------------------------------------------------------------
	/**
	* Restores an integer, which was split up in two bytes according to the
	* specification, it must have been split by its row byte representation
	* with MSBF-order
	*
	* @return the unspoiled integer value (between -32768 and 32767)
	*/
	int Storage::readShort()
	{
		short value = 0;
		unsigned char *p_value = reinterpret_cast<unsigned char*>(&value);
		readByEndianess(p_value, 2);
		return value;
	}


	// ----------------------------------------------------------------------
	void Storage::writeShort( int value )
	{
		if (value < -32768 || value > 32767)
		{
			throw std::invalid_argument("Storage::writeShort(): Invalid value, not in [-32768, 32767]");
		}

		short svalue = static_cast<short>(value);
		unsigned char *p_svalue = reinterpret_cast<unsigned char*>(&svalue);
		writeByEndianess(p_svalue, 2);
	}


	// ----------------------------------------------------------------------
	/**
	* restores an integer, which was split up in four bytes acording to the
	* specification, it must have been split by its row byte representation
	* with MSBF-order
	*
	* @return the unspoiled integer value (between -2.147.483.648 and 2.147.483.647)
	*/
	int Storage::readInt()
	{
		int value = 0;
		unsigned char *p_value = reinterpret_cast<unsigned char*>(&value);
		readByEndianess(p_value, 4);
		return value;
	}


	// ----------------------------------------------------------------------
	void Storage::writeInt( int value )
	{
		unsigned char *p_value = reinterpret_cast<unsigned char*>(&value);
		writeByEndianess(p_value, 4);
	}


	// ----------------------------------------------------------------------
	/**
	* restores a float , which was split up in four bytes acording to the
	* specification, it must have been split by its row byte representation
	* with MSBF-order
	*
	* @return the unspoiled float value
	*/
	float Storage::readFloat()
	{
		float value = 0;
		unsigned char *p_value = reinterpret_cast<unsigned char*>(&value);
		readByEndianess(p_value, 4);
		return value;
	}


	// ----------------------------------------------------------------------
	void Storage::writeFloat( float value )
	{
		unsigned char *p_value = reinterpret_cast<unsigned char*>(&value);
		writeByEndianess(p_value, 4);
	}


	// ----------------------------------------------------------------------
	void Storage::writeDouble( double value )
	{
		unsigned char *p_value = reinterpret_cast<unsigned char*>(&value);
		writeByEndianess(p_value, 8);
	}


	// ----------------------------------------------------------------------
	double Storage::readDouble( )
	{
		double value = 0;
		unsigned char *p_value = reinterpret_cast<unsigned char*>(&value);
		readByEndianess(p_value, 8);
		return value;
	}


	// ----------------------------------------------------------------------
	void Storage::writePacket(unsigned char* packet, int length)
	{
		store.insert(store.end(), &(packet[0]), &(packet[length]));
		iter_ = store.begin();   // reserve() invalidates iterators
	}


	// ----------------------------------------------------------------------
    void Storage::writePacket(const std::vector<unsigned char> &packet)
    {
        std::copy(packet.begin(), packet.end(), std::back_inserter(store));
		iter_ = store.begin();
    }


	// ----------------------------------------------------------------------
	void Storage::writeStorage(tcpip::Storage& other)
	{
		// the compiler cannot deduce to use a const_iterator as source
		store.insert<StorageType::const_iterator>(store.end(), other.iter_, other.store.end());
		iter_ = store.begin();
	}


	// ----------------------------------------------------------------------
	void Storage::checkReadSafe(unsigned int num) const 
	{
		if (std::distance(iter_, store.end()) < static_cast<int>(num))
		{
			std::ostringstream msg;
			msg << "tcpip::Storage::readIsSafe: want to read "  << num << " bytes from Storage, "
				<< "but only " << std::distance(iter_, store.end()) << " remaining";
			throw std::invalid_argument(msg.str());
		}
	}


	// ----------------------------------------------------------------------
	unsigned char Storage::readCharUnsafe()
	{
		char hb = *iter_;
		++iter_;
		return hb;
	}


	// ----------------------------------------------------------------------
	void Storage::writeByEndianess(const unsigned char * begin, unsigned int size)
	{
		const unsigned char * end = &(begin[size]);
		if (bigEndian_)
			store.insert(store.end(), begin, end);
		else
			store.insert(store.end(), std::reverse_iterator<const unsigned char *>(end), std::reverse_iterator<const unsigned char *>(begin));
		iter_ = store.begin();
	}


	// ----------------------------------------------------------------------
	void Storage::readByEndianess(unsigned char * array, int size)
	{
		checkReadSafe(size);
		if (bigEndian_)
		{
			for (int i = 0; i < size; ++i)
				array[i] = readCharUnsafe();
		}
		else
		{
			for (int i = size - 1; i >= 0; --i)
				array[i] = readCharUnsafe();
		}
	}


	// ----------------------------------------------------------------------
	std::string Storage::hexDump() const
	{
		static const int width = 2;

		std::ostringstream dump;
		// adapt stream attributes
		// 'showbase' inserts "0x", 'internal' makes leading '0' appear between "0x" and hex digits
		dump.setf(std::ostream::hex | std::ostream::showbase | std::ostream::internal);
		dump.fill('0');

		for(StorageType::const_iterator it = store.begin(); it != store.end(); ++it)
		{
			// insert spaces between values
			if (it != store.begin())
				dump << "  ";
			dump << std::setw(width) << static_cast<int>(*it);
		}

		return dump.str();
	}

}

#endif // BUILD_TCPIP

/*-----------------------------------------------------------------------
 * Source  $Source: $
 * Version $Revision: 620 $
 * Date    $Date: 2011-07-08 17:39:10 +0200 (Fri, 08 Jul 2011) $
 *-----------------------------------------------------------------------
 * $Log: $
 *-----------------------------------------------------------------------*/
