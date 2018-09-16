/*
 * Main.cpp
 *
 *  Created on: Sep 6, 2013
 *      Author: zivlit
 */

#include <stdio.h>      /* printf */
#include <stdlib.h>     /* system, NULL, EXIT_FAILURE */
#include <iostream>
#include <fstream>
#include <string.h>
#include <limits>

#include "Poco/Exception.h"

#include "../include/Manager.h"
#include "../include/fileParsers/XMLParser.h"
#include "../include/fileParsers/TXTParser.h"

using Poco::Exception;
using namespace std;

Manager* installPolicy(const char* fileName) {
	Manager *sim = new Manager();

	XMLParser xmlFile(fileName);
	xmlFile.init();
	xmlFile.parseDocument(*sim);

/*	TXTParser txtFile(fileName);
	if (txtFile.init() < 0) {
		std::cerr << "Unable to configure" << endl;
		exit(1);
	}
	txtFile.parseDocument(*sim);	*/

	return sim;
}

int main(int argc, char **argv) {
/*	if (system(NULL))
		puts("Ok");
	else exit (EXIT_FAILURE);
	system("echo lit0365L | sudo -v -S");
	std::cout << endl << endl;	*/
//	ITGSend -T UDP -a 127.0.0.1 -c 100 -C 10 -t 15000 -l sender.log -x receiver.log

	try {
		// Life cycle's Simulator
		Manager* Sim1 = installPolicy(argv[1]);
		Sim1->start();
		Sim1->run();
	//	Sim1->stop();
	//	Sim1->~Manager();
	}
    catch (Poco::Exception* error) {
        std::cout << "Error : " << error->displayText() << std::endl;
    }

    exit(0);
}
