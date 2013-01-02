
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <map>

#include <pancake/llvm.h>
#include <pancake/parser.h>

#include <llvm/LLVMContext.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Module.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/system_error.h>

using namespace llvm;
using std::string;

Module* pancake_clang_parse(const char* path);

pancake_module*
pancake_create_module(const char* text)
{
    pancake_module* module = (pancake_module*) malloc(sizeof(pancake_module));
    module->asm_text = 0;

    char* temp_name = (char*) alloca(10);
    strcpy(temp_name, "pancakeXXXXXX");

    int   temp_fd   = mkstemp(temp_name);
    FILE* temp_file = fdopen(temp_fd, "r+");

    fprintf(temp_file, "%s\n", text);
    fflush(temp_file);

    module->module = pancake_clang_parse(temp_name);
    

    fclose(temp_file);
    unlink(temp_name);

    return module;
}

char*
pancake_module_get_asm(pancake_module* module)
{
    if (module->asm_text)
	return module->asm_text;

    Module* mm = (Module*) module->module;

    std::string text;
    raw_string_ostream tstr(text);

    mm->print(tstr, 0);

    module->asm_text = strdup(text.c_str());

    return module->asm_text;
}

void 
pancake_destroy_module(pancake_module* module)
{
    /* TODO: free llvm module */

    if (module->asm_text)
	free(module->asm_text);

    free(module);
}

llvm::Module*
pancake_clang_parse(const char* src)
{
    char cmd[100];
    snprintf(cmd, 100, "clang -c -emit-llvm -o '%s.bc' -x cl '%s'", src, src);
    system(cmd);

    char bcf[50];
    snprintf(bcf, 50, "%s.bc", src);

    std::string errMsg;
    OwningPtr<MemoryBuffer> buf;

    printf("Loading file %s\n", bcf);

    error_code ec = MemoryBuffer::getFile(bcf, buf);
    if (ec != error_code::success())
	printf("error code: %s\n", ec.message().c_str());

    unlink(bcf);

    Module *mod = ParseBitcodeFile(buf.get(), getGlobalContext(), &errMsg);

    return mod;
}
