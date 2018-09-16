#ifndef XMLPARSER_H_
#define XMLPARSER_H_

#include <istream>
#include <vector>
#include <iostream>
#include <limits>

#include <Poco/DOM/AutoPtr.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/NodeIterator.h>
#include <Poco/DOM/NodeFilter.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Node.h>
#include <Poco/DOM/ElementsByTagNameList.h>
#include <Poco/DOM/NamedNodeMap.h>
#include <Poco/DOM/DocumentFragment.h>
#include "Poco/DOM/Text.h"
#include "Poco/DOM/DOMWriter.h"

#include <Poco/SAX/InputSource.h>
#include "Poco/XML/XMLWriter.h"

#include "../Rule.h"
#include "../Manager.h"

using namespace Poco::XML;
using namespace std;

class XMLParser {

private:
	int _XMLserialID;
	std::string _fXMLFileName;
	Poco::XML::AutoPtr<Poco::XML::Document> _fDom;

public:
	XMLParser(std::string fileName);
	~XMLParser();

	void clear();
	void init();
	void parseDocument(Manager &manager);
	Rule* parseRule(int flowID, Poco::XML::Node* node);

	std::string validate_add(std::string add);
};

#endif /* XMLPARSER_H_ */
