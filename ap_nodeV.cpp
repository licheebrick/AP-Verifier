//
// Created by li danyang on 2018/1/2.
//

#include "ap_nodeV.h"
#include "ap_verifier_utils.h"

APNodeV::APNodeV(PredicateNode* pn, vector< bdd >* ap_list) {
    this->in_port = pn->in_port;
    this->out_ports = pn->out_ports;
    this->type = VECTOR;
    this->match = new vector<bool>(ap_list->size(), false);
    for (int i = 0; i < ap_list->size(); i++) {
        if (((*ap_list)[i] & pn->predicate) != bddfalse) {
            (*match)[i] = true;
        }
    }
}

APNodeV::~APNodeV() {
    if (match != NULL) {
        delete match;
        match = NULL;
    }
}

void APNodeV::print_apv_node() {
    printf("Outports: %s\n", list_to_string(this->out_ports).c_str());
    printf("Match vector: ");
    print_bool_vector(*this->match);
}