#include "../include/fileParsers/XMLParser.h"

XMLParser::XMLParser(std::string fileName) :
	_XMLserialID(0), _fXMLFileName(fileName), _fDom() {}

XMLParser::~XMLParser() {
	this->clear();
}

void XMLParser::clear() {
	_fDom->release();
}

void XMLParser::init() {
	// get the factory
	Poco::XML::DOMParser parser;

	// filters white space nodes v1.4.2p
	parser.setFeature(parser.FEATURE_FILTER_WHITESPACE, true);

	// parse using builder to get fDom representation of the XML file
	_fDom = parser.parse(_fXMLFileName);
}

/**
 * parse the rules by input file
 *
 * @param manager reference to simulator
 */
void XMLParser::parseDocument(Manager &manager) {
	Poco::XML::Node *root = _fDom->getElementsByTagName("Policy")->item(0);
	Poco::XML::XMLString ruleID, policyID = root->attributes()->getNamedItem("ID")->getNodeValue();
	manager.setID(atoi(policyID.c_str()));
	std::cout << "Policy no. " << manager.getID() << endl << endl;	// Debug

	Poco::XML::NodeList* policyList = root->childNodes();
	for (uint node=0; node < policyList->length(); node++)
		if (policyList->item(node)->hasChildNodes())
			manager.insertRule(parseRule(node, policyList->item(node)));
}

/**
 * parse specific rule
 *
 * @param flowID a number of rule
 * @param rule an appropriate record
 * @return Rule* the parsed rule
 */
Rule* XMLParser::parseRule(int flowID, Poco::XML::Node* rule) {
	struct LinkProperties tLink = {flowID, "FORWARD", "", "", 0, "", 0};
	struct Metrics tMetrics = {std::numeric_limits<double>::infinity(), 0, 0, 0};

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
				attName = attNode->nodeName();
				value = attNode->getNodeValue();

				if (attName.compare("Flow") == 0) {	// TODO : checking
					tLink._flowID = atoi(value.c_str());
				}

				else if (attName.compare("Chain") == 0) {
					tLink._chain = value;	// TODO : Define cases
				}

				else if (attName.compare("Protocol") == 0) {
					if (value.empty())
						throw new Poco::Exception("Protocol isn't defined");
					else tLink._protocol = value;
				}
			}
		}

		else if (nameNode.compare("Source") == 0) {
/*			tHost = attributeMap->getNamedItem("IPAddress")->getNodeValue();
			tPort = attributeMap->getNamedItem("Port")->getNodeValue();
			tLink._src = Poco::Net::SocketAddress(tHost, attName);	*/

			value = attributeMap->getNamedItem("IPAddress")->getNodeValue();
			tLink._srcHost = (value.empty() ? "" : validate_add(value));

			value = attributeMap->getNamedItem("Port")->getNodeValue();
			tLink._srcPort = atoi(value.c_str());
			if (tLink._srcPort < 0)
				throw new Poco::Exception("Source Port isn't valid");	// TODO : handling Exception
		}

		else if (!nameNode.compare("Destination")) {
/*			tHost = attributeMap->getNamedItem("IPAddress")->getNodeValue();
			tPort = attributeMap->getNamedItem("Port")->getNodeValue();
			tLink._dst = Poco::Net::SocketAddress(tHost, tPort);	*/

			value = attributeMap->getNamedItem("IPAddress")->getNodeValue();
			tLink._dstHost = (value.empty() ? "" : validate_add(value));

			value = attributeMap->getNamedItem("Port")->getNodeValue();
			tLink._dstPort = atoi(value.c_str());
			if (tLink._dstPort < 0)
				throw new Poco::Exception("Destination Port isn't valid");	// TODO : handling Exception
		}

		if (!nameNode.compare("Metrics")) {
			for(uint att = 0; att < attributeMap->length(); att++) {
				attNode = attributeMap->item(att);
				nameNode = attNode->nodeName();
				value = attNode->getNodeValue();

				if (atof(value.c_str()) < 0)
					throw new Poco::Exception(nameNode + " isn't valid");

				if (nameNode.compare("Throughput") == 0 && value.compare("inf") != 0)
						tMetrics._maxThroughput = atof(value.c_str())/8;

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

std::string XMLParser::validate_add(std::string add) {
	Poco::StringTokenizer tAdd(add, ",");
	Poco::StringTokenizer::Iterator numsIt = tAdd.begin();

	if (tAdd.count() != 4)
		throw new Poco::Exception("IP Address isn't valid");	// TODO : handling Exception

	for (; numsIt != tAdd.end(); numsIt++)
		if (0 <= atoi((*numsIt).c_str()) && atoi((*numsIt).c_str()) <= 255)
			throw new Poco::Exception("IP Address isn't valid");	// TODO : handling Exception

	return add;
}
