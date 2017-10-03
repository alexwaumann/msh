/*

    Name: Alejandro Waumann
    ID: 1000944576
    Using Trevor Bakker's code for parsing user input into tokens:
    https://github.com/CSE3320/Shell-Assignment/blob/master/mfs.c
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESAPCE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command line size

#define MAX_NUM_ARGUEMENTS 11   // Mav shell only supports a command with 10 parameters

int main()
{

    char * cmd_str = (char *) malloc( MAX_COMMAND_SIZE );

    while( 1 )
    {
        // Print out the msh prompt
        printf("msh> ");

        /*
         * Read the command from the commandline. The
         * maximum command that will be read is MAX_COMMAND_SIZE
         * This while command will wait here until the user
         * inputs something since fgets returns NULL when there
         * is no input
         */
        while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

        // Parse input
        char * token[MAX_NUM_ARGUEMENTS];

        int token_count = 0;

        // Pointer to point to the token
        // parsed by strsep
        char * arg_ptr;

        char * working_str = strdup( cmd_str );

        /*
         * we are going to move the working_str pointer so
         * keep track of its original value so we can deallocate
         * the correct amount at the end
         */
        char * working_root = working_str;

        // Tokenize the input strings with whitespace used as the delimiter
        while( ( (arg_ptr = strsep( &working_str, WHITESAPCE )) != NULL) &&
                                  (token_count < MAX_NUM_ARGUEMENTS) )
        {
            token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
            if( strlen( token[token_count] ) == 0 )
            {
                token[token_count] = NULL;
            }
            token_count++;
        }

        free( working_root );

        // check if any arguments first
        // exit program on 'exit' or 'quit' cmd
        if( ( strcmp( token[0], "exit" ) == 0 ) ||
            ( strcmp( token[0], "quit" ) == 0 ) )
        {
            // exit while loop and free memory
            // exit(0) and return(0) are equivalent
            break;
        }

    }
    return 0;
}