

# arp 処理の実装について

## 概要

ARPモジュールは以下の処理を行う。

 1. ARPリプライを受け取ったら必要に応じてテーブルを更新する
 2. ARPリクエストが着たらそれに応じたARPリプライを返信する
 3. IPパケットを渡すとそのパケットが欲しいMACアドレスの名前解決を
   行い、ifnetモジュールに渡す。


## 1. ARPリプライを受け取ったら必要に応じてテーブルを更新する

処理の手順を示す。

```
void arp_module::proc_update_arptable(struct arphdr* ah, uint8_t port)
{
    arpentry newent(ah->psrc, ah->hwsrc);

    for (arpentry& ent : table[port].entrys) {
        if (ent.ip == newent.ip) {
            if (is_same(ent.mac, newent.mac)) {
                return;
            } else {
                ent.mac = newent.mac;
                return;
            }
        } else { /* ip isnt same */
            continue;
        }
    }
    table[port].entrys.push_back(newent);
}
```



## 2. ARPリクエストが着たらそれに応じたARPリプライを返信する

処理の手順を示す。

```
static struct rte_mbuf* alloc_reply_packet(struct arphdr* ah, uint8_t port)
{
	struct ether_addr mymac;
	memset(&mymac, 0, sizeof(mymac));

	bool macfound=false;
	ifnet& dev = dpdk::instance().devices[port];
	for (ifaddr& ifa : dev.addrs) {
		if (ifa.family == af_link) {
			mymac = ifa.raw.link;
			macfound = true;
		}
	}
	if (!macfound)
		throw slankdev::exception("address not found");


    dpdk& d = dpdk::instance();
	struct rte_mbuf* msg = rte::pktmbuf_alloc(d.get_mempool());
    msg->data_len = 64;
    msg->pkt_len  = 64;
	uint8_t* data = rte::pktmbuf_mtod<uint8_t*>(msg);
	struct ether_header* eh = reinterpret_cast<struct ether_header*>(data);
	eh->src = mymac;
	eh->dst = ah->hwsrc;
	eh->type = htons(0x0806);

	struct arphdr* rep_ah = reinterpret_cast<struct arphdr*>(data + sizeof(struct ether_header));
	rep_ah->hwtype = htons(1);
	rep_ah->ptype  = htons(0x0800);
	rep_ah->hwlen  = 6;
	rep_ah->plen   = 4;
	rep_ah->operation = htons(2); // 2 is reply
	rep_ah->hwsrc = eh->src;
	rep_ah->psrc  = ah->pdst;
	rep_ah->hwdst = eh->dst;
	rep_ah->pdst  = ah->psrc;

	return msg;
}

static bool is_request_to_me(struct arphdr* ah, uint8_t port)
{
	ifnet& dev = dpdk::instance().devices[port];
	for (ifaddr& ifa : dev.addrs) {
		if (ifa.family == af_inet && ifa.raw.in==ah->pdst)
			return true;
	}
	return false;
}

void arp_module::proc_arpreply(struct arphdr* ah, uint8_t port)
{
	if (is_request_to_me(ah, port)) {
		struct rte_mbuf* msg = alloc_reply_packet(ah, port);

		dpdk& d = dpdk::instance();
		d.devices[port].tx_push(msg);
	}
}
```



## 3. IPパケットを渡すとそのパケットが欲しいMACアドレスの名前解決を行い、ifnetモジュールに渡す。

BSDでいうとarpresolveの機能である。黄色い本に手順がかいてあるので、それにそって実装していく。

処理の手順を示す

 1. 送信先がbcastまたはmcastか確認 -> 今回はmcastはサポート外
 1. 既存の経路エントリの中にタイムアウトしていない完全な変換が存在しないかを確認
 1. 存在すれば経路エントリないのゲートウェイの値が層審査期へのリンクアドレス
 1. ARPリクエストを送信する。その間送信待ちをするパケットはARPエントリ待ちのキューに確保する

これはIPヘッダなどの用意が必要なため後回しにする。