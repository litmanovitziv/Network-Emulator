/*
 * Manager.cpp
 *
 *  Created on: Sep 6, 2013
 *      Author: zivlit
 */

#include "../include/Manager.h"

Manager::Manager() : _serialID(-1), _isActive(false), _handler(0), _readerThread() {
	_rulesTable = new std::list<Rule*>();
//	_aggregatorsTable = new std::list<Aggregator*>();
}

Manager::~Manager() {
	if (_isActive == true)
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
}

/**
 * @param serialID number of policy
 */
void Manager::setID(int serialID) {
	_serialID = serialID;
}

int Manager::getID() {
	return _serialID;
}

bool Manager::isActive() {
	return _isActive;
}

bool Manager::isExist(int flowID) {
	list<Rule*>::iterator tableIt = _rulesTable->begin();
	while (tableIt!=_rulesTable->end()) {
		if ((*tableIt)->getFlow() == flowID)
			return true;
		tableIt++;
	}
	return false;
}

/**
 * insert a new rule into table
 *
 * @param Rule* new rule
 */
void Manager::insertRule(Rule *newRule) {
	_rulesTable->push_back(newRule);
	newRule->printData();
}

/**
 * start the emulator : initialize the dedicated queues and reader thread
 */
void Manager::start() {
	std::cout << "opening library handle" << endl << endl;
	_handler = nfq_open();
	if (!_handler) {
		std::cout << "error during nfq_open()" << endl << endl;
		throw new Poco::Exception("A netfilter queue connection handle can't open");
	}

	std::cout << "unbinding existing nf_queue handler (if any)" << endl << endl;
	if (nfq_unbind_pf(_handler, AF_INET) < 0) {
		std::cout << "error during nfq_unbind_pf()" << endl << endl;
		nfq_close(_handler);
		throw new Poco::Exception("Binding problem of a nfqueue handler");
	}

	std::cout << "binding nfnetlink_queue as nf_queue handler" << endl << endl;
	if (nfq_bind_pf(_handler, AF_INET) < 0) {
		std::cout << "error during nfq_bind_pf()" << endl << endl;
		nfq_close(_handler);
		throw new Poco::Exception("Binding problem of a nfqueue handler");
	}

	list<Rule*>::iterator rulesTableIt = _rulesTable->begin();
	while (rulesTableIt!=_rulesTable->end()) {
		try {
			(*rulesTableIt)->create(_handler);
		} catch (Poco::Exception* error) {
			std::cout << "Error : " << error->displayText() << std::endl;	// TODO : debugging
			// TODO : logging event
		}
		rulesTableIt++;
	}

	_isActive = true;
	_readerThread.start(*this);	// TODO : define recv() packets as thread
}

/**
 * Receiving the packet and routing them to appropriate queue
 */
void Manager::run(){
	int fd, rv;
	char buf[4096] __attribute__ ((aligned));

//	TODO : interrupt the simulator
//	signal(SIGQUIT, &sigstop_handler);
	fd = nfq_fd(_handler);

	std::cout << "Start Simulator" << endl;
	while (_isActive && (rv = recv(fd, buf, sizeof(buf), 0)) && rv > 0) {
	//	std::cout << "pkt received" << endl << endl;
		nfq_handle_packet(_handler, buf, rv);
		std::cout << "check" << buf << endl;
	}
	std::cout << "Stop Simulator" << endl;
}

void Manager::sigstop_handler(int signalID) {
	this->stop();
}

/**
 * stop the emulator : stop the reader thread and destroy the queues
 */
void Manager::stop() {
//	std::cout << "stopping simulator . . . " << endl;
	_isActive = false;
	send(nfq_fd(_handler), "", 0, 0);
	_readerThread.join();	// TODO : stop the thread

	list<Rule*>::iterator tableIt = _rulesTable->begin();
	while (tableIt!=_rulesTable->end()) {
		(*tableIt)->destroy();
		tableIt++;
	}

	/* normally, applications SHOULD NOT issue this command, since
	 * it detaches other programs/sockets from AF_INET, too ! */
	std::cout << "unbinding from AF_INET" << endl << endl;
	nfq_unbind_pf(_handler, AF_INET);

	std::cout << "closing library handle" << endl << endl;
	nfq_close(_handler);
}
