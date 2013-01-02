
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <string>
#include <map>
using std::string;
using std::map;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

void
show_help()
{
    cout << "\nUsage\n"
         << "  fry -o out.bc -f fun_name -s \"a=1, b=2\"\n"
         << endl;
}

map<string, string> parse_spec_vals(string);
void spec_function_in_bitcode(string, string, string, map<string, string>);

int
main(int argc, char* argv[])
{
    string src_file;
    string dst_file;

    string spec_func;
    string spec_vals;

    int opt;
    while ((opt = getopt(argc, argv, "ho:f:s:")) != -1) {
        switch(opt) {
            case 'h':
                show_help();
                exit(0);
            case 'o':
                dst_file = string(optarg);
                break;
            case 'f':
                spec_func = string(optarg);
                break;
            case 's':
                spec_vals = string(optarg);
                break;
            default:
                cerr << "Error: Unknown option: " 
                     << char(opt) << "/" << int(opt) << endl;
                exit(1);
        }
    }

    if (optind != argc - 1) {
        cerr << "Error: No input file specified" << endl;
        show_help();
        exit(1);
    }

    src_file = string(argv[optind]);

    if (src_file.empty() || dst_file.empty() 
            || spec_func.empty() || spec_vals.empty()) {
        cerr << "Error: Missing required options" << endl;
        show_help();
        exit(1);
    }

    auto specs = parse_spec_vals(spec_vals);

    for (auto it = specs.begin(); it != specs.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }

    spec_function_in_bitcode(src_file, dst_file, spec_func, specs);

    return 0;
}
