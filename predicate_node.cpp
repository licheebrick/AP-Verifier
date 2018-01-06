//
// Created by li danyang on 2017/12/30.
//

#include "predicate_node.h"

PredicateNode::PredicateNode(uint32_t in_port, string match, PREDICATE_TYPE type, int hdr_len) {
    this->in_port = in_port;
    this->type = type;
    int match_len = 8;
    if (hdr_len != 1) {
        match_len = 8 * hdr_len + hdr_len - 1;
    }
    this->predicate = match2bdd(match, match_len);
}

PredicateNode::~PredicateNode(){

}

void PredicateNode::print_predicate_node() {
    printf("Outports: %s\n", list_to_string(this->out_ports).c_str());
    printf("Match packet header number: %lf\n", bdd_satcount(this->predicate));
    bdd_allsat(this->predicate, allsatPrintHandler);
}

string PredicateNode::to_string() {
    stringstream result;
    char buf[40];
    sprintf(buf, "%lf", bdd_satcount(this->predicate));
    result << "Outports: " << list_to_string(this->out_ports) << "\n";
    return result.str();
}

bdd match2bdd(string match, int length) {
    bdd result = bddtrue;
    int idx = 0;
    for (int i = 0; i < length; i++) {
        if (match[i] == 'x') {
            idx++;
            continue;
        } else {
            if (match[i] == '1') {
                result &= bdd_ithvar(idx);
                idx++;
            } else {
                if (match[i] == '0') {
                    result &= bdd_nithvar(idx);
                    idx++;
                }
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
