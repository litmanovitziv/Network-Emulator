#include "../include/fileParsers/TXTParser.h"

TXTParser::TXTParser(std::string fileName) : _fTXTFileName(fileName), _config() {} // ,_fileName(fileName) {}

TXTParser::~TXTParser() {
	this->clear();
}

void TXTParser::clear() {
	_config.close();
}

int TXTParser::init() {
	_config.open(_fTXTFileName.c_str(), std::ifstream::in);

	if (!_config.is_open()) {
		std::cerr << "Unable to open file" << endl;
		return -1;
	}

	else return 0;
}

void TXTParser::parseDocument(Manager &manager) {
	std::string line;
	if (!_config.eof()) {
		_config >> line;
		if (!line.compare("Policy")) {
			_config >> line;
			manager.setID(atoi(line.c_str()));
		}

		_config >> line;
		while (!line.compare("Rule"))
			manager.insertRule(parseRule(&line));
	}
}

Rule* TXTParser::parseRule(std::string *line) {
	std::string tHost, tPort;
	struct LinkProperties tLink;
	struct Metrics tMetrics;

	while (!_config.eof()) {
		_config >> *line;

		if (!line->compare("Rule"))
			break;

		else if (!line->compare("Flow")) {
			_config >> *line;
			tLink._flowID = atoi(line->c_str());
		}

		else if (!line->compare("Chain")) {
			_config >> *line;
			tLink._chain = *line;
		}

		else if (!line->compare("Source")) {
			_config >> *line;
			if (line->compare("all") != 0) {
				tLink._srcHost = *line;
				_config >> *line;
				tLink._srcPort = atoi(line->c_str());
			}
			else {
				tLink._srcHost = "";
				tLink._srcPort = 0;
			}
		}

		else if (!line->compare("Destination")) {
			_config >> *line;
			if (line->compare("all") != 0) {
				tLink._dstHost = *line;
				_config >> *line;
				tLink._dstPort = atoi(line->c_str());
			}
			else {
				tLink._dstHost = "";
				tLink._dstPort = 0;
			}
		}

		else if (line->compare("Chain") == 0) {
			_config >> *line;
			tLink._protocol = *line;
		}

		else if (line->compare("Protocol") == 0) {
			_config >> *line;
			if (line->compare("all") != 0)
				tLink._protocol = *line;
			else tLink._protocol = "";
		}

		else if (line->compare("Throughput") == 0) {
			_config >> *line;
			if (line->compare("inf") == 0)
				tMetrics._maxThroughput = std::numeric_limits<double>::infinity();
			else tMetrics._maxThroughput = atof(line->c_str())/8;
		}

		else if (line->compare("PacketLoss") == 0) {
			_config >> *line;
			tMetrics._loss_ratio = atof(line->c_str())/100;
		}

		else if (line->compare("Reordering") == 0) {
			_config >> *line;
			tMetrics._reorder_ratio = atof(line->c_str())/100;
		}

		else if (line->compare("Delay") == 0) {
			_config >> *line;
			tMetrics._delay = atof(line->c_str());
		}
	}

	return new Rule(tLink, tMetrics);
}
