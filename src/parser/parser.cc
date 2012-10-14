
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <stdio.h>

#include <pancake/llvm.h>
#include <pancake/parser.h>

llvm::Module* pancake_clang_parse(const char* path);

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

    module->module = pancake_clang_parse(temp_name);

    fclose(temp_file);

    return module;
}

char* 
pancake_module_get_asm(pancake_module* module)
{
    if (module->asm_text)
	return module->asm_text;

    module->asm_text = strdup("rofl");

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

/*
 * The following is modified from the clang-interpreter example 
 * distributed with clang.
 *
 */

using namespace clang;
using namespace clang::driver;

// This function isn't referenced outside its translation unit, but it
// can't use the "static" keyword because its address is used for
// GetMainExecutable (since some platforms don't support taking the
// address of main, and some platforms can't implement GetMainExecutable
// without being given the address of a function in the main executable).
llvm::sys::Path 
GetExecutablePath(const char *Argv0) 
{
  // This just needs to be some symbol in the binary; C++ doesn't
  // allow taking the address of ::main however.
  void *MainAddr = (void*) (intptr_t) GetExecutablePath;
  return llvm::sys::Path::GetMainExecutable(Argv0, MainAddr);
}

llvm::Module*
pancake_clang_parse(const char* path)
{
  void *MainAddr = (void*) (intptr_t) GetExecutablePath;
  llvm::sys::Path Path = GetExecutablePath(__FILE__);
  TextDiagnosticPrinter *DiagClient =
    new TextDiagnosticPrinter(llvm::errs(), DiagnosticOptions());

  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
  DiagnosticsEngine Diags(DiagID, DiagClient);
  Driver TheDriver(Path.str(), llvm::sys::getDefaultTargetTriple(),
                   "a.out", /*IsProduction=*/false, Diags);
  TheDriver.setTitle("clang interpreter");

  // FIXME: This is a hack to try to force the driver to do something we can
  // recognize. We need to extend the driver library to support this use model
  // (basically, exactly one input, and the operation mode is hard wired).
  
  char** argv = (char**) alloca(2*sizeof(char*));
  argv[1] = strdupa(path);
  argv[2] = strdupa(path);
  llvm::SmallVector<const char *, 16> Args(argv, argv + 2);
  //Args.push_back("-fsyntax-only");
  OwningPtr<Compilation> C(TheDriver.BuildCompilation(Args));
  if (!C) {
    fprintf(stderr, "clang_parse: giving up at line %d.\n", __LINE__);
    exit(1);      
  }

  // FIXME: This is copied from ASTUnit.cpp; simplify and eliminate.

  // We expect to get back exactly one command job, if we didn't something
  // failed. Extract that job from the compilation.
  const driver::JobList &Jobs = C->getJobs();
  if (Jobs.size() != 1 || !isa<driver::Command>(*Jobs.begin())) {
    SmallString<256> Msg;
    llvm::raw_svector_ostream OS(Msg);
    C->PrintJob(OS, C->getJobs(), "; ", true);
    Diags.Report(diag::err_fe_expected_compiler_job) << OS.str();
    fprintf(stderr, "clang_parse: giving up at line %d.\n", __LINE__);
    exit(1);
  }

  const driver::Command *Cmd = cast<driver::Command>(*Jobs.begin());
  if (llvm::StringRef(Cmd->getCreator().getName()) != "clang") {
    Diags.Report(diag::err_fe_expected_clang_command);
    fprintf(stderr, "clang_parse: giving up at line %d.\n", __LINE__);
    exit(1);
  }

  // Initialize a compiler invocation object from the clang (-cc1) arguments.
  const driver::ArgStringList &CCArgs = Cmd->getArguments();
  OwningPtr<CompilerInvocation> CI(new CompilerInvocation);
  CompilerInvocation::CreateFromArgs(*CI,
                                     const_cast<const char **>(CCArgs.data()),
                                     const_cast<const char **>(CCArgs.data()) +
                                       CCArgs.size(),
                                     Diags);

  // Show the invocation, with -v.
  if (CI->getHeaderSearchOpts().Verbose) {
    llvm::errs() << "clang invocation:\n";
    C->PrintJob(llvm::errs(), C->getJobs(), "\n", true);
    llvm::errs() << "\n";
  }

  // FIXME: This is copied from cc1_main.cpp; simplify and eliminate.

  // Create a compiler instance to handle the actual work.
  CompilerInstance Clang;
  Clang.setInvocation(CI.take());

  // Create the compilers actual diagnostics engine.
  Clang.createDiagnostics(int(CCArgs.size()),const_cast<char**>(CCArgs.data()));
  if (!Clang.hasDiagnostics()) {
      fprintf(stderr, "clang_parse: Failed to create diagnostics engine.\n");
      exit(1);
  }

  // Infer the builtin include path if unspecified.
  if (Clang.getHeaderSearchOpts().UseBuiltinIncludes &&
      Clang.getHeaderSearchOpts().ResourceDir.empty())
    Clang.getHeaderSearchOpts().ResourceDir =
      CompilerInvocation::GetResourcesPath("YAY_JIT", MainAddr);

  // Create and execute the frontend to generate an LLVM bitcode module.
  OwningPtr<CodeGenAction> Act(new EmitLLVMOnlyAction());
  if (!Clang.ExecuteAction(*Act)) {
      fprintf(stderr, "clang_parse: Failed to generate module bitcode.\n");
      exit(1);
  }

  llvm::Module *module = Act->takeModule();

  if (!module) {
      fprintf(stderr, "clang_parse: Error producing module.\n");
      exit(1);
  }

  return module;

  /*
  int Res = 255;
  if (llvm::Module *Module = Act->takeModule())
    Res = Execute(Module, envp);

  // Shutdown.

  //llvm::llvm_shutdown();

  return Res;
  */
}
