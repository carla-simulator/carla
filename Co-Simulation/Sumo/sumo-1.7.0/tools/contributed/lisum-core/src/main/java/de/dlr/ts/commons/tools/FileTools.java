/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    FileTools.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.tools;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class FileTools {

    final static Charset ENCODING = StandardCharsets.UTF_8;

    /**
     *
     */
    private FileTools() {
    }

    /**
     *
     * @param source
     * @param dest
     * @throws IOException
     */
    public static void copyFile(File source, File dest) throws IOException {
        InputStream is = null;
        OutputStream os = null;
        try {
            is = new FileInputStream(source);
            os = new FileOutputStream(dest);
            byte[] buffer = new byte[1024];
            int length;
            while ((length = is.read(buffer)) > 0) {
                os.write(buffer, 0, length);
            }
        } finally {
            is.close();
            os.close();
        }
    }

    /**
     * Creates a file in the hard disc and its directories. For example
     * "c:/hello/bye/file.txt" will create the directory hello, then the
     * directory bye and finally file.txt
     *
     * @param path the file path
     * @return <code>true</code> if the named file does not exist and was
     * successfully created; <code>false</code> if the named file already exists
     *
     * @throws java.io.IOException SecurityException If a security manager
     * exists and its <code>{@link
     * java.lang.SecurityManager#checkWrite(java.lang.String)}</code> method
     * denies write access to the file
     */
    public static boolean createFile(String path) throws IOException {
        createDirectories(path);

        File file = new File(path);
        boolean cc = file.createNewFile();

        return cc;
    }

    /**
     *
     * @param f
     * @throws IOException
     */
    public static void delete (File f) throws IOException {
        if (f.isDirectory()) {
            for (File c : f.listFiles()) {
                delete (c);
            }
        }
        if (!f.delete()) {
            throw new FileNotFoundException("Failed to delete file: " + f);
        }
    }

    /**
     * Creates the given directories ignoring the file in the path. For example,
     * "c:/hello/bye/file.txt" will create the directories hello and bye
     * ignoring file.txt.
     *
     * @param path A directories path
     */
    public static void createDirectories(String path) {
        File kk = new File(path);
        String parent = kk.getParent();

        File file = new File(parent);
        file.mkdirs();
    }

    /**
     * Deletes leading and/or trailing <b>/</b> or <b>\\</b> characters from the
     * directory name
     *
     * @param dir
     * @return
     */
    public static String cleanDirectoryName(String dir) {
        if (dir == null) {
            return null;
        }

        if (dir.endsWith("/") || dir.endsWith("\\")) {
            dir = dir.substring(0, dir.length() - 1);
        }

        if (dir.startsWith("/") || dir.startsWith("\\")) {
            dir = dir.substring(1, dir.length());
        }

        return dir;
    }


    /**
     *
     * @param fileName
     * @param lines
     * @throws IOException
     */
    public static void appendToTextFile(String fileName, String... lines) throws IOException {
        File file = new File(fileName);

        if (!file.exists()) {
            createFile(fileName);
        }

        FileWriter fw = new FileWriter(file, true);

        for (String line : lines) {
            fw.append(line);
            fw.append(System.lineSeparator());
        }

        fw.close();
    }

    /**
     *
     * @param fileName
     * @return
     */
    public static byte[] readBinaryFile(String fileName) {
        try {
            File file = new File(fileName);
            byte[] bytes;

            InputStream insputStream;
            insputStream = new FileInputStream(file);
            long length = file.length();
            bytes = new byte[(int) length];
            insputStream.read(bytes);

            return bytes;
        } catch (Exception e) {
            Logger.getLogger(FileTools.class.getName()).log(Level.SEVERE, null, e);
        }

        return null;
    }

    /**
     *
     * @param fileName
     * @param data
     */
    public static void writeBinaryFile(String fileName, byte[] data) {
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(fileName);
            fos.write(data, 0, data.length);
            fos.flush();
            fos.close();
        } catch (FileNotFoundException ex) {
            Logger.getLogger(FileTools.class.getName()).log(Level.SEVERE, null, ex);
        } catch (IOException ex) {
            Logger.getLogger(FileTools.class.getName()).log(Level.SEVERE, null, ex);
        } finally {
            try {
                fos.close();
            } catch (IOException ex) {
                Logger.getLogger(FileTools.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }

    /**
     *
     * @param fileName
     * @return
     * @throws IOException
     * @throws java.nio.file.NoSuchFileException
     */
    public static List<String> readSmallTextFile(String fileName) throws IOException {
        Path path = Paths.get(fileName);
        return Files.readAllLines(path, ENCODING);
    }

    /**
     *
     * @param fileName
     * @param lines
     * @throws IOException
     */
    public static void writeSmallTextFile(String fileName, List<String> lines) throws IOException {
        Path path = Paths.get(fileName);
        Files.write(path, lines, ENCODING);
    }

    /**
     *
     * @param fileName
     * @param text
     * @throws IOException
     */
    public static void writeSmallTextFile(String fileName, String text) throws IOException {
        List<String> lines = new ArrayList<String>();
        lines.addAll(Arrays.asList(text.split("\n")));

        Path path = Paths.get(fileName);
        Files.write(path, lines, ENCODING);
    }

    /**
     *
     * @param fileName
     * @param lines
     * @throws IOException
     */
    public static void writeLargerTextFile(String fileName, List<String> lines) throws IOException {
        Path path = Paths.get(fileName);

        BufferedWriter writer = Files.newBufferedWriter(path, ENCODING);

        for (String line : lines) {
            writer.write(line);
            writer.newLine();
        }

        writer.close();
    }

    /**
     *
     * @param fileName
     * @return
     */
    public static String getFolder(String fileName) {
        File file = new File(fileName);
        file = new File(file.getAbsolutePath());

        return file.getParent();
    }
}
