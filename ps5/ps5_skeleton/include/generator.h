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
