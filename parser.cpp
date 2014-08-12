#include "symbol.h"
#include "cfg.h"
#include "regex.h"
#include "grammar.h"
#include "lexer.h"
#include <sstream>

void regex_test() {
    while(true) {
        cout << "Input a regex: ";
        string str;
        cin >> str;
        Regex_eval rgx(str, true);
        while (true) {
            cin >> str;
            if (str == "n")
                break;
            if (str == "q")
                return;
            cout << (rgx.accept(str)?"Accept":"Reject") << endl; 
        }
    }
}

string file_to_string(const char *filename) {
    ifstream file(filename);
    stringstream ss;
    ss << file.rdbuf();
    file.close();
    return ss.str();
}

int main(int argc, char *argv[]) {
    //regex_test();
    /*
    CFG<Parse_data> *cfg = read_grammar(ss.str());
    if (cfg)
        cout << endl << cfg->str() << endl << endl;
        */
    SymbolTable *symtab = new SymbolTable();
    Lexer<int> *lexer = read_lexer(file_to_string(argv[1]), symtab);
    CFG<int> *cfg = read_grammar(file_to_string(argv[2]), symtab);
    //cout << cfg->dfa->str() << endl << endl;
    cfg->parse(lexer->lex(file_to_string(argv[3]), true, true), true);

    // for (int i = 0; i < lexer->rules.size()-2; ++i) {
    //     cout << lexer->rules[i].regex.dfa->str() << endl;
    // }
    
    return 0;
}
