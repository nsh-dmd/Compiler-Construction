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

  for ( size_t i = 0; i < MIN(n_args, 6); i++)
      ASM1(pushq, record[i]);

  if (n_args > 6) {
      // i = argument index for remaining args not fitting into registers
      size_t r = n_args - 6;
      for (size_t i = 6; i < r; i++) {
          ASM1(pushq, %rbp + 8*i);
      }
  }
}

static void generate_function_call ( symbol_t *function ) {

  printf( "_%s:\n", function->name );
  ASM1(pushq, %rbp);
  ASM2(movq, %rsp, %rbp);

  // make space arguments on stack
  allocate_args( function->nparms );

  // local variables
  ASM1( subq, 8 * tlhash_size(function->locals) );

  node_t *func_node = function->node;

  switch (func_node->type) {
      case PRINT_STATEMENT:
      for ( size_t i=0; i<func_node->n_children; i++ ) {
          generate_print(func_node->children[i], function);
      }
      ASM2 ( movq, $'\n', %rdi );
      ASM1 ( call, putchar );
      break;

      case EXPRESSION:
      generate_subexpression(func_node, function);
      break;

      case ASSIGNMENT_STATEMENT:
      generate_assignment(func_node, function);
      break;

      case RETURN_STATEMENT:
      generate_subexpression(func_node->children[0], function);
      puts("leave");
      puts("ret");
      break;
  }

}

static void generate_identifier (node_t node, symbol_t *function) {
    if (node->entry == SYM_GLOBAL_VAR) {
        printf("\tmovq\t_%s, ", node->entry->name);
    }
    else if (node->entry == SYM_LOCAL_VAR) {
        printf("\tmovq\t%ld(%%rbp)\n", -8 * (function->nparms + node->entry->seq + 1) );
    }
    else if (node->entry == SYM_PARAMETER) {
        if (node->entry->seq < 7) {
            printf("\tmovq\t%ld(%%rbp)\n", record[node->entry->seq] );
        }
        else {
            printf("\tmovq\t%ld(%%rbp)\n", -8 * (node->entry->seq + 6) );
        }
    }
}
static void add_exp(node_t *node, symbol_t *func) {
    generate_subexpression(node->children[0], func);

    ASM1(pushq, %rax);
    generate_subexpression(node->children[1], func);
    ASM2(addq, %rax, %rsp);

    ASM1(popq, %rax);
}

static void mult_exp(node_t *node, symbol_t *func) {
    ASM1(pushq, %rdx);
    generate_subexpression(node->children[1], func);

    ASM1(pushq, %rax);
    generate_subexpression(node->children[0], func);
    ASM0(cqo);
    ASM1(imulq, %rsp);

    ASM1(popq, %rdx);
}

static void sub_exp(node_t *node, symbol_t *func) {

    generate_subexpression(node->children[0], func);

    ASM1(pushq, %rax);
    generate_subexpression(node->children[1], func);
    ASM2(subq, %rax, %rsp);

    ASM1(popq, %rax);
}

static void div_exp(node_t *node, symbol_t *func) {
    ASM1(pushq, %rdx);
    generate_subexpression(node->children[1], func);

    ASM1(pushq, %rax);
    generate_subexpression(node->children[0], func);
    ASM0 ( cqo );
    ASM1 ( idivq, %rsp );

    ASM1 ( popq, %rdx );
}

/* puts expression result in %rax */
static void generate_subexpression (node_t *node, symbol_t *function ) {

    // numbers translate into setting them in %rax
    if (node->type == NUMBER_DATA) {
        FASM3(movq, %rax, *(int64_t *)node->data, %l64d);
    }
    // variables translate into copying their contents to %rax
    else if (node->type == IDENTIFIER_DATA) {
        // if IDENTIFIER_DATA then it can be global, local || parameter variable
        generate_identifier(node, function);
        printf(", %%rax\n");
    }

    // arithmetic *, /, +, -
    else if (node->n_children == 2) {
        if (node->data) {

            switch (*(char*)node->data) {
                case '*':
                    mult_exp(node, function);
                    break;
                case '/':
                    div_exp(node, function);
                    break;
                case '+':
                    add_exp(node, function);
                    break;
                case '-':
                    sub_exp(node, function);
                    break;
            }
        }
    }

    // negation
    if (node->n_children == 1) {
        generate_subexpression(node->children[0], first_function);
        ASM1(negq, %rax);
    }


}


/* Generate code for r.h.s expression */
static void generate_assignment ( node_t *node, symbol_t func ) {
    generate_subexpression(node, func);
    generate_identifier(node, func);
}

static void generate_print(node_t node, symbol_t func) {

    switch( node->type ) {
        case STRING_DATA:
        printf("\tmovq\t$STR%zu, %%rsi\n", *((size_t *)node->data));
        ASM2(movq, $strout, %rdi);
        ASM1 (call, printf);
        break;

        case NUMBER_DATA:
        printf("\tmovq\t$%lld, %%rsi\n", *((int64_t *)node->data) );
        ASM2(movq, $intout, %rdi);
        ASM1(call, printf);
        break;

        case IDENTIFIER_DATA:
        generate_identifier ( node, func );
        printf ( "%%rsi\n" );
        ASM2 ( movq, $intout, %rdi );
        ASM1 ( call, printf );
        break;

        case EXPRESSION:
        generate_subexpression ( node, func );
        ASM2 ( movq, %rax, %rsi );
        ASM2 ( movq, $intout, %rdi );
        ASM1 ( call, printf );
        break;
    }
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
    symbol_t *func;
    symbol_t *globals[tlhash_size(global_names)];
    tlhash_values(global_names, (void **)&globals);
    size_t n = tlhash_size(global_names);

    for ( size_t i = 0; i < n; i++ ) {
        if ( globals[i]->type == SYM_FUNCTION && globals[i]->seq == 0 ) {
            first_function = globals[i];
            break;
        }
    }

    generate_stringtable();
    generate_global_variables();
    generate_main(func);
    for (size_t i = 0; i < n; i++) {
        if (globals[i] == SYM_FUNCTION) {
            generate_function_call(globals[i]);
        }
    }
}
