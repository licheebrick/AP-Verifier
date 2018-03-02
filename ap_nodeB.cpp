
#include "ap_nodeB.h"

APNodeB::APNodeB(PredicateNode *pn, vector<bdd> *ap_list) {
    this->in_port = pn->in_port;
    this->out_ports = pn->out_ports;
    this->type = BITSET;
    for (int i = 0; i < ap_list->size(); i++) {
        if (((*ap_list)[i] & pn->predicate) != bddfalse) {
            match[i] = 1;
        }
    }
}

void APNodeB::print_apb_node() {
    printf("Outports: %s\n", list_to_string(this->out_ports).c_str());
    cout << "Match bitset: " << this->match << endl;
}