#ifndef GENERATOR_H
#define GENERATOR_H
void generate_program ( void );
#endif

#define MIN(a,b) (((a)<(b)) ? (a):(b))
#define ASM1(op, a) puts("\t"#op"\t"#a)
#define ASM2(op, a, b) puts("\t"#op"\t"#a", "#b)
#define FASM2(op, a, b) printf("\t"#op"\t$%l64d, "#a"\n", #b);

static const char *record[6] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};
