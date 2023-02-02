
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT LESS GREATGREAT PIPE NEWLINE AMPERSAND AMPGR8GR8 AMPGR8

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include <cstring>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command
	| 
        ;


simple_command:	
	comm_list iomodifier_list NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| command_and_args NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	
	;
	
comm_list:
	comm_list PIPE command_and_args
	| command_and_args
	
	;
command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       if(!strcmp($1, "exit")){
			return 0;
		} 
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;
	
iomodifier_list:
	iomodifier_list iomodifier_opt
	| /* can be empty */
	;
	
iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	| LESS WORD {
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	} 
	| GREATGREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand.flag = 1;
		Command::_currentCommand._outFile = $2;
	}
	| AMPERSAND {
		Command::_currentCommand._background = 1;
	}
	
	| AMPGR8 WORD{
		printf("   Yacc: insert error \"%s\"\n", $2);
		Command::_currentCommand._errFile = $2;
	}
	| AMPGR8GR8 WORD{
		printf("   Yacc: insert error \"%s\"\n", $2);
		Command::_currentCommand.flag = 1;
		Command::_currentCommand._errFile = $2;
	}
		
	;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
