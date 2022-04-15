#include "llvm_gen.h"

int main(int argc, char *argv[])
{
    FILE *tree    = fopen(argv[1], "rb");
    FILE *llvm_ir = fopen(argv[2], "wb");

    if (tree == NULL && llvm_ir == NULL)
        fprintf(stderr, "\nSorry, file \"%s\" doesn't exist and file \"%s\" cannot be open (system error)\n", argv[1], argv[2]);
    else if (tree == NULL)
    {
        fprintf(stderr, "\nSorry, file \"%s\" doesn't exist\n", argv[1]);
        fclose(llvm_ir);
    }
    else if (llvm_ir == NULL)
    {
        fprintf(stderr, "\nSorry, file \"%s\" cannot be opened (system error).\n", argv[2]);
        fclose(tree);
    }
    else
    {
        llvm_generate(tree, llvm_ir);

        fclose(tree);
        fclose(llvm_ir);

        printf("OK. The program has been compiled into assembler file.\n");
    }

    return 0;
}
