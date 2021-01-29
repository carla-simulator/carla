/*
 * Thanks to Rodney Thomson for this code
 * http://www.mathworks.com/matlabcentral/fileexchange/25249-tcp-ip-socket-communications-in-matlab-using-java-classes
 * This class improves the TraCI4Matlab performance when reading from the server.
 *
 * Copyright 2019 Universidad Nacional de Colombia,
 * Politecnico Jaime Isaza Cadavid.
 * Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
 * $Id: DataReader.java 48 2018-12-26 15:35:20Z afacostag $
 */

package co.edu.unalmed.gaunal.traci4matlab.utils;

import java.io.DataInput;
import java.io.EOFException;
import java.io.IOException;
import java.io.StreamCorruptedException;

/**
 *
 * @author GaunalJD
 */
public class DataReader {
    public DataReader(DataInput data_input)
    {
       m_data_input = data_input;
    }

    public byte[] readBuffer(int length) throws IOException
    {
       byte[] buffer = new byte[length];

       try
       {
           m_data_input.readFully(buffer, 0, length);
       }

       catch (StreamCorruptedException e)
       {
           System.out.println("Stream Corrupted Exception Occured");
           buffer = new byte[0];
       }
       catch (EOFException e)
       {
           System.out.println("EOF Reached");
           buffer = new byte[0];
       }
       catch (IOException e)
       {
           System.out.println("IO Exception Occured");
           buffer = new byte[0];
       }

       return buffer;
    }

    private DataInput m_data_input;
}
