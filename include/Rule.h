/*
 * rule.h
 *
 *  Created on: Sep 6, 2013
 *      Author: zivlit
 */

#ifndef RULE_H_
#define RULE_H_

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>

#include <string>
#include <deque>

#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>		/* for NF_ACCEPT */
#include <libnetfilter_queue/libnetfilter_queue.h>

#include "Poco/Net/SocketAddress.h"

#include "../include/OutputManager.h"

#define NORMALLY_ACCEPT 0
#define MODIFIED_ACCEPT 1
#define DROP 2
#define CHANGE 3

using namespace std;

struct LinkProperties {
	int _flowID;
	std::string _chain;
	Poco::Net::SocketAddress _src;
	Poco::Net::SocketAddress _dst;
	std::string _protocol;

	std::string _srcHost;
	int _srcPort;
	std::string _dstHost;
	int _dstPort;
};

struct Metrics {
	double _maxThroughput;
	double _delay;
	double _loss_ratio;
	double _reorder_ratio;
};

class Rule {

	public:
		Rule(struct LinkProperties link, struct Metrics metrics);
		Rule& operator=(const Rule& sourceDetectObj);
		~Rule();
		void copy(const Rule& sourceDetObj);
		void clear();

		void setLinkProperties(struct LinkProperties link);
		void setMetrics(struct Metrics metrices);
		void setEgress(OutputManager* egress);
		struct nfq_q_handle *getQueue();

		u_int32_t print_pkt(struct nfq_data *tb);
		void printData();

		void create(struct nfq_handle* handler);
		void registerRule(std::string action);
		void destroy();

		int isPass();
		int isMaxLimitExceeded(struct timeval *timestamp, size_t size);
		int difftime(struct timeval *ts1, struct timeval *ts2);

		int _task(struct nfq_q_handle *qh,
						  struct nfgenmsg *nfmsg,
						  struct nfq_data *nfa,
						  void *data);

	private:
		struct LinkProperties _link;
		struct Metrics _metrices;

		struct nfq_q_handle *_qh;
//		std::deque<u_int32_t>* _reorderQ;
		OutputManager* _egress;

		size_t _counterSize;
		struct timeval *_ts;

		static int task(struct nfq_q_handle *qh,
						  struct nfgenmsg *nfmsg,
						  struct nfq_data *nfa,
						  void *data);
		u_int32_t treat_pkt(struct nfq_data *tb, u_int32_t *verdict);

};

#endif /* RULE_H_ */
