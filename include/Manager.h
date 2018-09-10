/*
 * Manager.h
 *
 *  Created on: Sep 10, 2013
 *      Author: zivlit
 */

#ifndef MANAGER_H_
#define MANAGER_H_

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <vector>
#include <list>

#include <unistd.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>		/* for NF_ACCEPT */
#include <libnetfilter_queue/libnetfilter_queue.h>

#include "Poco/ThreadPool.h"

#include "../include/Rule.h"
#include "../include/Aggregator.h"
// #include "../include/OutputManager.h"

using namespace std;

class Manager {
	public:
		Manager();
		Manager& operator=(const Manager& sourceDetectObj);
		~Manager();
		void copy(const Manager& sourceDetObj);
		void clear();

		// Life cycle
		void setID(int serialID);
		int getID();

		void insertFlow(Rule *newFlow);
		void start();
		void process();
		void stop();

	private:
		int _serialID;
		bool _isActive;
		struct nfq_handle *_handler;

		std::list<Rule*> *_rulesTable;
		std::list<Aggregator*> *_aggregatorsTable;
		Poco::ThreadPool _active;

};

#endif /* MANAGER_H_ */
