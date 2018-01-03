//
// Created by li danyang on 2017/12/30.
//

#include "predicate_node.h"

PredicateNode::PredicateNode(uint32_t in_port, string match, PREDICATE_TYPE type, int hdr_len) {
    this->in_port = in_port;
    this->type = type;
    this->predicate = match2bdd(match, hdr_len);
}

PredicateNode::~PredicateNode(){

}

void PredicateNode::print_predicate_node() {
    printf("Outports: %s\n", list_to_string(this->out_ports).c_str());
    printf("Match packet header number: %lf\n", bdd_satcount(this->predicate));
    bdd_allsat(this->predicate, allsatPrintHandler);
}

bdd match2bdd(string match, int length) {
    bdd result = bddtrue;
    for (int i = 0; i < length; i++) {
        if (match[i] == 'x') {
            continue;
        } else {
            if (match[i] == '1') {
                result &= bdd_ithvar(i);
            } else {
                result &= bdd_nithvar(i);
            }
        }
    }
    return result;
}

void allsatPrintHandler(char* varset, int size)
{
    for (int v=0; v<size; ++v)
    {
        cout << (varset[v] < 0 ? 'X' : (char)('0' + varset[v]));
    }
    cout << endl;
}
