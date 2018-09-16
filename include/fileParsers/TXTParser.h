#ifndef TXTPARSER_H_
#define TXTPARSER_H_

#include <stdio.h>      /* printf */
#include <stdlib.h>     /* system, NULL, EXIT_FAILURE */
#include <iostream>
#include <fstream>
#include <string.h>
#include <limits>

#include "../Rule.h"
#include "../Manager.h"

using namespace std;

class TXTParser {

private:
	std::string _fTXTFileName;
	std::ifstream _config;

public:
	TXTParser(std::string fileName);
	~TXTParser();

	void clear();
	int init();
	void parseDocument(Manager &manager);
	Rule* parseRule(int flowID, std::string *line) throw();

};

#endif /* TXTPARSER_H_ */
