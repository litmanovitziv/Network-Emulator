/*
 * Manager.h
 *
 *  Created on: Sep 10, 2013
 *      Author: zivlit
 */

#ifndef MANAGER_H_
#define MANAGER_H_

#include <unistd.h>	/* standard unix functions, like getpid() */
#include <sys/types.h>	/* various type definitions, like pid_t */
#include <stdlib.h>
#include <iostream>
#include <istream>

#include <math.h>
#include <string>
#include <signal.h>
#include <sys/select.h>

#include <vector>
#include <list>

#include <unistd.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>		/* for NF_ACCEPT */
#include <libnetfilter_queue/libnetfilter_queue.h>

#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/Task.h"
#include "Poco/SAX/InputSource.h"

#include "../include/Rule.h"

using namespace std;

class Manager : public Poco::Runnable {

	public:
		Manager();
		Manager& operator=(const Manager& sourceDetectObj);
		~Manager();
		void copy(const Manager& sourceDetObj);
		void clear();

		void setID(int serialID);
		void sigstop_handler(int signalID);
		int getID();
		bool isActive();
		bool isExist(int flowID);

		// Life cycle
		void insertRule(Rule *newRule);
		void start();
		void run();
		void stop();

	private:
		int _serialID;
		bool _isActive;

		struct nfq_handle *_handler;
		std::list<Rule*> *_rulesTable;
		Poco::Thread _readerThread;

};

#endif /* MANAGER_H_ */
