/*
 * Name: Alejandro Waumann
 * ID: 1000944576
 */

#define _GNU_SOURCE 1
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

/*
 * We will use WHITESPACE as the delimiter when we split
 * the command line arguments into tokens
 */
#define WHITESPACE " \t\n"

#define MAX_COMMAND_SIZE 255

/* Mav shell only supports a command with 10 parameters */
#define MAX_NUM_ARGUEMENTS 11

int tokenize_cmd( char *cmd_str, char **token);

int main()
{
    char * cmd_str = (char *) malloc( MAX_COMMAND_SIZE );

    while( 1 )
    {
        /* msh command prompt */
        printf("msh> ");

        /*
         * Read the command from the command line. This while command
         * will wait here until the user inputs something since fgets
         * returns NULL when there is no input
         */
        memset(cmd_str, '\0', MAX_COMMAND_SIZE);
        while( !fgets( cmd_str, MAX_COMMAND_SIZE, stdin ) );

        /* Parse input */
        char * token[MAX_NUM_ARGUEMENTS];
        int token_count = tokenize_cmd(cmd_str, token);
        /* cmd contains the command from command line string */
        char * cmd = token[0];

        if( ( strcmp( cmd, "exit") == 0 ) || ( strcmp( cmd, "quit" ) == 0 ) )
        {
            // cleanup function
            break;
        }
    }

    free( cmd_str );
}

/*
 * Name: tokenize_cmd
 * Parameters:
 *      char *cmd_str (command line arguments string)
 *      char **token  (array to store tokens in)
 * Return Value:
 *      returns int with # of valid tokens
 * Description: tokenizes the command line string using
 *              WHITESPACE as the delimiter
 */
int tokenize_cmd( char *cmd_str, char **token )
{

    /* arg_ptr will point to the token parsed by strsep */
    char * arg_ptr;
    char * working_str = strdup( cmd_str );

    /*
     * track the original allocated space to deallocate the correct
     * amount of space since the working_str pointer will be moving
     */
    char * working_root = working_str;

    /* Tokenize cmd_str with WHITESPACE as the demiliter */
    int arg_count = 0;      // tracks # of total of arguments
    int token_count = 0;    // tracks # of valid arguments
    while( ( (arg_ptr = strsep( &working_str, WHITESPACE )) != NULL ) &&
             ( arg_count < MAX_NUM_ARGUEMENTS ) )
    {
        char * temp = strndup( arg_ptr, MAX_COMMAND_SIZE );
        if( strcmp(temp, "") != 0 )
        {
            token[token_count] = temp;
            token_count++;
        }
        else
        {
            free( temp );   // no need to keep temp's value
        }
        arg_count++;
    }

    free( working_root );
    return token_count;
}
