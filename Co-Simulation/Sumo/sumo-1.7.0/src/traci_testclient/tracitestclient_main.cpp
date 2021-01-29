/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
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
/// @file    tracitestclient_main.cpp
/// @author  Friedemann Wesner
/// @author  Axel Wegener
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @date    2008/04/07
///
// Main method for TraCITestClient
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <string>
#include <cstdlib>
#include "TraCITestClient.h"


// ===========================================================================
// method definitions
// ===========================================================================
int main(int argc, char* argv[]) {
    std::string defFile = "";
    std::string outFileName = "testclient_out.txt";
    int port = -1;
    std::string host = "localhost";

    if ((argc == 1) || (argc % 2 == 0)) {
        std::cout << "Usage: TraCITestClient -def <definition_file>  -p <remote port>"
                  << "[-h <remote host>] [-o <outputfile name>]" << std::endl;
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.compare("-def") == 0) {
            defFile = argv[i + 1];
            i++;
        } else if (arg.compare("-o") == 0) {
            outFileName = argv[i + 1];
            i++;
        } else if (arg.compare("-p") == 0) {
            port = atoi(argv[i + 1]);
            i++;
        } else if (arg.compare("-h") == 0) {
            host = argv[i + 1];
            i++;
        } else {
            std::cerr << "unknown parameter: " << argv[i] << std::endl;
            return 1;
        }
    }

    if (port == -1) {
        std::cerr << "Missing port" << std::endl;
        return 1;
    }
    if (defFile.compare("") == 0) {
        std::cerr << "Missing definition file" << std::endl;
        return 1;
    }

    try {
        TraCITestClient client(outFileName);
        return client.run(defFile, port, host);
    } catch (tcpip::SocketException& e) {
        std::cerr << "Socket error running the test client: " << e.what();
        return 1;
    } catch (libsumo::TraCIException& e) {
        std::cerr << "TraCI error running the test client: " << e.what();
        return 1;
    }
}
