//
// Created by li danyang on 2017/12/31.
//
#include "router.h"

Router::Router(uint32_t router_id) :router_id(router_id){
    printf("Creating router %u...\n", router_id);
}

Router::~Router() {

}

void Router::print_router() {
    map<uint32_t, map<Json::Value, PredicateNode*> >::iterator it;
    printf("Printing router %d...\n", this->router_id);
    for (it = predicate_map.begin(); it != predicate_map.end(); it++) {
        map<Json::Value, PredicateNode*>::iterator pit;
        printf("%s\n", string(40, '-').c_str());
        printf("Inport %d's predicate list:\n", (*it).first);
        for (pit = (*it).second.begin(); pit != (*it).second.end(); pit++) {
            (*pit).second->print_predicate_node();
        }
    }
    printf("%s\n", string(40, '-').c_str());
}

void Router::convert_to_ap(AP_TYPE type, vector< bdd* >* ap_list) {
    map<uint32_t, map<Json::Value, PredicateNode*> >::iterator it;
    switch (type) {
        case VECTOR:
            for (it = predicate_map.begin(); it != predicate_map.end(); it++) {
                map<Json::Value, APNodeV*> ap_map;
                map<Json::Value, PredicateNode*>::iterator pit;
                for (pit = it->second.begin(); pit != it->second.end(); pit++){
                    APNodeV* apv = new APNodeV(pit->second, ap_list);
                    ap_map.insert(make_pair(pit->first, apv));
                }
                ap_vec_map.insert(make_pair(it->first, ap_map));
            }
            break;
        case NUM_SET:
            break;
        case BITSET:
            break;
    }
}