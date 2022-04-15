#include "llvm_gen.h"

static std::map<int, llvm::AllocaInst *> values_table;
static llvm::FunctionPassManager *FPM;

void llvm_generate (FILE *tree_lang, FILE *llvm_ir)
{
    assert(tree_lang);
    assert(llvm_ir);

    text_t text = {};
    construct_text(&text);

    fill_text(tree_lang, &text);

    text.counter = text.lines[0].line;

    while ((isspace(*(text.counter)) || *(text.counter) == '{') && *(text.counter) != '\0')
    {
        if (*(text.counter) == '\n' && text.line_counter < text.n_real_lines - 1)
            text.counter = text.lines[++(text.line_counter)].line;
        else
            text.counter++;
    }

    bin_tree tree = {};
    construct_tree(&tree, "tree");

    variables var = {};

    free(tree.root);
    tree.root = fill_tree(&text, &var);

    generate_llvm_ir(&tree, llvm_ir, &var);

    destruct_text(&text);
    destruct_tree(&tree);
}

static llvm::AllocaInst *create_entry_block_alloca (llvm::LLVMContext *context, llvm::Function *function, const std::string &var_name)
{
  llvm::IRBuilder<> temp_builder(&function->getEntryBlock(), function->getEntryBlock().begin());

  return temp_builder.CreateAlloca(llvm::Type::getDoubleTy(*context), 0, var_name.c_str());
}

void generate_llvm_ir (bin_tree *tree, FILE *llvm_ir, variables *var)
{
    // Preparation
    bin_tree_elem *vertex = tree->root;

    int n_glob_vars = 0;
    // while (vertex->left->type == COMMAND && (int) vertex->left->value == ASSIGN) // global variables
    // {
    //     analyse_expr(vertex->left->right, llvm_ir, var);
    //     fprintf(llvm_ir, "\npop [rdx+%d]\n", (int) vertex->left->left->value);
    //     vertex = vertex->right;

    //     n_glob_vars++;
    // }

    find_glob_vars(vertex, n_glob_vars);
    main_var_convert_variables(vertex, n_glob_vars);

    uint64_t n_func_local_variables = 1 + get_max_local_variable(vertex->left); // begins with 0th element
    uint64_t n_func_arguments  = get_n_func_arguments(vertex->left);

    // Beginning to generate LLVM IR
    llvm::LLVMContext context;
    llvm::Module* module = new llvm::Module("top", context);
    llvm::IRBuilder<> builder(context);

    // Generating function declarations from stdlib
    generate_llvm_ir_std_func_decl(&context, module, &builder);

    // Generating user function declarations
    bin_tree_elem *bunch_function = vertex->right;
    while (bunch_function != nullptr)
    {
        llvm::Function *function = generate_llvm_ir_func_decl(bunch_function->left, &context, module, &builder, var);
        if (function == nullptr)
            fprintf(stderr, "Error: function redefinition \"%s\"\n", var->var[(int) bunch_function->left->value]);

        bunch_function = bunch_function->right;
    }

    // Generate int main()
    llvm::FunctionType *func_type = llvm::FunctionType::get(builder.getInt32Ty(), false);
    llvm::Function *main_func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "main", module);

    // Set entry point for int main()
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", main_func);
    builder.SetInsertPoint(entry);

    llvm::Value *init_value = llvm::ConstantFP::get(context, llvm::APFloat(0.));
    for (uint64_t i = n_func_arguments; i < n_func_local_variables; ++i)
    {
        llvm::AllocaInst *alloca = create_entry_block_alloca(&context, main_func, std::string("x") + std::to_string(i));

        builder.CreateStore(init_value, alloca);
        values_table[i] = alloca;
    }

    generate_llvm_ir_body(vertex->left->right, &context, module, &builder, var); // begin from bunch
    builder.CreateRet(llvm::ConstantInt::get(builder.getInt32Ty(), 0));

    // Generating LLVM IR for all other functions
    bunch_function = vertex->right;
    while (bunch_function != nullptr)
    {
        llvm::Function *function = generate_llvm_ir_func_def(bunch_function->left, &context, module, &builder, var);
        if (function == nullptr)
            fprintf(stderr, "Error: function redefinition \"%s\"\n", var->var[(int) bunch_function->left->value]);

        bunch_function = bunch_function->right;
    }

    // Dump LLVM IR
    std::string llvm_ir_text;
    llvm::raw_string_ostream os(llvm_ir_text);
    module->print(os, nullptr);
    os.flush();
    fprintf(llvm_ir, "%s", llvm_ir_text.c_str());
}

void find_glob_vars (bin_tree_elem *element, int n_glob_vars)
{
    if (element->left != nullptr)
        find_glob_vars(element->left, n_glob_vars);

    if (element->type == VAR && element->value < n_glob_vars)
        element->type = GLOB_VAR;

    if (element->right != nullptr)
        find_glob_vars(element->right, n_glob_vars);
}

void main_var_convert_variables (bin_tree_elem *element, int n_glob_vars)
{
    if (element->left != nullptr)
        main_var_convert_variables(element->left, n_glob_vars);

    if (element->type == VAR)
        element->value -= n_glob_vars;

    if (element->right != nullptr)
        main_var_convert_variables(element->right, n_glob_vars);
}

int get_n_func_arguments (bin_tree_elem *element)
{
    int n_arguments = 0;

    for (bin_tree_elem *arg = element->left; arg != nullptr; arg = arg->left)
        n_arguments++;

    return n_arguments;
}

int get_max_local_variable (bin_tree_elem *element)
{
    if (element->type == VAR)
        return (int) element->value;
    
    int n_left_local_variables  = -1;
    int n_right_local_variables = -1;

    if (element->left != nullptr)
        n_left_local_variables = get_max_local_variable(element->left);

    if (element->right != nullptr)
        n_right_local_variables = get_max_local_variable(element->right);

    return std::max(n_left_local_variables, n_right_local_variables);
}

llvm::Value *generate_llvm_ir_body (bin_tree_elem *vertex, llvm::LLVMContext *context, llvm::Module *module, llvm::IRBuilder<> *builder, variables *var)
{
    llvm::Value *ret_val = nullptr;
    while (vertex != nullptr && ret_val == nullptr)
    {
        generate_llvm_ir_cmd(vertex->left, context, module, builder, var);
        vertex = vertex->right;
    }

    return ret_val;
}

llvm::Value *generate_llvm_ir_cmd (bin_tree_elem *vertex, llvm::LLVMContext *context, llvm::Module *module, llvm::IRBuilder<> *builder, variables *var)
{
    if (vertex->type == COMMAND && (int) vertex->value == ASSIGN)
    {
        llvm::Value *assign_value = generate_llvm_ir_expr(vertex->right, context, module, builder, var);
        if (assign_value == nullptr)
            return nullptr;

        if (vertex->left->type == VAR)
        {
            llvm::Value *variable = values_table[(int) vertex->left->value];
            if (variable != nullptr)
            {
                builder->CreateStore(assign_value, variable);
                return assign_value;
            }
            else
                return nullptr;
        }
        // else if (element->left->type == GLOB_VAR)
        // {

        // }
    }
    else if (vertex->type == COMMAND && (int) vertex->value == IF)
    {
        llvm::Value *lhs_comp = generate_llvm_ir_expr(vertex->left->left,  context, module, builder, var);
        llvm::Value *rhs_comp = generate_llvm_ir_expr(vertex->left->right, context, module, builder, var);

        llvm::Value *cond_res = nullptr;

        switch ((int) vertex->left->value)
        {
            case JE:
                cond_res = builder->CreateFCmp(llvm::FCmpInst::FCMP_UEQ, lhs_comp, rhs_comp, "tmpres_ueq");
                break;
            case JA:
                cond_res = builder->CreateFCmp(llvm::FCmpInst::FCMP_UGT, lhs_comp, rhs_comp, "tmpres_ugt");
                break;
            case JAE:
                cond_res = builder->CreateFCmp(llvm::FCmpInst::FCMP_UGE, lhs_comp, rhs_comp, "tmpres_uge");
                break;
            case JB:
                cond_res = builder->CreateFCmp(llvm::FCmpInst::FCMP_ULT, lhs_comp, rhs_comp, "tmpres_ult");
                break;
            case JBE:
                cond_res = builder->CreateFCmp(llvm::FCmpInst::FCMP_ULE, lhs_comp, rhs_comp, "tmpres_ule");
                break;
            case JNE:
                cond_res = builder->CreateFCmp(llvm::FCmpInst::FCMP_UNE, lhs_comp, rhs_comp, "tmpres_une");
                break;
            default:
                return nullptr;
        }

        llvm::Function *function = builder->GetInsertBlock()->getParent();

        llvm::BasicBlock *if_body       = llvm::BasicBlock::Create(*context, "if_then", function);
        llvm::BasicBlock *continue_body = llvm::BasicBlock::Create(*context, "if_else");

        builder->CreateCondBr(cond_res, if_body, continue_body);

        builder->SetInsertPoint(if_body);
        generate_llvm_ir_body(vertex->right, context, module, builder, var); // begin from bunch
        builder->CreateBr(continue_body);

        function->getBasicBlockList().push_back(continue_body);
        builder->SetInsertPoint(continue_body);
    }
    else if (vertex->type == COMMAND && (int) vertex->value == WHILE)
    {
        llvm::Function *function = builder->GetInsertBlock()->getParent();

        llvm::BasicBlock *loop_cond = llvm::BasicBlock::Create(*context, "loop_cond", function);
        builder->CreateBr(loop_cond);
        builder->SetInsertPoint(loop_cond);

        llvm::Value *lhs_comp = generate_llvm_ir_expr(vertex->left->left,  context, module, builder, var);
        llvm::Value *rhs_comp = generate_llvm_ir_expr(vertex->left->right, context, module, builder, var);

        llvm::Value *cond_res = nullptr;

        switch ((int) vertex->left->value)
        {
            case JE:
                cond_res = builder->CreateFCmp(llvm::FCmpInst::FCMP_UEQ, lhs_comp, rhs_comp, "tmpres_ueq");
                break;
            case JA:
                cond_res = builder->CreateFCmp(llvm::FCmpInst::FCMP_UGT, lhs_comp, rhs_comp, "tmpres_ugt");
                break;
            case JAE:
                cond_res = builder->CreateFCmp(llvm::FCmpInst::FCMP_UGE, lhs_comp, rhs_comp, "tmpres_uge");
                break;
            case JB:
                cond_res = builder->CreateFCmp(llvm::FCmpInst::FCMP_ULT, lhs_comp, rhs_comp, "tmpres_ult");
                break;
            case JBE:
                cond_res = builder->CreateFCmp(llvm::FCmpInst::FCMP_ULE, lhs_comp, rhs_comp, "tmpres_ule");
                break;
            case JNE:
                cond_res = builder->CreateFCmp(llvm::FCmpInst::FCMP_UNE, lhs_comp, rhs_comp, "tmpres_une");
                break;
            default:
                return nullptr;
        }

        llvm::BasicBlock *loop_body = llvm::BasicBlock::Create(*context, "loop", function);
        llvm::BasicBlock *loop_end  = llvm::BasicBlock::Create(*context, "loop_end");

        builder->CreateCondBr(cond_res, loop_body, loop_end);

        builder->SetInsertPoint(loop_body);
        generate_llvm_ir_body(vertex->right, context, module, builder, var); // begin from bunch
        builder->CreateBr(loop_cond);

        function->getBasicBlockList().push_back(loop_end);
        builder->SetInsertPoint(loop_end);
    }
    else if (vertex->type == FUNC && ((int) vertex->value == SCAN || (int) vertex->value == PRINT))
    {
        if ((int) vertex->value == SCAN && vertex->left->type == VAR)
        {
            return builder->CreateCall(module->getFunction("scanf"), {builder->CreateGlobalStringPtr("%lg"), values_table[(int) vertex->left->value]});
        }
        else if ((int) vertex->value == PRINT)
        {
            llvm::Value *print_val = generate_llvm_ir_expr(vertex->left, context, module, builder, var);
            return builder->CreateCall(module->getFunction("printf"), {builder->CreateGlobalStringPtr("%lg\n"), print_val});
        }
        else
            return nullptr;
    }
    else if (vertex->type == USER_FUNC)
    {
        std::vector<llvm::Value *> callee_arguments;
        for (bin_tree_elem *arg_vertex = vertex->left; arg_vertex != nullptr; arg_vertex = arg_vertex->left)
        {
            llvm::Value *arg = generate_llvm_ir_expr(arg_vertex->right, context, module, builder, var);
            if (arg == nullptr)
                return nullptr;
            
            callee_arguments.push_back(arg);
        }

        llvm::Function *callee_function = module->getFunction(std::string(var->var[(int) vertex->value]));
        if (callee_function == nullptr)
        {
            fprintf(stderr, "Call unknown function \"%s\"\n", var->var[(int) vertex->value]);
            return nullptr;
        }

        if (callee_arguments.size() != callee_function->getFunctionType()->getNumParams())
        {
            fprintf(stderr, "Call function \"%s\"\n with invalid amount of arguments", var->var[(int) vertex->value]);
            return nullptr;
        }

        return builder->CreateCall(callee_function, callee_arguments, "tmp");
    }
    else if (vertex->type == RETURN)
    {
        if (vertex->left == nullptr)
            return builder->CreateRetVoid();
        else
            return builder->CreateRet(generate_llvm_ir_expr(vertex->left, context, module, builder, var));
    }
    else
        return nullptr;

    return nullptr;
}

llvm::Value *generate_llvm_ir_expr (bin_tree_elem *element, llvm::LLVMContext *context, llvm::Module *module, llvm::IRBuilder<> *builder, variables *var)
{
    if (element->type == NUM)
        return llvm::ConstantFP::get(*context, llvm::APFloat(element->value));
    // else if (element->type == GLOB_VAR)
    // {
        
    // }
    else if (element->type == VAR)
    {
        llvm::AllocaInst *var = values_table[(int) element->value];
        return builder->CreateLoad(var->getAllocatedType(), var, (std::string("x") + std::to_string((int) element->value)).c_str());
    }   
    else if (element->type == USER_FUNC)
    {
        std::vector<llvm::Value *> callee_arguments;
        for (bin_tree_elem *vertex = element->left; vertex != nullptr; vertex = vertex->left)
        {
            llvm::Value *arg = generate_llvm_ir_expr(vertex->right, context, module, builder, var);
            if (arg == nullptr)
                return nullptr;
            
            callee_arguments.push_back(arg);
        }

        llvm::Function *callee_function = module->getFunction(std::string(var->var[(int) element->value]));
        if (callee_function == nullptr)
        {
            fprintf(stderr, "Call unknown function \"%s\"\n", var->var[(int) element->value]);
            return nullptr;
        }

        if (callee_arguments.size() != callee_function->getFunctionType()->getNumParams())
        {
            fprintf(stderr, "Call function \"%s\"\n with invalid amount of arguments", var->var[(int) element->value]);
            return nullptr;
        }

        return builder->CreateCall(callee_function, callee_arguments, "tmp");
    }
    else if (element->type == OPER && (int) element->value == ADD)
    {
        llvm::Value *left_val  = generate_llvm_ir_expr(element->left,  context, module, builder, var);
        llvm::Value *right_val = generate_llvm_ir_expr(element->right, context, module, builder, var);

        if (left_val != nullptr || right_val != nullptr)
            return builder->CreateFAdd(left_val, right_val, "addtmp");
        else
            return nullptr;
    }
    else if (element->type == OPER && (int) element->value == SUB)
    {
        llvm::Value *left_val  = generate_llvm_ir_expr(element->left,  context, module, builder, var);
        llvm::Value *right_val = generate_llvm_ir_expr(element->right, context, module, builder, var);

        if (left_val != nullptr || right_val != nullptr)
            return builder->CreateFSub(left_val, right_val, "subtmp");
        else
            return nullptr;
    }
    else if (element->type == OPER && (int) element->value == MUL)
    {
        llvm::Value *left_val  = generate_llvm_ir_expr(element->left,  context, module, builder, var);
        llvm::Value *right_val = generate_llvm_ir_expr(element->right, context, module, builder, var);

        if (left_val != nullptr || right_val != nullptr)
            return builder->CreateFMul(left_val, right_val, "multmp");
        else
            return nullptr;
    }
    else if (element->type == OPER && (int) element->value == DIV)
    {
        llvm::Value *left_val  = generate_llvm_ir_expr(element->left,  context, module, builder, var);
        llvm::Value *right_val = generate_llvm_ir_expr(element->right, context, module, builder, var);

        if (left_val != nullptr || right_val != nullptr)
            return builder->CreateFDiv(left_val, right_val, "divtmp");
        else
            return nullptr;
    }
    else if ((element->type == OPER && (int) element->value == POW) || (element->type == FUNC && (int) element->value == POWER))
    {
        llvm::Value *left_val  = generate_llvm_ir_expr(element->left,  context, module, builder, var);
        llvm::Value *right_val = generate_llvm_ir_expr(element->right, context, module, builder, var);

        if (left_val != nullptr || right_val != nullptr)
        {
            llvm::Function *callee_function = module->getFunction("pow");
            std::vector<llvm::Value *> callee_arguments;
            callee_arguments.push_back(left_val);
            callee_arguments.push_back(right_val);

            return builder->CreateCall(callee_function, callee_arguments, "powtmp");
        }
        else
            return nullptr;
    }
    else if (element->type == FUNC)
    {
        llvm::Value *argument = generate_llvm_ir_expr(element->left, context, module, builder, var);
        std::vector<llvm::Value *> callee_argument;
        callee_argument.push_back(argument);

        llvm::Value *double_one = llvm::ConstantFP::get(*context, llvm::APFloat(1.));

        llvm::Function *callee_function = nullptr;
        if (argument != nullptr)
        {
            if ((int) element->value == SIN)
                return builder->CreateCall(module->getFunction("sin"), callee_argument, "sintmp");
            else if ((int) element->value == COS)
                return builder->CreateCall(module->getFunction("cos"), callee_argument, "costmp");
            else if ((int) element->value == TG)
                return builder->CreateCall(module->getFunction("tan"), callee_argument, "tantmp");
            else if ((int) element->value == CTG)
            {
                callee_function = module->getFunction("tan");
                return builder->CreateFDiv(double_one, builder->CreateCall(callee_function, callee_argument, "tantmp"), "ctantmp");
            }

            else if ((int) element->value == ARCSIN)
                return builder->CreateCall(module->getFunction("asin"), callee_argument, "asintmp");
            else if ((int) element->value == ARCCOS)
                return builder->CreateCall(module->getFunction("acos"), callee_argument, "acostmp");
            else if ((int) element->value == ARCTG)
                return builder->CreateCall(module->getFunction("atan"), callee_argument, "atantmp");
            else if ((int) element->value == ARCCTG)
            {
                llvm::Value *pi_div_2 = llvm::ConstantFP::get(*context, llvm::APFloat(M_PI_2));
                callee_function = module->getFunction("actan");
                return builder->CreateFSub(pi_div_2, builder->CreateCall(callee_function, callee_argument, "atantmp"), "actantmp");
            }

            else if ((int) element->value == SH)
                return builder->CreateCall(module->getFunction("sinh"), callee_argument, "sinhtmp");
            else if ((int) element->value == CH)
                return builder->CreateCall(module->getFunction("cosh"), callee_argument, "coshtmp");
            else if ((int) element->value == TH)
                return builder->CreateCall(module->getFunction("tanh"), callee_argument, "tanhtmp");
            else if ((int) element->value == CTH)
            {
                callee_function = module->getFunction("tanh");
                return builder->CreateFDiv(double_one, builder->CreateCall(callee_function, callee_argument, "tanhtmp"), "ctanhtmp");
            }

            else if ((int) element->value == LN)
                return builder->CreateCall(module->getFunction("ln"), callee_argument, "lntmp");
            else if ((int) element->value == EXP)
                return builder->CreateCall(module->getFunction("exp"), callee_argument, "exptmp");
            else
                return nullptr;
        }
        else
            return nullptr;
    }

    return nullptr;
}

void find_func_var (bin_tree_elem *element, int *param, int *n_param, int first_loc_var)
{
    if (element->left != nullptr)
        find_func_var(element->left, param, n_param, first_loc_var);

    if (element->type == VAR)
    {
        int find_state = 0;

        for (int i = first_loc_var; i < *n_param; i++)
        {
            if (param[i] == (int) element->value)
            {
                find_state = 1;
                element->value = i;
                break;
            }
        }

        if (find_state == 0 && element->value >= first_loc_var) // see user_func_optimize()
        {
            param[*n_param] = (int) element->value;
            element->value = *n_param;
            (*n_param)++;
        }
    }

    if (element->right != nullptr)
        find_func_var(element->right, param, n_param, first_loc_var);
}

int get_func_return_double (bin_tree_elem *element)
{
    int func_return_double = 0;

    if (element->type == RETURN)
    {
        if (element->left != nullptr)
            return 1;
        else
            return 0;
    }

    if (element->left != nullptr)
        func_return_double = get_func_return_double(element->left);

    if (element->right != nullptr)
        func_return_double = std::max(get_func_return_double(element->right), func_return_double);

    return func_return_double;
}

void generate_llvm_ir_std_func_decl (llvm::LLVMContext *context, llvm::Module *module, llvm::IRBuilder<> *builder)
{
    std::vector<llvm::Type *> double_arg(1, llvm::Type::getDoubleTy(*context));
    llvm::ArrayRef<llvm::Type *> arg_ref(double_arg);
    llvm::FunctionType *func_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(*context), arg_ref, false);

    llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "sin",  module);
    llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "cos",  module);
    llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "tan",  module);

    llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "acos", module);
    llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "asin", module);
    llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "atan", module);

    llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "sinh", module);
    llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "cosh", module);
    llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "tanh", module);

    llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "exp", module);
    llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ln",  module);

    std::vector<llvm::Type *> double_args(2, llvm::Type::getDoubleTy(*context));
    llvm::ArrayRef<llvm::Type *> args_ref(double_args);
    func_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(*context), args_ref, false);

    llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "pow",  module);

    llvm::FunctionType *io_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), {llvm::Type::getInt8PtrTy(*context)}, true);
    llvm::Function *scanf_func  = llvm::Function::Create(io_type, llvm::GlobalValue::ExternalLinkage, "scanf",  module);
    llvm::Function *printf_func = llvm::Function::Create(io_type, llvm::GlobalValue::ExternalLinkage, "printf", module);
}

llvm::Function *generate_llvm_ir_func_decl (bin_tree_elem *vertex, llvm::LLVMContext *context, llvm::Module *module, llvm::IRBuilder<> *builder, variables *var)
{
    // Preparation
    int *param = (int *) calloc(MAX_FUNC_PARAM, sizeof(int));
    int n_param = 0;

    for (bin_tree_elem *elem = vertex->left; elem != nullptr; elem = elem->left)
        param[n_param++] = elem->value;

    int first_loc_var = n_param;

    user_func_convert_variables(vertex, param, n_param);
    find_func_var(vertex->right, param, &n_param, first_loc_var);
    free(param);

    int n_func_arguments       = get_n_func_arguments(vertex);
    int return_double_value    = get_func_return_double(vertex);

    // Function arguments
    std::vector<llvm::Type *> doubles(n_func_arguments, llvm::Type::getDoubleTy(*context));
    llvm::ArrayRef<llvm::Type *> args_ref(doubles);

    // Function prototype
    llvm::FunctionType *func_type = nullptr;
    if (return_double_value == 1)
        func_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(*context), args_ref, false);
    else
        func_type = llvm::FunctionType::get(llvm::Type::getVoidTy(*context), args_ref, false);

    // Create function
    llvm::Function *function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, var->var[(int) vertex->value], module);

    if (function->getName() != std::string(var->var[(int) vertex->value])) // already exist in module
    {
        function->eraseFromParent();
        return nullptr;
    }

    return function;
}

llvm::Function *generate_llvm_ir_func_def (bin_tree_elem *vertex, llvm::LLVMContext *context, llvm::Module *module, llvm::IRBuilder<> *builder, variables *var)
{
    llvm::Function *function = module->getFunction(std::string(var->var[(int) vertex->value]));
    if (function == nullptr)
        return nullptr;

    // Fill values table
    llvm::FunctionType *function_type = function->getFunctionType();
    uint64_t n_func_arguments = function_type->getNumParams();

    // Set entry point
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(entry);

    values_table.clear(); // clear to fulfill with new values           
    size_t index = 0;
    for (llvm::Function::arg_iterator iter = function->arg_begin(); index != n_func_arguments; ++index, ++iter)
    {
        llvm::AllocaInst *alloca = create_entry_block_alloca(context, function, std::string("x") + std::to_string(index));
        builder->CreateStore(iter, alloca);
        values_table[index] = alloca;
    }

    llvm::Value *init_value = llvm::ConstantFP::get(*context, llvm::APFloat(0.));
    uint64_t n_func_local_variables = 1 + get_max_local_variable(vertex); // begins with 0th element
    for (uint64_t i = n_func_arguments; i < n_func_local_variables; ++i)
    {
        llvm::AllocaInst *alloca = create_entry_block_alloca(context, function, std::string("x") + std::to_string(i));

        builder->CreateStore(init_value, alloca);
        values_table[i] = alloca;
    }
        
    generate_llvm_ir_body(vertex->right, context, module, builder, var);

    if (function_type->getReturnType() == builder->getVoidTy())
        builder->CreateRetVoid();
        
    llvm::verifyFunction(*function);

    return function;
}

void user_func_convert_variables (bin_tree_elem *element, int *param, int n_param)
{
    if (element->left != nullptr)
        user_func_convert_variables(element->left, param, n_param);

    if (element->type == VAR)
    {
        int find_state = 0;

        for (int i = 0; i < n_param; i++)
        {
            if (param[i] == (int) element->value) // if agrument of function
            {
                find_state     = 1;
                element->value = i;
            }
        }

        if (find_state == 0 && element->value < n_param)
            element->value += MARK_FUNC_USED_VARIABLE;   // the trick to mark the element of tree, which is not function argument
    }

    if (element->right != nullptr)
        user_func_convert_variables(element->right, param, n_param);
}
