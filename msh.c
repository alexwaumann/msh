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
#define MAX_HISTORY_SIZE 10

/* Mav shell only supports a command with 10 parameters */
#define MAX_NUM_ARGUEMENTS 11

int tokenize_cmd( char *cmd_str, char **token );
int add_history_entry( char *cmd_str, char **history, int history_count );
void cleanup_token( char **token, int token_count );
void cleanup_history( char **history, int history_count );

int main()
{
    char * cmd_str = (char *) malloc( MAX_COMMAND_SIZE );

    /* Track history of last 10 commands */
    char * history[MAX_HISTORY_SIZE];
    int history_count = 0;

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
        char * token[MAX_NUM_ARGUEMENTS + 1];   // extra spot for NULL pointer
        int token_count = tokenize_cmd(cmd_str, token);

        /* update the history with the command */
        history_count = add_history_entry( cmd_str, history, history_count );

        /* cmd contains the command from command line string */
        char * cmd = token[0];

        if( ( strcmp( cmd, "exit\0") == 0 ) || ( strcmp( cmd, "quit\0" ) == 0 ) )
        {
            cleanup_token( token, token_count );
            break;
        }
        else if( strcmp( cmd, "history\0" ) == 0 )
        {
            /* Print the history log */
            int i;
            for( i = 0; i < history_count; i++ )
            {
                printf("%i: %s", i, history[i]);
            }
        }
        else if( strcmp( cmd, "\0" ) != 0 )
        {
            
            char location1[MAX_COMMAND_SIZE] = "./";
            char location2[MAX_COMMAND_SIZE] = "/usr/local/bin/";
            char location3[MAX_COMMAND_SIZE] = "/usr/bin/";
            char location4[MAX_COMMAND_SIZE] = "/bin/";

            strcat(location1, cmd);
            strcat(location2, cmd);
            strcat(location3, cmd);
            strcat(location4, cmd);

            pid_t child_pid = fork();
            int status;

            if( child_pid == 0 )
            {
                execv(location1, token);
                execv(location2, token);
                execv(location3, token);
                execv(location4, token);
                printf("%s: Command not found\n", cmd);
                exit( EXIT_SUCCESS );
            }

            waitpid( child_pid, &status, 0 );
        }

        /* free allocated data in token array */
        cleanup_token( token, token_count );
    }

    free( cmd_str );
    /* free allocated data in history array */
    cleanup_history( history, history_count );

    return 0;
}

/*
 * Name: tokenize_cmd
 * Parameters:
 *      char *cmd_str (command line arguments string)
 *      char **token  (array to store tokens in)
 * Return Value:
 *      returns int with # of valid tokens
 * Description: Tokenizes the command line string using
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
        if( strcmp(arg_ptr, "") != 0 )
        {
            token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
            token_count++;
        }
        arg_count++;
    }

    /* 
     * Append a NULL pointer; will never be out of bounds since
     * token_count is capped at MAX_NUM_ARGUEMENTS, which is
     * 1 less than the allocated spots in char * token[]
     */
    token[token_count] = NULL;

    /*
     * If there were no valid tokens then add an empty
     * string for token[0] to indicate an empty command
     */
    if( token_count == 0 )
    {
        token[0] = "\0";
        token[1] = NULL;
    }

    free( working_root );
    return token_count;
}

/*
 * Name: add_history_entry
 * Parameters:
 *      char *cmd_str (command line arguements string)
 *      char **history (array to store history entries in)
 *      int history_count (running count of history entries - stops at 10)
 * Return Value:
 *      returns int with # of history entries
 * Description: Manages adding entries into history log. After 10 entries,
 *              entries are all shifted up one (oldest entry is deleted/freed)
 *              and the new one is added to the bottom of the log
 */
int add_history_entry( char *cmd_str, char **history, int history_count )
{
    /* 
     * allocate new memory to store the command since cmd_str
     * will be overwritten on next command
     */
    char * cmd = strndup( cmd_str, MAX_COMMAND_SIZE );

    if( history_count < MAX_HISTORY_SIZE )
    {
        history[history_count] = cmd;
        history_count++;
    }
    else
    /* shift entries up 1 entry to make room for the new entry */
    {
        int i;
        free ( history[0] );
        for( i = 1; i < MAX_HISTORY_SIZE; i++ )
        {
            history[i-1] = history[i];
        } 
        history[MAX_HISTORY_SIZE - 1] = cmd;
    }

    return history_count;
}

/*
 * Name: cleanup_token
 * Parameters:
 *      char **token (array where tokens are stored)
 *      int token_count (# of tokens allocated)
 * Return Value:
 *      no return value
 * Description: Frees the dynamically allocated data in **token
 */
void cleanup_token( char**token, int token_count )
{
    int i;
    for( i = 0; i < token_count; i++ ) {
        free( token[i] );
    }
}

/*
 * Name: cleanup_history
 * Parameters:
 *      char **history (array where history entries are stored)
 *      int history_count (# of history entries allocated)
 * Return Value:
 *      no return value
 * Description: Frees the dynamically allocated data in **history
 */
void cleanup_history( char **history, int history_count )
{
    int i;
    for( i = 0; i < history_count; i++ ) {
        free( history[i] );
    }
}
