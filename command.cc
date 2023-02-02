
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include<time.h>


#include "command.h"

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}
static int pid;

void Command::execute()
{
	
	// Don't do anything if there are no simple commands
	if (_numberOfSimpleCommands == 0)
	{
		prompt();
		return;
	}
	// Print contents of Command data structure
		print();
	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec
	// Save default input, output, and error because we will
	// change them during redirection and we will need to restore them
	// at the end.
	// The dup() system call creates a copy of a file descriptor.
	int defaultin = dup( 0 );
	int defaultout = dup( 1 );
	int defaulterr = dup( 2 );
	int infd;
	int outfd;
	int errfd;
	
	if (_inputFile)
		infd= open(_inputFile , O_RDONLY);
	else
		infd = dup(defaultin);
	
	
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		// testing cd command
		int res =strcmp(_simpleCommands[0]->_arguments[0], "cd");
		if(res==0)
		{
			char * username = getenv("HOME"); // default 
				if(_simpleCommands[0]->_arguments[1]) // if there is one argument after cd
				{	
				
					const char* dirName= _simpleCommands[0]->_arguments[1];
					if (opendir(dirName)) // if directory is found
					{
  						printf("%s is a directory\n",_simpleCommands[0]->_arguments[1]);
   						chdir(_simpleCommands[0]->_arguments[1]); // cd to the given directory
  					}

					else
					{
  						perror("something went wrong");
  						exit( 2) ; // directory is not found
					}
				}
	
		else 
		{
			chdir(username); // default path is home if argument array is empty
		}
	
		continue; // don't fork a new process nor enter execvp 
		}
	
	dup2(infd, 0); // redirect i/p to infd
	close(infd);
	if (i == _numberOfSimpleCommands - 1) //executing last command 
	{
		 if (_outFile) 	// check for any given output files to post output in
		 {
		   		// create one if there isnt a found output file
			if(_currentCommand.flag ==1) 		// if output is to be appended, flag is set to 1		
			{
				if(open(_outFile, O_WRONLY)<0)
				{
				  outfd = creat(_outFile, 0666);
			 	  outfd = open(_outFile, O_WRONLY|O_APPEND);
			 	  
			  	} else outfd = open(_outFile, O_WRONLY|O_APPEND);
			}
			
			else outfd = creat( _outFile,0666); 	
		 } 
		
		else outfd = dup(defaultout); 	// no output files, post on console
		
		if(_errFile)
		{		
			if(_currentCommand.flag ==1) 
			{	if(open(_errFile, O_WRONLY|O_APPEND)<0)
				{
				  errfd = creat(_errFile, 0666);
			 	  errfd = open(_errFile, O_WRONLY|O_APPEND);
			 	  
			  	} else errfd = open(_errFile, O_WRONLY|O_APPEND);
			}
					
			else errfd = creat(_errFile, 0666); 
				
		} else errfd = dup(defaulterr);
			
	}
	else 
	{
		int fdpipe[3]; // piping
		if ( pipe(fdpipe) == -1) 
		{
			perror( "error: pipe");
			exit( 2 );				
		}

		pipe(fdpipe);
		outfd = fdpipe[1]; // copy o/p file descriptors
		infd = fdpipe[0]; // copy i/p file descriptors
		errfd = fdpipe[2]; //copy err file descriptors		
	}

		dup2(outfd, 1);  // redirect output to outfd
		close(outfd);
		// dup2( defaulterr, 2 );
		dup2(errfd, 2);
		close(errfd);
		pid = fork(); // fork a new process
		if ( pid == -1 ) 
		{
			perror( "process: fork\n");
			exit( 2 );
		}
		if (pid == 0)  // child process to execvp
		{
			execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);

			// when execvp yields negative >> invalid command
			if(execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments)<0)
					// printf("couldn't execute this command \n");
					fprintf(stderr, "%s", "execvp: command not found\n" );
					//perror("execvp");
					exit( 2 );
		}
		
	}
	

	
	dup2(defaultin, 0); 
	dup2(defaultout, 1);
	dup2(defaulterr, 2 );
	close(defaultin);
	close(defaultout);
	close(defaulterr);
	
	if (!_background)
	{
		waitpid( pid , 0 , 0 );
	}
	
		// Clear to prepare for next command
		
		clear();
	
	prompt();
	// Print new prompt
}
 //Shell implementation

void
Command::prompt()
{ 
	char cwd[1024];
	getcwd(cwd,sizeof(cwd));
	printf("myshell>%s>",cwd);
	fflush(stdout);
}

void process(int sig) 
{ 
 // waitpid(-1, NULL, WNOHANG);  // checks for terminated process
  FILE* fil;
 
  fil = fopen("/home/nouran/process_termination.log", "a+");
  if(!fil) {  
    printf("couldn't open the file !\n");
    exit(-1);
  }
  time_t t=time(NULL);
  fprintf(fil, "child %d terminated %s\n", pid,ctime(&t));  
  //fprintf(fp, "Child process was terminated\n");
  fclose(fil);

}
void  INThandler(int sig)
{
	char  c;

     signal(sig, SIG_IGN);
     printf(" exit [Y/N] ? ");
     c = getchar();
     if (c == 'y' || c == 'Y')
          exit(0);
     else
     	
        signal(SIGINT, INThandler);
     	getchar();
     	Command::_currentCommand.prompt();// Get new line character
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int 
main()

{ 
	signal (SIGCHLD,process);
	signal(SIGINT, INThandler);
	Command::_currentCommand.prompt();
	yyparse();

	
	return 0;
}

