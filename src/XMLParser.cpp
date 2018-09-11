#include "../include/fileParsers/XMLParser.h"

XMLParser::XMLParser(std::string fileName) : _fXMLFileName(fileName) {}

XMLParser::~XMLParser() {
	_fDom->release();
}

int XMLParser::init(){
	//get the factory
	Poco::XML::DOMParser parser;

	// filters white space nodes v1.4.2p
	parser.setFeature(parser.FEATURE_FILTER_WHITESPACE, true);

	// parse using builder to get fDom representation of the XML file
	_fDom = parser.parse(_fXMLFileName);

	return 0;
}

void XMLParser::parseDocument(Manager &manager) {
	Poco::XML::Node *root = _fDom->getElementsByTagName("Policy")->item(0);
//	Poco::XML::XMLString serialID = root->attributes()->getNamedItem("ID")->getNodeValue();
//	manager.setID(atoi(serialID.c_str()));

	Poco::XML::NodeList* policyList = root->childNodes();
	for (uint node=0; node < policyList->length(); node++)
		manager.insertRule(parseRule(policyList->item(node)));
}

Rule* XMLParser::parseRule(Poco::XML::Node* rule) {
	struct LinkProperties tLink;;
	struct Metrics tMetrics;

	Poco::XML::NodeIterator ruleIterator(Poco::XML::NodeIterator(rule, Poco::XML::NodeFilter::SHOW_ALL));
	Poco::XML::Node *node, *attNode;
	Poco::XML::XMLString nameNode, attName, value, tHost, tPort;
	Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> attributeMap;

	node = ruleIterator.nextNode();
	while (node) {
		nameNode = node->nodeName();

		if (node->hasAttributes())
			attributeMap = node->attributes();

		if (nameNode.compare("LinkProperties") == 0) {
			for(uint att = 0; att < attributeMap->length(); att++) {
				attNode = attributeMap->item(att);
				nameNode = attNode->nodeName();
				value = attNode->getNodeValue();

				if (nameNode.compare("Flow") == 0)
					tLink._flowID = atoi(value.c_str());

				else if (nameNode.compare("Chain") == 0) {
					tLink._chain = value;	// TODO : Define cases
				}

				else if (nameNode.compare("Protocol") == 0) {
					if (value.compare("all") != 0)
						tLink._protocol = value;
					else tLink._protocol = "";
				}
			}
		}

		else if (nameNode.compare("Source") == 0) {
/*			tHost = attributeMap->getNamedItem("IPAddress")->getNodeValue();
			tPort = attributeMap->getNamedItem("Port")->getNodeValue();
			tLink._src = Poco::Net::SocketAddress(tHost, tPort);	*/

			value = attributeMap->getNamedItem("IPAddress")->getNodeValue();
			if (value.compare("all") != 0) {
				tLink._srcHost = value;
				value = attributeMap->getNamedItem("Port")->getNodeValue();
				tLink._srcPort = atoi(value.c_str());
			}
			else {
				tLink._srcHost = "";
				tLink._srcPort = 0;
			}
		}

		else if (!nameNode.compare("Destination")) {
/*			tHost = attributeMap->getNamedItem("IPAddress")->getNodeValue();
			tPort = attributeMap->getNamedItem("Port")->getNodeValue();
			tLink._dst = Poco::Net::SocketAddress(tHost, tPort);	*/

			value = attributeMap->getNamedItem("IPAddress")->getNodeValue();
			if (value.compare("all") != 0) {
				tLink._dstHost = value;
				value = attributeMap->getNamedItem("Port")->getNodeValue();
				tLink._dstPort = atoi(value.c_str());
			}
			else {
				tLink._srcHost = "";
				tLink._dstPort = 0;
			}
		}

		if (!nameNode.compare("Metrics")) {
			for(uint att = 0; att < attributeMap->length(); att++) {
				attNode = attributeMap->item(att);
				nameNode = attNode->nodeName();
				value = attNode->getNodeValue();

				if (nameNode.compare("Throughput") == 0) {
					if (value.compare("inf") == 0)
						tMetrics._maxThroughput = std::numeric_limits<double>::infinity();
					else tMetrics._maxThroughput = atof(value.c_str())/8;
				}

				else if (nameNode.compare("PacketLoss") == 0)
					tMetrics._loss_ratio = atof(value.c_str())/100;

				else if (nameNode.compare("Reordering") == 0)
					tMetrics._reorder_ratio = atof(value.c_str())/100;

				else if (nameNode.compare("Delay") == 0)
					tMetrics._delay = atof(value.c_str());
			}
		}

		node = ruleIterator.nextNode();
	}

	return new Rule(tLink, tMetrics);
}

/*
*/
