
#include "ap_nodeS.h"

APNodeS::APNodeS(PredicateNode *pn, vector<bdd> *ap_list) {
    this->in_port = pn->in_port;
    this->out_ports = pn->out_ports;
    this->type = NUM_SET;
    this->match = new set< uint64_t >;
    for (uint64_t i = 0; i < ap_list->size(); i++) {
        if (((*ap_list)[i] & pn->predicate) != bddfalse) {
            match->insert(i);
        }
    }
}

APNodeS::~APNodeS() {
    if (match != NULL) {
        delete match;
        match = NULL;
    }
}

void APNodeS::print_aps_node() {
    printf("Outports: %s\n", list_to_string(this->out_ports).c_str());
    cout << "Match set: (";
    for (auto it = match->begin(); it != match->end(); it++) {
        cout << *it << ", ";
    }
    cout << ")" << endl;
}