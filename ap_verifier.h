//
// Created by li danyang on 2017/12/30.
//

#ifndef AP_VERIFIER_AP_VERIFIER_H
#define AP_VERIFIER_AP_VERIFIER_H
#include <map>
#include <json/json.h>
#include "predicate_node.h"
#include "router.h"
#include "log4cxx/logger.h"

class APVerifier {

public:
    // header length, in byte
    int hdr_len;

    AP_TYPE ap_type;

    uint32_t ap_size;

    // topology
    std::map< uint32_t, std::vector<uint32_t>* > topology;

    // router map: from router_id to predicate list;
    std::map< uint32_t, Router* > id_to_router;

    // router port map: from router_id to port list;
    // std::map< uint32_t, List_t> router_to_ports;

    // list to place atomic predicates represented in bdd;
    std::vector< bdd >* ap_bdd_list;

    // map from port_id to router_id
    std::map< uint32_t, uint32_t > inport_to_router;

    APVerifier(int length, AP_TYPE type);
    int get_length() { return this->hdr_len; }

    ~APVerifier();

    void add_link(uint32_t from_port, uint32_t to_port);

    void print_topology();

    string topology_to_string();

    long add_then_load_router(uint32_t router_id, Json::Value *root);

    void make_atomic_predicates();

    void convert_router_to_ap();

    void query_reachability(uint32_t from_port, uint32_t to_port);

    void propagate_bdd(bdd packet_header, std::list< uint32_t > passed_port, uint32_t from_port,
                       uint32_t dst_port);

    void propagate_vec(std::vector< bool > packet_header, std::list< uint32_t > passed_port, uint32_t from_port,
                   uint32_t dst_port);

    void propagate_bset(std::bitset<BITSETLEN> packet_header, std::list< uint32_t > passed_port, uint32_t from_port,
                       uint32_t dst_port);
};

bdd match2bdd(string match, int length);

#endif //AP_VERIFIER_AP_VERIFIER_H
