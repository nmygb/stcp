
#pragma once


#if 0
class ssnt_txrxwk : public ssn_thread {
    System* sys;
public:
    ssnt_txrxwk(System* s) : sys(s) {}
    void operator()()
    {
        const uint8_t nb_ports = sys->ports.size();
        for (;;) {
            for (uint8_t pid = 0; pid < nb_ports; pid++) {
                uint8_t nb_rxq = sys->ports[pid].rxq.size();
                uint8_t nb_txq = sys->ports[pid].txq.size();
                assert(nb_txq == nb_rxq);

                for (uint8_t qid=0; qid<nb_rxq; qid++) {
                    dpdk::Port& in_port  = sys->ports[pid];
                    dpdk::Port& out_port = sys->ports[pid^1];

                    in_port.rx_burst_allq();

#if 1
                    const size_t burst_size = 32;
                    rte_mbuf* pkts[burst_size];
                    bool ret = in_port.rxq[qid].pop_bulk(pkts, burst_size);
                    if (ret) out_port.txq[qid].push_bulk(pkts, burst_size);
#else
                    rte_mbuf* pkt;
                    bool ret = in_port.rxq[qid].pop(&pkt);
                    if (ret) out_port.txq[qid].push(pkt);
#endif

                    out_port.tx_burst_allq();
                }
            }
        }
    }
};
#endif


class ssnt_rx : public ssn_thread {
    System* sys;
public:
    ssnt_rx(System* s) : sys(s) {}
    void operator()()
    {
        const uint8_t nb_ports = sys->ports.size();
        for (;;) {
            for (uint8_t pid = 0; pid < nb_ports; pid++) {
                uint8_t nb_rxq = sys->ports[pid].rxq.size();
                for (uint8_t qid=0; qid<nb_rxq; qid++) {
                    sys->ports[pid].rx_burst_bulk(qid);
                }
            }
        }
    }
};



class ssnt_tx : public ssn_thread {
    System* sys;
public:
    ssnt_tx(System* s) : sys(s) {}
    void operator()()
    {
        const uint8_t nb_ports = sys->ports.size();
        for (;;) {
            for (uint8_t pid = 0; pid < nb_ports; pid++) {
                uint8_t nb_txq = sys->ports[pid].txq.size();
                for (uint8_t qid=0; qid<nb_txq; qid++) {
                    // printf("YUKARIKFDFDFDF\n");
                    sys->ports[pid].tx_burst_bulk(qid);
                }
            }
        }
    }
};


class ssnt_wk : public ssn_thread {
    System* sys;
public:
    ssnt_wk(System* s) : sys(s) {}
    void operator()()
    {
        const uint8_t nb_ports = sys->ports.size();
        for (;;) {
            for (uint8_t pid = 0; pid < nb_ports; pid++) {
                uint8_t nb_rxq = sys->ports[pid].rxq.size();
                uint8_t nb_txq = sys->ports[pid].txq.size();
                assert(nb_rxq == nb_txq);

                for (uint8_t qid=0; qid<nb_rxq; qid++) {
                    dpdk::Port& in_port  = sys->ports[pid];
                    dpdk::Port& out_port = sys->ports[pid^1];

                    const size_t burst_size = 32;
                    rte_mbuf* pkts[burst_size];
                    bool ret = in_port.rxq[qid].pop_bulk(pkts, burst_size);
                    if (ret) out_port.txq[qid].push_bulk(pkts, burst_size);
                }
            }
        }
    }
};

