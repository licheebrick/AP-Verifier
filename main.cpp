#include <iostream>
#include <bdd.h>
#include "json/json.h"
#include <fstream>
#include "ap_verifier.h"

using namespace std;
int var_num = 4;
bdd *singleVarTrue;
bdd *singleVarFalse;

bdd construct_from_string(string str, int len) {
    bdd nbdd = bddtrue;
    for (int i = 0; i < len; i++) {
        if (str[i] == 'x')
            continue;
        else {
            if (str[i] == '1')
                nbdd &= singleVarTrue[i];
            else
                nbdd &= singleVarFalse[i];
        }
    }
    return nbdd;
}

void testBDDbasic() {
    string a = "xxx1";
    string b = "1xx0";
    bdd abdd, bbdd;
    abdd = construct_from_string(a, var_num);
    bbdd = construct_from_string(b, var_num);
    cout << bddtable << abdd << endl;
    cout << bddtable << bbdd << endl;
    bdd x = abdd | bbdd;
    cout << bddtable << x << endl;
    if (x != bddfalse) {
        cout << "Wrong..." << endl;
    }
    string r1 = "xx11";
    string r2 = "11xx";
    string r3 = "xxxx";
    bdd bdd1 = construct_from_string(r1, var_num);
    bdd bdd2 = construct_from_string(r2, var_num);
    bdd bdd3 = construct_from_string(r3, var_num);
    bdd bdd2A = (bdd3 - bdd2) | bdd1;
    bdd bdd2B = bdd2 - bdd1;
    cout << bddtable << bdd2A << endl;
    cout << bddtable << bdd2B << endl;
    cout << bdd_satcount(bdd2A) << endl;
    cout << bdd_satcount(bdd2B) << endl;
}

list<long> load_apverifier_from_dir(string json_file_path, APVerifier *A) {
    struct timeval start,end;
    ifstream jsfile;

    Json::Value root;
    Json::Reader reader;
    list<long> t_list;
    long total_run_time = 0;

    // read topology
    string file_name = json_file_path + "/" + "topology.json";
    jsfile.open(file_name.c_str());
    if (!jsfile.good()) {
        printf("Error opening the file %s\n", file_name.c_str());
        return t_list;
    }
    reader.parse(jsfile, root, false);
    Json::Value topology = root["topology"];
    for (unsigned i = 0; i < topology.size(); i++) {
        A->add_link(topology[i]["src"].asInt(), topology[i]["dst"].asInt());
    }
    A->print_topology();
    jsfile.close();
}

int main(int argc, char* argv[]) {
    std::cout << "Hello, here is atomic-predicates Verifier." << std::endl;
    bool do_run_test = false;
    bool do_load_json_files = false;
    int hdr_len = 1;
    var_num = 8;

    string json_files_path = "";

    for (int i = 1; i < argc; i++) {
        if ( strcmp(argv[i] , "--help") == 0 ) {
            printf("Usage: ap-verifier [run option(s)][settings]\n");
            printf("  run options:\n");
            printf("\t --test  runs all the unit tests.\n");
            printf("\t --load <path> : load the rules from json files in the <path>.\n");
            // printf("\t --policy <file> : loads the source and probe nodes from a json policy <file>.\n");
            // printf("\t --filter <filter-wc> : cluster based on <wc-filter>.\n");

            printf("  settings:\n");
            printf("\t --hdr-len <length> : <length> of packet header (default is 1 byte).\n");
            break;
        }
        if ( strcmp(argv[i],"--test") == 0 ) {
            do_run_test = true;
        }

        if ( strcmp(argv[i],"--load") == 0)  {
            if (i+1 >= argc) {
                printf("Please specify path to json files after --load.\n");
                return -1;
            }
            do_load_json_files = true;
            json_files_path = string(argv[++i]);
        }

        if ( strcmp(argv[i],"--hdr-len") == 0 ) {
            if (i+1 >= argc) {
                printf("Please specify length of header after --hdr-len.\n");
                return -1;
            }
            hdr_len = atoi(argv[++i]);
            var_num = 8 * hdr_len;
        }
    }

    APVerifier *A = new APVerifier(var_num);

    // prepare bdd basics
    bdd_init(10000, 10000);
    bdd_setvarnum(var_num);
    singleVarTrue = (bdd *)malloc(sizeof(bdd) * var_num);
    singleVarFalse = (bdd *)malloc(sizeof(bdd) * var_num);
    for (int i = 0; i < var_num; i++) {
        singleVarTrue[i] = bdd_ithvar(i);
        singleVarFalse[i] = bdd_nithvar(i);
    }
    if (do_run_test) {
        testBDDbasic();
    }

    if (do_load_json_files) {
        load_apverifier_from_dir(json_files_path, A);
    }
    bdd_done();
    return 0;
}