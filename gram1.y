%left '*' '/'
%left '+' '-'
%right '='
%%
program : functions ;
functions : functions function | function ;
function : type name '(' arg_list ')' '{' exprs '}' ;
type : name ;
arg_list : arg_list_ | ;
arg_list_ : arg_list_ ',' arg | arg ;
arg : type name ;
exprs : exprs expr ';' | ;
expr : 
	expr '=' expr | 
	'(' expr ')' | 
	name '(' expr_args ')' |
	expr '+' expr | 
	expr '-' expr | 
	expr '*' expr | 
	expr '/' expr | 
	name | 
	int
	;
expr_args : expr_args_ | ;
expr_args_ : expr_args ',' expr | expr ;