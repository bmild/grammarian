#include "cfg.h"
#include "automata.h"

/*

CFG<float> statement_blocks = {
    P,                  // Start
    { P, S, B },        // Nonterminals
    { t, ob, cb, col }, // Terminals
    //Productions
    {
        {P, {S} },
        {S, {t} },
        {S, {ob, B} },
        {B, {cb} },
        {B, {S, cb} },
        {B, {S, col, B} }
    }
};

CFG<float> basic_arithmetic = {
    E,                      // Start
    { E, T, S },                // Nonterminals
    { mul, plu, i, op, cp },    // Terminals
    //Productions
    {
        {E, {T, plu, E} },
        {E, {T} },
        {T, {S, mul, T} },
        {T, {S} },
        {S, {i} },
        {S, {op, E, cp} }
    }
};

CFG<float> basic_arith_alt = {
    E,                          // Start
    { E, T, X, Y },             // Nonterminals
    { mul, plu, op, cp, i },    // Terminals
    //Productions
    {
        {E, {T, X} },
        {T, {op, E, cp} },
        {T, {i, Y} },
        {X, {plu, E} },
        {X, {} },
        {Y, {mul, T} },
        {Y, {} }
    }
};

CFG<float> regex_gram {
    E,                                  // Start
        { E, D, T, S, F, G, P },            // Nonterminals
            { mul, plu, a, b, c, op, cp },      // Terminals
                //Productions
                {
                    {E, {T, D} },
                        {D, {plu, E} },
                            {D, {} },
                                {T, {F, S} },
                                    {S, {T} },
                                        {S, {} },
                                            {F, {P, G} },
                                                {G, {mul, G} },
                                                    {G, {} },
                                                        {P, {op, E, cp} },
                                                            {P, {a} },
                                                                {P, {b} },
                                                                    {P,
                                                                            {c}
    }
                }
};

CFG<float> balanced_parens {
    P,
        { P },
            { op, cp },
                {
                    {P, {op, P, cp, P} },
                        {P, {P, op, P, cp} },
                            {P, {} }
                }
};

CFG<float> ambiguous {
    E, { E }, { i, plu, mul, op, cp },
                  {{E, {E, mul, E}},{E,{E,plu,E}}, {E, {op, E, cp}}, {E, {i}}, {P,
                                                                {plu}},
                                                                      {P,
    {mul}}},
                         {mul,plu}, {{mul,LEFT},{plu,LEFT}}
};

CFG<float> ambiguous2 {
    E, { E }, { i, plu, mul, op, cp },
                  {{E, {
                              {{E, mul, E}}, 
                              {{E,plu,E}}, 
                              {{op,E,cp}}, 
                              {{i}}
                          } 
                   }},

                         {mul,plu}, {{mul,LEFT},{plu,LEFT}}
};

*/

