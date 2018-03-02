
#ifndef AP_VERIFIER_PREDICATE_NODE_H
#define AP_VERIFIER_PREDICATE_NODE_H

#include <bdd.h>
#include "ap_verifier_utils.h"
using namespace std;

class PredicateNode {
public:
    bdd predicate;
    struct List_t out_ports;
    uint64_t in_port;
    PREDICATE_TYPE type;

    PredicateNode(uint64_t in_port, string match, PREDICATE_TYPE type, int hdr_len);

    ~PredicateNode();

    void print_predicate_node();
    string to_string();
};

bdd match2bdd(string match, int length);
void allsatPrintHandler(char* varset, int size);

#endif //AP_VERIFIER_PREDICATE_NODE_H
