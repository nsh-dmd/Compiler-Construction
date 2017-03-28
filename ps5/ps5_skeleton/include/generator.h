#ifndef GENERATOR_H
#define GENERATOR_H
void generate_program ( void );
#endif

#define MIN(a,b) (((a)<(b)) ? (a):(b))
#define ASM1(op, a) puts("\t"#op"\t"#a)
#define ASM2(op, a, b) puts("\t"#op"\t"#a", "#b)
#define FASM3(op, a, b, type) printf("\t"#op"\t$"#type", "#a"\n", #b);

static const char *record[6] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};
static void add_exp(node_t *node, symbol_t *func);
static void mult_exp(node_t *node, symbol_t *func);
static void sub_exp(node_t *node, symbol_t *func);
static void div_exp(node_t *node, symbol_t *func);
static void generate_function_call ( symbol_t *function );
static void generate_subexpression (node_t *node, symbol_t *function );
static void generate_print(node_t *node, symbol_t *func);
static void generate_assignment(node_t *node, symbol_t *func);
static void generate_identifier (node_t *node, symbol_t *function);
static void allocate_args ( size_t n_args );
static void generate_global_variables ( void );
static void generate_stringtable ( void );
static void generate_main ( symbol_t *first );
