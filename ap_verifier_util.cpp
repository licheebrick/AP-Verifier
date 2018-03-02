
#include <sstream>
#include <iostream>
#include "ap_verifier_utils.h"

int compare (const void * a, const void * b)
{
    return ( *(uint64_t*)a - *(uint64_t*)b );
}

List_t make_sorted_list_from_array (uint32_t count, uint64_t elems[]) {
    List_t result;
    result.size = count;
    result.shared = false;
    if (count > 0)
        result.list = (uint64_t *)malloc(count * sizeof(uint64_t));
    else
        result.list = NULL;
    for (uint32_t i = 0; i < count; i++) {
        result.list[i] = elems[i];
    }
    qsort(result.list, result.size, sizeof(uint64_t), compare);
    return result;
}

List_t val_to_list(const Json::Value &val) {
    uint64_t elems[val.size()];
    for (unsigned i = 0; i < val.size(); i++)
        elems[i] = val[i].asUInt();
    return make_sorted_list_from_array(val.size(),elems);
}

std::string list_to_string(List_t p) {
    std::stringstream result;
    result << "( ";
    for (uint32_t i = 0; i < p.size; i++) {
        result << p.list[i] << " ";
    }
    result << ")";
    return result.str();
}

void print_bool_vector(std::vector< bool > packet_header) {
    printf("[ ");
    for (std::vector< bool >::iterator it = packet_header.begin(); it != packet_header.end(); it++) {
        printf("%d, ", (*it)? 1:0);
    }
    printf("]\n");
}

void print_passed_port(std::list< uint64_t > passed_port) {
    printf("{ ");
    for (std::list< uint64_t >::iterator it = passed_port.begin(); it != passed_port.end(); it++) {
        printf("%lu => ", *it);
    }
    printf("}\n");
}