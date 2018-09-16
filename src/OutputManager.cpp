/*
 * OutputManager.cpp
 *
 *  Created on: Oct 2, 2013
 *      Author: zivlit
 */

#include "../include/OutputManager.h"
using namespace std;

OutputManager::OutputManager() :
		_QRdWrMutex(1, 1), _QisEmpty(0, 1000),	_qh(0), _egressQ(), _thread(), _isActive(false) {
	_thread.setPriority(Thread::PRIO_HIGHEST);
}

OutputManager::~OutputManager() {
	if (_isActive)
		stop();
	this->clear();
}

void OutputManager::clear() {
	struct pkt_data *pkt;
	while (!_egressQ.empty()){
		pkt = _egressQ.front();
		nfq_set_verdict(_qh, pkt->_id, NF_ACCEPT, 0, NULL);
		_egressQ.pop();
		free(pkt);
	}
}

/**
 * start the thread
 *
 * @param struct nfq_q_handle netfilter queue
 */
void OutputManager::start(struct nfq_q_handle *qh) {
	_qh = qh;
	_isActive = true;
	_thread.start(*this);
}

/**
 * stop the thread
 */
void OutputManager::stop() {
	_isActive = false;
	_thread.join();
	_qh = 0;
}

/**
 * insert packet into egress queue
 *
 * @param struct pkt_data* packet structure
 */
void OutputManager::insert(struct pkt_data* pkt) {
	_QRdWrMutex.wait();
	_egressQ.push(pkt);
	_QRdWrMutex.set();
	_QisEmpty.set();
}

/**
 * retrieve packet from egress queue
 *
 * @return struct pkt_data* packet structure
 */
struct pkt_data* OutputManager::remove() {
	_QisEmpty.wait();
	_QRdWrMutex.wait();
//	PktQ::iterator pkt = _egressQ.erase(_egressQ.begin());
	_QRdWrMutex.set();
	return 0;	// (*pkt);
}

/**
 * perform a delay
 *
 * @param struct timeval output timestamp
 */
void OutputManager::delay(struct timeval output_ts) {
	struct timeval curr_time, delay_time;
	memset(&delay_time, 0, sizeof(struct timeval));
	gettimeofday(&curr_time,NULL);
	if (timercmp(&output_ts, &curr_time, >) > 0) {
		// std::cout << "output timestamp : "; printTS(output_ts);
		// std::cout << "current system timestamp : "; printTS(curr_time);
		// delay the packet for the difference between output_ts and curr_time
		timersub(&output_ts, &curr_time, &delay_time);
		// std::cout << "delay timestamp : "; printTS(delay_time);
		select(0, 0, 0, 0, &delay_time);
		// std::cout << "select result : " << ansD << endl;
		// gettimeofday(&curr_time,NULL);
		// std::cout << "current system timestamp : "; printTS(curr_time);
	}
}

/**
 * log the timestamp
 * @param struct timeval output timestamp
 */
void OutputManager::printTS(struct timeval ts) {
	std::cout << ts.tv_sec << " second, "
			<< ts.tv_usec << " microsecond" << endl;
}

void OutputManager::run() {
	struct pkt_data *pkt;

	std::cout << "Start Output Manager" << endl;
	while (_isActive) {
//		std::cout << "Output: entering . . . " << endl;
		_QisEmpty.wait();
//		std::cout << "Output: after wait" << endl;
		pkt = _egressQ.front();
//		std::cout << "Output: packet " << pkt->_id << " : retrieve pkt" << endl;
//		pkt = remove();
//		printTS(pkt->_outTS);
		delay(pkt->_outTS);
//		std::cout << "Output: packet " << pkt->_id << " : delay pkt" << endl;
		std::cout << "Output: packet " << pkt->_id << " is sending now : " << endl;
		printTS(pkt->_outTS);
		nfq_set_verdict(_qh, pkt->_id, NF_ACCEPT, 0, NULL);
//		std::cout << "Output: packet " << pkt->_id << " : verdict pkt" << endl;
		_QRdWrMutex.wait();
		_egressQ.pop();
		_QRdWrMutex.set();
//		std::cout << "Output: packet " << pkt->_id << " : pop pkt" << endl;
		free(pkt);
//		std::cout << "Output: free pkt" << endl;
//		std::cout << endl;
	}
	std::cout << "Stop Output Manager" << endl;
}
