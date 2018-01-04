//
// Created by li danyang on 2017/12/31.
//
#include "router.h"

Router::Router(uint32_t router_id) :router_id(router_id){
    printf("Creating router %u...\n", router_id);
}

Router::~Router() {
    printf("Into ~Router()\n");

    // clear predicate_map
    std::map< uint32_t, std::map<Json::Value, PredicateNode*>* >::iterator pmit;
    for (pmit = predicate_map.begin(); pmit != predicate_map.end(); pmit++) {
        if (pmit->second != NULL) {
            std::map<Json::Value, PredicateNode*>::iterator it;
            for (it = pmit->second->begin(); it != pmit->second->end(); it++) {
                if (it->second != NULL) {
                    delete it->second;
                    it->second = NULL;
                }
            }
            pmit->second->clear();
            pmit->second = NULL;
        }
    }
    predicate_map.clear();

    // clear dealt_bdd_map
    dealt_bdd_map.clear();

    // clear ap_vec_map
    std::map< uint32_t, std::map<Json::Value, APNodeV*>* >::iterator avmit;
    for (avmit = ap_vec_map.begin(); avmit != ap_vec_map.end(); avmit++) {
        if (avmit->second != NULL) {
            std::map<Json::Value, APNodeV*>::iterator it;
            for (it = avmit->second->begin(); it != avmit->second->end(); it++) {
                if (it->second != NULL) {
                    delete it->second;
                    it->second = NULL;
                }
            }
            avmit->second->clear();
            avmit->second = NULL;
        }
    }
    ap_vec_map.clear();

    // clear ap_bset_map
    std::map< uint32_t, std::map<Json::Value, APNodeB*>* >::iterator abmit;
    for (abmit = ap_bset_map.begin(); abmit != ap_bset_map.end(); abmit++) {
        if (abmit->second != NULL) {
            std::map<Json::Value, APNodeB*>::iterator it;
            for (it = abmit->second->begin(); it != abmit->second->end(); it++) {
                if (it->second != NULL) {
                    delete it->second;
                    it->second = NULL;
                }
            }
            abmit->second->clear();
            abmit->second = NULL;
        }
    }
    ap_bset_map.clear();
}

void Router::print_router() {
    map<uint32_t, map<Json::Value, PredicateNode*>* >::iterator it;
    printf("Printing router %d...\n", this->router_id);
    for (it = predicate_map.begin(); it != predicate_map.end(); it++) {
        map<Json::Value, PredicateNode*>::iterator pit;
        printf("%s\n", string(40, '-').c_str());
        printf("Inport %d's predicate list:\n", (*it).first);
        for (pit = (*it).second->begin(); pit != (*it).second->end(); pit++) {
            (*pit).second->print_predicate_node();
        }
    }
    printf("%s\n", string(40, '-').c_str());
}

void Router::convert_to_ap(AP_TYPE type, vector< bdd >* ap_list) {
    map<uint32_t, map<Json::Value, PredicateNode*>* >::iterator it;
    switch (type) {
        case VECTOR: {
            for (it = predicate_map.begin(); it != predicate_map.end(); it++) {
                map<Json::Value, APNodeV *> *ap_map = new map<Json::Value, APNodeV *>;
                map<Json::Value, PredicateNode *>::iterator pit;
                for (pit = it->second->begin(); pit != it->second->end(); pit++) {
                    APNodeV *apv = new APNodeV(pit->second, ap_list);
                    ap_map->insert(make_pair(pit->first, apv));
                }
                ap_vec_map.insert(make_pair(it->first, ap_map));
            }
            break;
        }
        case NUM_SET:
            break;
        case BITSET: {
            for (it = predicate_map.begin(); it != predicate_map.end(); it++) {
                map<Json::Value, APNodeB *> *ap_map = new map<Json::Value, APNodeB *>;
                map<Json::Value, PredicateNode *>::iterator pit;
                for (pit = it->second->begin(); pit != it->second->end(); pit++) {
                    APNodeB *apb = new APNodeB(pit->second, ap_list);
                    ap_map->insert(make_pair(pit->first, apb));
                }
                ap_bset_map.insert(make_pair(it->first, ap_map));
            }
        }
        case NONE:
            break;
    }
}

void Router::print_router_apv_map() {
    map<uint32_t, map<Json::Value, APNodeV*>* >::iterator it;
    printf("Printing router %d's AP(vector version)...\n", this->router_id);
    for (it = ap_vec_map.begin(); it != ap_vec_map.end(); it++) {
        map<Json::Value, APNodeV*>::iterator pit;
        printf("%s\n", string(40, '-').c_str());
        printf("Inport %d's predicate list:\n", (*it).first);
        for (pit = (*it).second->begin(); pit != (*it).second->end(); pit++) {
            (*pit).second->print_apv_node();
        }
    }
    printf("%s\n", string(40, '-').c_str());
}

void Router::print_router_apb_map(){
    map<uint32_t, map<Json::Value, APNodeB*>* >::iterator it;
    printf("Printing router %d's AP(bitset version)...\n", this->router_id);
    for (it = ap_bset_map.begin(); it != ap_bset_map.end(); it++) {
        map<Json::Value, APNodeB*>::iterator pit;
        printf("%s\n", string(40, '-').c_str());
        printf("Inport %d's predicate list:\n", (*it).first);
        for (pit = (*it).second->begin(); pit != (*it).second->end(); pit++) {
            (*pit).second->print_apb_node();
        }
    }
    printf("%s\n", string(40, '-').c_str());
}

void Router::print_router_apn_map(){
    printf("Printing router %d's AP(number set version)...\n", this->router_id);
}

void Router::print_router_ap_map(AP_TYPE type) {
    switch (type) {
        case VECTOR:
            this->print_router_apv_map();
            break;
        case BITSET:
            this->print_router_apb_map();
            break;
        case NUM_SET:
            this->print_router_apn_map();
            break;
        case NONE:
            break;
        default:
            printf("Wrong AP_TYPE provide.\n");
    }
}
