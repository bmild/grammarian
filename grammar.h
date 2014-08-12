#ifndef GRAMMAR_H
#define GRAMMAR_H
#include "cfg.h"
#include "symbol.h"
using namespace std;

struct Parse_data {
    union {
        CFG<int> *cfg;
        vector<Production<int>> *productions;
        vector<Symbols *> *rhs_list;
        Symbols *rhs;
    };
    int prec;
    int assoc;
    Symbols *precsym;
    SymMap *pm;
    SymMap *am;

    Symbol sym;
};
typedef vector<Parse_data*> VPD;


CFG<Parse_data> *grammar_grammar();
CFG<int> *read_grammar(string input, SymbolTable *symtab);

#endif