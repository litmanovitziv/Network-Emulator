/*
 * OutputManager.cpp
 *
 *  Created on: Oct 2, 2013
 *      Author: zivlit
 */

#include "../include/OutputManager.h"
using namespace std;

OutputManager::OutputManager() : _mutex1(1, 1), _mutex2(1, 1), _emptyQ(0, 1), _egressQ(), _reorderQ() {
//	std::cout << "OutputManager constructor" << endl;
}

OutputManager::~OutputManager() {
	this->clear();
}

void OutputManager::clear() {
/*	while (!_egressQ.empty()) {
		delete _egressQ.back();
		_egressQ.pop_back();
	}	*/

	PktQ::iterator pkt;
	for(pkt = _egressQ.begin(); pkt != _egressQ.end(); pkt++){
		delete *pkt; // TODO : check
		*pkt = 0;
	}
	_egressQ.clear();

	// TODO : destroy element and clear
	for(pkt = _reorderQ.begin(); pkt != _reorderQ.end(); pkt++){
		delete *pkt; // TODO : check
		*pkt = 0;
	}
	_reorderQ.clear();
}

void OutputManager::insert(struct pkt_data* pkt, std::string type) {
	if (type.compare("output") == 0)
		_egressQ.push_back(pkt);
	else _reorderQ.push_front(pkt);
}

struct pkt_data* OutputManager::remove(std::string type) {
	PktQ::iterator pkt;
	if (type.compare("output") == 0)
		pkt = _egressQ.erase(_egressQ.begin());
	else pkt = _reorderQ.erase(_reorderQ.begin());
	return (*pkt);
}

bool OutputManager::isEmpty(std::string type) {
	if (type.compare("output") == 0)
		return _egressQ.empty();
	else return _reorderQ.empty();
}
