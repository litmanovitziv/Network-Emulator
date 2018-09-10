#ifndef XMLPARSER_H_
#define XMLPARSER_H_

#include <istream>
#include <vector>
#include <iostream>

#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/NodeIterator.h>
#include <Poco/DOM/NodeFilter.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Node.h>
#include <Poco/DOM/ElementsByTagNameList.h>
#include <Poco/DOM/NamedNodeMap.h>
#include <Poco/DOM/DocumentFragment.h>

#include "../Rule.h"
#include "../Manager.h"

using namespace std;

class XMLParser {

private:
	std::string _fXMLFileName;
	Poco::XML::AutoPtr<Poco::XML::Document> _fDom;

public:
	XMLParser(std::string fileName);
	~XMLParser();

	void clear();
	int init();
	void parseDocument(Manager &manager);
	Rule* parseRule(Poco::XML::Node* node);

};

#endif /* XMLPARSER_H_ */
