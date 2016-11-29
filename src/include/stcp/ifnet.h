
#pragma once

#include <stcp/socket.h>
#include <stcp/ifaddr.h>


namespace slank {




enum {
    BURST_SIZE = 64,
};


class ifnet {
private:
    pkt_queue rx;
    pkt_queue tx;
    uint16_t rx_packets;
    uint16_t tx_packets;
    uint16_t drop_packets;
    std::string name;

    /*
     * TODO use static variable or enum
     */
    uint8_t  port_id;
    uint16_t rx_ring_size;     /* rx ring size */
    uint16_t tx_ring_size;     /* tx ring size */
    uint16_t num_rx_rings;     /* num of rx_rings per port */
    uint16_t num_tx_rings;     /* num of tx_rings per port */

public:
    bool promiscuous_mode;
    std::vector<ifaddr> addrs;

    /*
     * TODO XXX can't set ring_size before constructor().
     */
    ifnet(uint8_t p) :
        rx_packets(0),
        tx_packets(0),
        drop_packets(0),

        port_id(p),
        rx_ring_size(128), // TODO fix
        tx_ring_size(512), // TODO fix
        num_rx_rings(1  ), // TODO fix
        num_tx_rings(1  ), // TODO fix
        promiscuous_mode(true)
    { name = "PORT" + std::to_string(port_id); }

    void init();
    uint16_t io_rx();
    uint16_t io_tx(size_t num_request_to_send);
    size_t rx_size() { return rx.size(); }
    size_t tx_size() { return tx.size(); }
    bool   rx_empty() { return rx.empty(); }
    bool   tx_empty() { return tx.empty(); }
    void print_stat() const;

    void ioctl(uint64_t request, void* arg);

private:
    void ioctl_siocsifaddr(const stcp_ifreq* ifr);
    void ioctl_siocgifaddr(stcp_ifreq* ifr);
    void ioctl_siocsifhwaddr(const stcp_ifreq* ifr);
    void ioctl_siocgifhwaddr(stcp_ifreq* ifr);
    void ioctl_siocsifnetmask(const stcp_ifreq* ifr);
    void ioctl_siocgifnetmask(stcp_ifreq* ifr);
    void ioctl_siocpromisc(const uint64_t* val);

public:
    void rx_push(mbuf* msg) { rx.push(msg); rx_packets++; }
    void tx_push(mbuf* msg) { tx.push(msg); tx_packets++; }
    mbuf* rx_pop() { return rx.pop(); }
    mbuf* tx_pop() { return tx.pop(); }
};




} /* namespace */
