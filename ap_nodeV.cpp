//
// Created by li danyang on 2018/1/2.
//

#include "ap_nodeV.h"

APNodeV::APNodeV(PredicateNode* pn, vector< bdd* >* ap_list) {
    this->in_port = pn->in_port;
    this->out_ports = pn->out_ports;
    this->type = VECTOR;
    this->match = new vector<bool>(ap_list->size(), false);
    for (int i = 0; i < ap_list->size(); i++) {
        if ((*(*ap_list)[i] & pn->predicate) != bddfalse) {
            (*match)[i] = true;
        }
    }
}