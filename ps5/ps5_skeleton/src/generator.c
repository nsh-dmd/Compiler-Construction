#include "vslc.h"

static void
generate_stringtable ( void )
{
    /* These can be used to emit numbers, strings and a run-time
     * error msg. from main
     */
    puts ( ".section .rodata" );
    puts ( "intout: .string \"\%ld \"" );
    puts ( "strout: .string \"\%s \"" );
    puts ( "errout: .string \"Wrong number of arguments\"" );

    // lable all strings with number
    for (size_t i = 0; i < stringc; i++) {
        printf("STR%zu: .string\t%s\n", i, string_list[i] );
    }
}

static void generate_global_variables ( void ) {

  puts ( ".section data" );

  // number of global variables in hash table
  size_t n_globs = tlhash_size( global_names );
  // get global variables values and store in a symbol array
  symbol_t **symbols = malloc( n_globs * sizeof(symbol_t *) );
  tlhash_values( global_names, (void **)&symbols);

  // put lables under .section data for global vars
  for (size_t i = 0; i < n_globs; i++) {
    if ( symbols[i]->type == SYM_GLOBAL_VAR ) {
      printf( "_%s\t.zero 8\n", symbols[i]->name);
    }
  }

}


static void allocate_args ( size_t n_args ) {

  // for ( size_t i = 0; i < MIN(n_args, 6); i++) {
  for ( size_t i = 0; i < n_args; i++) {
      ASM1(pushq, record[i]);
      if (i > 5) {
          // i = argument index for remaining args not fitting into registers
          ASM1(pushq, %rbp + 8*i);
      }
  }

}

static void generate_function_call ( symbol_t *function ) {

  printf( "_%s:\n", function->name );
  ASM1(pushq, %rbp);
  ASM2(movq, %rsp, %rbp);

  // arguments on stack
  allocate_args( function->nparms );

  // local variables ????????
  for (size_t i = 0; i < tlhash_size(function->locals); i++) {
      ASM2(subq, $8, %rsp);
  }

}

/* puts expression result in %rax */
static void generate_subexpression1 (node_t *node, symbol_t *function ) {

    // numbers translate into setting them in %rax
    if (node->type == NUMBER_DATA) {
        FASM3(movq, %rax, *(int64_t *)node->data, %l64d);
    }
    // variables translate into copying their contents to %rax
    else if (node->type == IDENTIFIER_DATA) {
        FASM3(movq, %rax, node->entry, %ld);
    }
}

/* obtains expression result in %rax */
static void generate_subexpression2 ( ) {

}

/* Generate code for r.h.s expression */
static void generate_assignment ( node_t *node ) {
    ASM2(movq, , %rax);
}
static void
generate_main ( symbol_t *first )
{
    puts ( ".globl main" );
    puts ( ".section .text" );
    puts ( "main:" );
    puts ( "\tpushq %rbp" );
    puts ( "\tmovq %rsp, %rbp" );

    puts ( "\tsubq $1, %rdi" );
    printf ( "\tcmpq\t$%zu,%%rdi\n", first->nparms );
    puts ( "\tjne ABORT" );
    puts ( "\tcmpq $0, %rdi" );
    puts ( "\tjz SKIP_ARGS" );

    puts ( "\tmovq %rdi, %rcx" );
    printf ( "\taddq $%zu, %%rsi\n", 8*first->nparms );
    puts ( "PARSE_ARGV:" );
    puts ( "\tpushq %rcx" );
    puts ( "\tpushq %rsi" );

    puts ( "\tmovq (%rsi), %rdi" );
    puts ( "\tmovq $0, %rsi" );
    puts ( "\tmovq $10, %rdx" );
    puts ( "\tcall strtol" );

    /*  Now a new argument is an integer in rax */
    puts ( "\tpopq %rsi" );
    puts ( "\tpopq %rcx" );
    puts ( "\tpushq %rax" );
    puts ( "\tsubq $8, %rsi" );
    puts ( "\tloop PARSE_ARGV" );

    /* Now the arguments are in order on stack */
    for ( int arg=0; arg<MIN(6,first->nparms); arg++ )
        printf ( "\tpopq\t%s\n", record[arg] );

    puts ( "SKIP_ARGS:" );
    printf ( "\tcall\t_%s\n", first->name );
    puts ( "\tjmp END" );
    puts ( "ABORT:" );
    puts ( "\tmovq $errout, %rdi" );
    puts ( "\tcall puts" );

    puts ( "END:" );
    puts ( "\tmovq %rax, %rdi" );
    puts ( "\tcall exit" );
}

void
generate_program ( void )
{
    generate_stringtable();

    /* Put some dummy stuff to keep the skeleton from crashing */
    puts ( ".globl main" );
    puts ( ".section .text" );
    puts ( "main:" );
    puts ( "\tmovq $0, %rax" );
    puts ( "\tcall exit" );
}
