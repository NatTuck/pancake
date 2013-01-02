
#include <stdlib.h>
#include <ctype.h>

#include <string>
#include <vector>
#include <map>
using std::string;
using std::vector;
using std::map;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <fstream>
using std::ofstream;

#include <llvm/LLVMContext.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Module.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/system_error.h>
#include <llvm/Function.h>
#include <llvm/Argument.h>
#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Constants.h>

#include "nat/string.hh"

using namespace llvm;

string
type_name(Type* type)
{
    string name;
    raw_string_ostream stream(name);
    type->print(stream);
    name = stream.str();
    return name;
}

map<string, string>
parse_spec_vals(string spec_vals)
{
    map<string, string> specs;

    spec_vals = string_remove_whitespace(spec_vals);

    vector<string> ys = string_split(",", spec_vals);
    for (size_t ii = 0; ii < ys.size(); ++ii) {
        vector<string> pair = string_split("=", ys[ii]);
        specs[pair[0]] = pair[1];
    }

    return specs;
}

void 
spec_function_in_bitcode(string src_file, string dst_file, 
        string spec_func, map<string, string> spec_vals)
{
    cout << "Reading input: '" << src_file << "'" << endl;

    string errMsg;
    OwningPtr<MemoryBuffer> buf;

    error_code ec = MemoryBuffer::getFile(src_file, buf);
    if (ec != error_code::success()) {
        cerr << "Error: Unable to load input bitcode." << endl;
        cerr << "LLVM error code: " <<  ec.message() << endl;
        exit(1);
    }
    
    Module *mod = ParseBitcodeFile(buf.get(), getGlobalContext(), &errMsg);

    Function *fun = mod->getFunction(spec_func);

    cout << "Function " << spec_func << " has " << fun->arg_size() << " args." << endl;

    vector<string> arg_names;
    vector<Type*>  arg_types;

    auto& args = fun->getArgumentList();
    for (auto it = args.begin(); it != args.end(); ++it) {
        string name = it->getName().str();
        Type*  type = it->getType();

        if (spec_vals.find(name) != spec_vals.end()) {
            cout << "Arg " << name << ":" << type_name(type) << " (spec: "
                 << spec_vals[name] << ")" << endl;
        }
        else {
            cout << "Arg " << name << ":" << type_name(type) << endl;
            arg_names.push_back(name);
            arg_types.push_back(type);
        }
    }

    string spec_name = spec_func + "_spec";
    FunctionType* ft = FunctionType::get(fun->getReturnType(), arg_types, false);
    Function* sfun = Function::Create(ft, Function::ExternalLinkage, spec_name, mod);

    map<string, Value*> sfun_args;

    int ii = 0;
    for (auto it = sfun->arg_begin(); it != sfun->arg_end(); ++it, ++ii) {
        it->setName(arg_names[ii]);
        sfun_args[arg_names[ii]] = it;
    }

    BasicBlock *block = BasicBlock::Create(getGlobalContext(), "entry", sfun);
    IRBuilder<> bb(getGlobalContext());
    bb.SetInsertPoint(block);
    
    vector<Value*> call_args;

    for (auto it = args.begin(); it != args.end(); ++it) {
        string name = it->getName().str();
        Type*  type = it->getType();

        if (spec_vals.find(name) != spec_vals.end()) {
            if (type_name(type) != "i32") {
                cerr << "Can only specialize integers." << endl;
                exit(1);
            }

            call_args.push_back(ConstantInt::get(type, atoi(spec_vals[name].c_str())));
        }
        else {
            call_args.push_back(sfun_args[name]);
        }
    }

    Value* rv = bb.CreateCall(fun, call_args);
    bb.CreateRet(rv);

    verifyFunction(*sfun);

    cout << "Writing output to " << dst_file.c_str() << endl;
    
    ofstream out_ofs(dst_file);
    raw_os_ostream raw_ofs(out_ofs);

    WriteBitcodeToFile(mod, raw_ofs);
}
