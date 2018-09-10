/*
 * Manager.cpp
 *
 *  Created on: Sep 6, 2013
 *      Author: zivlit
 */

#include "../include/Manager.h"

Manager::Manager() : _serialID(-1), _isActive(false), _handler(0), _active(1, 16, 120, 0) {
	_rulesTable = new std::list<Rule*>();
	_aggregatorsTable = new std::list<Aggregator*>();
}

Manager::~Manager() {
	if (_isActive)
		stop();
	this->clear();
}

void Manager::clear() {
	delete _handler;

	list<Rule*>::iterator tableIt = _rulesTable->begin();
	while (tableIt!=_rulesTable->end()) {
		(*tableIt)->~Rule();
		tableIt++;
	}
	_rulesTable->clear();
	delete _rulesTable;

	delete _aggregatorsTable;
}

void Manager::setID(int serialID) {
	_serialID = serialID;
}

int Manager::getID() {
	return _serialID;
}

void Manager::insertFlow(Rule *newFlow) {
	OutputManager* tEgress = new OutputManager();
	_rulesTable->push_back(newFlow);
	newFlow->setEgress(tEgress);
	_aggregatorsTable->push_back(new Aggregator(tEgress));
	newFlow->printData(); // TODO : Debug
}

void Manager::start() {
	std::cout << "opening library handle" << endl << endl;
	_handler = nfq_open();
	if (!_handler) {
		std::cout << "error during nfq_open()" << endl << endl;
		exit(1);
	}

	std::cout << "unbinding existing nf_queue handler (if any)" << endl << endl;
	if (nfq_unbind_pf(_handler, AF_INET) < 0) {
		std::cout << "error during nfq_unbind_pf()" << endl << endl;
		exit(1);
	}

	std::cout << "binding nfnetlink_queue as nf_queue handler" << endl << endl;
	if (nfq_bind_pf(_handler, AF_INET) < 0) {
		std::cout << "error during nfq_bind_pf()" << endl << endl;
		exit(1);
	}

	_active.addCapacity(_rulesTable->size()-_active.capacity());

	list<Rule*>::iterator rulesTableIt = _rulesTable->begin();
	list<Aggregator*>::iterator aggTableIt = _aggregatorsTable->begin();
	while (rulesTableIt!=_rulesTable->end()) {
		(*rulesTableIt)->create(_handler);
		(*aggTableIt)->setQueue((*rulesTableIt)->getQueue());
		_active.start(**aggTableIt);	// TODO : start thread
		rulesTableIt++;
		aggTableIt++;
	}

	_isActive = true;
}

void Manager::process(){
	int fd, rv;
	char buf[4096] __attribute__ ((aligned));

	fd = nfq_fd(_handler);

	// TODO : interrupt the simulator
	while (_isActive && (rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
		std::cout << "pkt received" << endl << endl;
		nfq_handle_packet(_handler, buf, rv);
	}
}

void Manager::stop() {
	list<Rule*>::iterator tableIt = _rulesTable->begin();
	while (tableIt!=_rulesTable->end()) {
		(*tableIt)->destroy();
		tableIt++;
	}

	_active.joinAll();	// TODO : stop thread

	/* normally, applications SHOULD NOT issue this command, since
	 * it detaches other programs/sockets from AF_INET, too ! */
	std::cout << "unbinding from AF_INET" << endl << endl;
	nfq_unbind_pf(_handler, AF_INET);

	std::cout << "closing library handle" << endl << endl;
	nfq_close(_handler);

	_isActive = false;
}
