/*
 * Rule.cpp
 *
 *  Created on: Sep 6, 2013
 *      Author: zivlit
 */

#include "../include/Rule.h"
using namespace std;

Rule::Rule(struct LinkProperties link, struct Metrics metrics) :
		_qh(0), _egress(0), _counterSize(0), _ts(0) {
//	memset(_ts, 0, sizeof(struct timeval));

	_link._flowID = link._flowID;
	_link._chain = link._chain;
	_link._protocol = link._protocol;
	_link._src = link._src;
	_link._dst = link._dst;
	_link._dstHost = link._dstHost;
	_link._dstPort = link._dstPort;
	_link._srcHost = link._srcHost;
	_link._srcPort = link._srcPort;

	_metrices._maxThroughput = metrics._maxThroughput;
	_metrices._delay = metrics._delay;
	_metrices._loss_ratio = metrics._loss_ratio;
	_metrices._reorder_ratio = metrics._reorder_ratio;
}

Rule::~Rule(){
	destroy();
	this->clear();
}

void Rule::clear() {
	delete _qh;
}

void Rule::setLinkProperties(struct LinkProperties link) {
	_link._flowID = link._flowID;
	_link._chain = link._chain;
	_link._protocol = link._protocol;
	_link._src = link._src;
	_link._dst = link._dst;
	_link._dstHost = link._dstHost;
	_link._dstPort = link._dstPort;
	_link._srcHost = link._srcHost;
	_link._srcPort = link._srcPort;
}

void Rule::setMetrics(struct Metrics metrics) {
	_metrices._maxThroughput = metrics._maxThroughput;
	_metrices._delay = metrics._delay;
	_metrices._loss_ratio = metrics._loss_ratio;
	_metrices._reorder_ratio = metrics._reorder_ratio;
}

void Rule::setEgress(OutputManager* egress) {
	_egress = egress;
}

struct nfq_q_handle *Rule::getQueue() {
	return _qh;
}


void Rule::create(struct nfq_handle* handler) {
	std::cout << "binding this socket to queue " << _link._flowID << endl << endl;
	if ((_qh = nfq_create_queue(handler, _link._flowID, &task, this)) < 0){ // the bug is here
		std::cout << "error during nfq_create_queue()" << endl << endl;
		exit(1);
	}

	std::cout << "setting copy_packet mode" << endl << endl;
	if (nfq_set_mode(_qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
		std::cout << "can't set packet_copy mode " << endl << endl;
		exit(1);
	}

	std::cout << "register this rule into iptables " << _link._flowID << endl << endl;
	registerRule("A");
	// TODO : start thread
}

void Rule::registerRule(std::string action) {
	std::ostringstream command, tmp1, tmp2;

	command << "iptables"
			<< " -" << action << (_link._chain.empty() ? " FORWARD" : " ") << _link._chain
			<< (_link._protocol.empty() ? "" : " -p ") << _link._protocol;

	command << (_link._srcHost.empty() ? "" : " -s ") << _link._srcHost;
	tmp1 << " --sport " << _link._srcPort;
	command	<< (_link._srcPort>0 ? tmp1.str().c_str() : "");

	command << (_link._dstHost.empty() ? "" : " -d ") << _link._dstHost;
	tmp2 << " --dport " << _link._dstPort;
	command	<< (_link._dstPort>0 ? tmp2.str().c_str() : "");

/*	command << (_link._src.host().toString().empty() ? "" : " -s ") << _link._src.host().toString();
	tmp1 << " --sport " << _link._src.port();
	command	<< (_link._src.port()>0 ? tmp1.str().c_str() : "");

	command << (_link._dst.host().toString().empty() ? "" : " -s ") << _link._dst.host().toString();
	tmp1 << " --dport " << _link._dst.port();
	command	<< (_link._dst.port()>0 ? tmp1.str().c_str() : "");	*/

	command << " -j NFQUEUE"
			<< " --queue-num " << _link._flowID;

	std::cout << command.str() << endl << endl;
	system(command.str().c_str());
	sleep(30);

//	sudo iptables -A OUTPUT -p icmp -j NFQUEUE --queue-num 0
//	sudo iptables -A INPUT -p tcp -s localhost -d localhost -dport 4444 -j NFQUEUE --queue-num 1
//	sudo iptables -A OUTPUT -p icmp -d 212.199.219.242 -j NFQUEUE --queue-num 0
//	sudo iptables -A INPUT -p tcp -s localhost -d localhost --dport 4444 -j NFQUEUE --queue-num 1
}

void Rule::destroy() {
	std::cout << "Unregister this rule into iptables " << _link._flowID << endl << endl;
	registerRule("D");
	// TODO : stop thread

	std::cout << "unbinding from queue " << 0 << endl << endl;
	nfq_destroy_queue(this->_qh);
}

int Rule::task(struct nfq_q_handle *qh,
				  struct nfgenmsg *nfmsg,
				  struct nfq_data *nfa,
				  void *data)
{
	Rule* rule = (Rule*)data;
	return rule->_task(qh,nfmsg,nfa,data);
}

int Rule::_task(struct nfq_q_handle *qh,
				  struct nfgenmsg *nfmsg,
				  struct nfq_data *nfa,
				  void *data)
{
	u_int32_t id, verdict, ans;
	std::cout << "Rule: entering callback, Flow: " << _link._flowID << endl << endl;

//	struct nfqnl_msg_packet_hdr *ph = nfq_get_msg_packet_hdr(nfa);
//	id = ntohl(ph->packet_id);
//	id = print_pkt(nfa);
	id = treat_pkt(nfa, &verdict);
	ans = nfq_set_verdict(qh, id, verdict, 0, NULL);
//	ans = nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
	return ans;
}

u_int32_t Rule::treat_pkt(struct nfq_data *tb, u_int32_t *verdict) {
//	u_int32_t id;
	char *data;
	size_t size;
	struct pkt_data *pkt = new struct pkt_data;
	struct nfqnl_msg_packet_hdr *ph = nfq_get_msg_packet_hdr(tb);
	pkt->_id = ntohl(ph->packet_id);
	std::cout << "Rule: entering callback, Flow: " << _link._flowID << ", packet: " << pkt->_id << endl << endl;

	// TODO : define delay
/*	struct timeval tTSin;
	nfq_get_timestamp(tb, &pkt->_outTS);
	tTSin.tv_sec = pkt->_outTS.tv_sec;
	tTSin.tv_usec = pkt->_outTS.tv_usec;

	int tTSinUsec = pkt->_outTS.tv_usec + int(_metrices._delay*1000L);
	pkt->_outTS.tv_usec = tTSinUsec%1000000L;
	pkt->_outTS.tv_sec += tTSinUsec/1000000L;	*/

	// packet loss
	if (rand()/(float)RAND_MAX < _metrices._loss_ratio)
		*verdict = NF_DROP;

	// throughput
//	else if ((size = nfq_get_payload(tb, &data)) >= 0)
//		*verdict = isMaxLimitExceeded(&pkt->_inTS, size);

	// packet reorder
	else if (rand()/(float)RAND_MAX < _metrices._reorder_ratio) {
		std::cout << "Rule: packet " << pkt->_id << " is queued" << endl << endl;
		_egress->_mutex2.wait();
		_egress->insert(pkt, "reorder");
		_egress->_mutex2.set();
		_egress->_emptyQ.set();
		*verdict = NF_STOLEN;
	}

	else {
		std::cout << "Rule: packet " << pkt->_id << " is sending now" << endl << endl;
		_egress->_mutex1.wait();
		_egress->insert(pkt, "output");
		_egress->_mutex1.set();
		_egress->_emptyQ.set();
		*verdict = NF_STOLEN;
//		*verdict = NF_ACCEPT;
	}

	return pkt->_id;
}

/** Verdict methods can return the following values :
 * 0 if the packet is to be accepted without modification,
 * 1 if the packet is to be accepted with modification(s),
 * 2 if the packet is to be changed.
 */

// Method that decides whether a packet should be dropped
int Rule::isPass() {
	if (rand()/(float)RAND_MAX < _metrices._loss_ratio)
		return NF_DROP;
	return NF_ACCEPT;
}

int Rule::isMaxLimitExceeded(struct timeval *timestamp, size_t size) {
	if (timercmp(timestamp, _ts, >) > 0) {
		_counterSize = 0;
		_ts = timestamp;
	}
	_counterSize += size;
	if (_counterSize > _metrices._maxThroughput)
		return NF_DROP;
	return NF_ACCEPT;
}

int Rule::difftime(struct timeval *ts1, struct timeval *ts2) {
	struct timeval *tTS = NULL;
	tTS->tv_sec = ts1->tv_sec - ts2->tv_sec;
	tTS->tv_usec = ts1->tv_usec - ts2->tv_usec;
	if (tTS->tv_sec == 0) {
		if (tTS->tv_usec == 0)
			return 0;
		else if (tTS->tv_usec > 0)
			return 1;
		else return -1;
	}
	else if (tTS->tv_sec > 0)
		return 1;
	else return -1;
}

void Rule::printData() {
//	std::cout << "Print rule" << endl;
	std::cout << "Flow no. " << _link._flowID << endl
			<< "Chain: " << _link._chain << endl
			<< "Source: Host " << _link._srcHost << ", Port " << _link._srcPort << endl
			<< "Destination: Host " << _link._dstHost << ", Port " << _link._dstPort << endl
//			<< "Source: " << _link._src.toString() << endl
//			<< "Destination: " << _link._dst.toString() << endl
			<< "Protocol: " << _link._protocol << endl;

	std::cout << "Flow metrics:" << endl
			<< "Max Throughput: "<< _metrices._maxThroughput << endl
			<< "Delay: " << _metrices._delay << endl
			<< "Packet Loss: " << _metrices._loss_ratio << endl
			<< "Reorder: " << _metrices._reorder_ratio << endl;

	std::cout << endl;
}

/* returns packet id */
u_int32_t Rule::print_pkt (struct nfq_data *tb) {
	int id = 0;
	struct nfqnl_msg_packet_hdr *ph;
	struct nfqnl_msg_packet_hw *hwph;
	u_int32_t mark,ifi;
	int ret;
	unsigned char *data;

	ph = nfq_get_msg_packet_hdr(tb);
	if (ph) {
		id = ntohl(ph->packet_id);
		std::cout << "hw_protocol=" << ntohs(ph->hw_protocol)
				<< " hook=" << ph->hook << "id=" << id << endl << endl;
	}

	hwph = nfq_get_packet_hw(tb);
	if (hwph) {
		int i, hlen = ntohs(hwph->hw_addrlen);

		for (i = 0; i < hlen-1; i++)
			std::cout << "hw_src_addr=" << hwph->hw_addr[i] << endl << endl;
		std::cout << "hw_src_addr=" << hwph->hw_addr[hlen-1] << endl << endl;
	}

	mark = nfq_get_nfmark(tb);
	if (mark)
		std::cout << "mark=" << mark << " ";

	ifi = nfq_get_indev(tb);
	if (ifi)
		std::cout << "indev=" << ifi << " ";

	ifi = nfq_get_outdev(tb);
	if (ifi)
		std::cout << "outdev=" << ifi << " ";

	ifi = nfq_get_physindev(tb);
	if (ifi)
		std::cout << "physindev=" << ifi << " ";

	ifi = nfq_get_physoutdev(tb);
	if (ifi)
		std::cout << "physoutdev=" << ifi << " ";

	ret = nfq_get_payload(tb, &data);
	if (ret >= 0)
		std::cout << "payload_len=" << ret << " ";

	std::cout << endl << endl;

	return id;
}
