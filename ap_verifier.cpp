//
// Created by li danyang on 2017/12/30.
//

#include "ap_verifier.h"
#include <vector>
#include <map>

APVerifier::APVerifier(int length) {
    this->length = length;
}

APVerifier::~APVerifier() {

}

void APVerifier::add_link(uint32_t from_port, uint32_t to_port) {
    if (topology.count(from_port) == 0) {
        topology[from_port] = new std::vector<uint32_t>(1, to_port);
    } else {
        topology[from_port]->push_back(to_port);
    }
}

void APVerifier::print_topology() {
    std::map< uint32_t, std::vector<uint32_t>* >::iterator it;
    for (it = topology.begin(); it != topology.end(); it++) {
        printf("%u --> ( ", (*it).first);
        for (size_t i = 0; i < (*it).second->size(); i++) {
            printf("%u ", (*it).second->at(i));
        }
        printf(")\n");
    }
}