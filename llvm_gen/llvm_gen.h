#ifndef LLVM_GEN_H_INCLUDED
#define LLVM_GEN_H_INCLUDED

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h>

#include <utility>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "tree.h"
#include "llvm_gen_config.h"

#include "text.h"

#define MULTIPLY(left, right)                                                                   \
    create_tree_element(OPER, MUL, left, right)

#define DIVIDE(left, right)                                                                     \
    create_tree_element(OPER, DIV, left, right)

#define ADDITION(left, right)                                                                   \
    create_tree_element(OPER, ADD, left, right)

#define SUBTRACT(left, right)                                                                   \
    create_tree_element(OPER, SUB, left, right)

#define CR_NUM(value)                                                                           \
    create_tree_element(NUM, value, nullptr, nullptr)

#define CR_VAR(value)                                                                           \
    create_tree_element(VAR, value, nullptr, nullptr)

struct variables
{
    int curr_size = 0;

    char var[MAX_VAR_NUM][MAX_VAR_NAME_LENGTH] = {0};
};

struct elements
{
    bin_tree_elem **elements_ = nullptr;

    int curr_size_ = 0;
};

void llvm_generate    (FILE *tree_lang, FILE *llvm_ir);
void generate_llvm_ir (bin_tree *tree,  FILE *llvm_ir, variables *var);
void generate_llvm_ir_std_func_decl        (                       llvm::LLVMContext *context, llvm::Module *module, llvm::IRBuilder<> *builder);
llvm::Function *generate_llvm_ir_func_decl (bin_tree_elem *vertex, llvm::LLVMContext *context, llvm::Module *module, llvm::IRBuilder<> *builder, variables *var);
llvm::Function *generate_llvm_ir_func_def  (bin_tree_elem *vertex, llvm::LLVMContext *context, llvm::Module *module, llvm::IRBuilder<> *builder, variables *var);
llvm::Value    *generate_llvm_ir_body      (bin_tree_elem *vertex, llvm::LLVMContext *context, llvm::Module *module, llvm::IRBuilder<> *builder, variables *var);
llvm::Value    *generate_llvm_ir_cmd       (bin_tree_elem *vertex, llvm::LLVMContext *context, llvm::Module *module, llvm::IRBuilder<> *builder, variables *var);
llvm::Value    *generate_llvm_ir_expr      (bin_tree_elem *vertex, llvm::LLVMContext *context, llvm::Module *module, llvm::IRBuilder<> *builder, variables *var);

bin_tree_elem *fill_tree (text_t *text, variables *var);

int var_search (variables* var, char* temp_var_name);

int get_max_local_variable (bin_tree_elem *element);
int get_n_func_arguments   (bin_tree_elem *element);
int get_func_return_double (bin_tree_elem *element);

void find_glob_vars(bin_tree_elem *element, int n_glob_vars);
void find_func_var (bin_tree_elem *element, int *param, int *n_param, int first_loc_var);

void user_func_convert_variables (bin_tree_elem *element, int *param, int n_param);
void main_var_convert_variables  (bin_tree_elem *element, int n_glob_vars);

#endif // LLVM_GEN_H_INCLUDED
