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
#define MAX_PIDLIST_SIZE 10
#define MAX_HISTORY_SIZE 15

/* Mav shell only supports a command with 10 parameters */
#define MAX_NUM_ARGUEMENTS 11

int tokenize_cmd( char *cmd_str, char **token );
int add_history_entry( char *cmd_str, char **history, int history_count );
int add_pid_entry( pid_t pid, pid_t *pidlist, int pid_count );
void cleanup_token( char **token, int token_count );
void cleanup_history( char **history, int history_count );

int main()
{
    /* Block SIGINT and SIGTSTP in msh process */
    sigset_t newmask;
    sigemptyset( &newmask );
    sigaddset( &newmask, SIGINT );
    sigaddset( &newmask, SIGTSTP );
    if( sigprocmask( SIG_BLOCK, &newmask, NULL ) < 0 )
    {
        perror( "sigprocmask: " );
    }

    char * cmd_str = (char *) malloc( MAX_COMMAND_SIZE );

    /* Track history of last 10 commands */
    char * history[MAX_HISTORY_SIZE];
    int history_count = 0;

    /* 
     * Track if executing a command from history
     * 
     * cmd_from_history: represents a boolean value
     * 0: NOT executing cmd from history
     * 1: executing cmd from history
     * 
     * history_index: the index from history log to execute
     * default value: -1 to indicate no index has been set
     */
    int cmd_from_history = 0;
    int history_index = -1;

    /* Track PIDs for the last 10 processes spawned */
    pid_t pidlist[MAX_PIDLIST_SIZE];
    int pid_count = 0;

    /* Track suspended process: -1 indicates no current process suspended */
    pid_t tstp_pid = -1;

    while( 1 )
    {

        if( !cmd_from_history )
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
        }
        else
        {
            char * entry_from_history = history[history_index];
            strncpy( cmd_str, entry_from_history, MAX_COMMAND_SIZE );
        }

        /* reset cmd_from_history to false to prevent an endless loop */
        cmd_from_history = 0;

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
        else if ( strcmp( cmd, "showpids\0" ) == 0 )
        {
            /* Prints a log of PIDs of the last 10 processes spawned */
            int i;
            for( i = 0; i < pid_count; i++ )
            {
                printf("%i: %d\n", i, pidlist[i]);
            }
        }
        else if( strcmp( cmd, "cd\0" ) == 0 )
        {
            /* change directories */
            int failed = chdir(token[1]);
            if( failed )
            {
                printf("-msh: cd: %s: No such file or directory\n", token[1]);
            }
        }
        else if( strcmp( cmd, "bg\0" ) == 0 )
        {
            /* background suspended process if one exists */
            if( tstp_pid != -1 )
            {
                kill( tstp_pid, SIGCONT );
            }
            else
            {
                printf( "-msh: bg: No suspended process found\n" );
            }
        }
        else if( ( strcmp( cmd, "\0" ) != 0 ) && ( cmd[0] != '!' ) )
        {
            /* execute a command with the given arguements */
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
                /* Unblock SIGINT and SIGTSTP signals for child process */
                if( sigprocmask( SIG_UNBLOCK, &newmask, NULL ) < 0 )
                {
                    perror( "sigprocmask: " );
                }
        
                execv(location1, token);
                execv(location2, token);
                execv(location3, token);
                execv(location4, token);
                printf("%s: Command not found\n", cmd);
                exit( EXIT_SUCCESS );
            }
        
            waitpid( child_pid, &status, WUNTRACED );
            if( WIFSTOPPED( status ) && ( tstp_pid == -1 ) )
            {
                /* save the PID of suspended process */
                tstp_pid = child_pid;
            }
            else if( WIFSTOPPED( status ) && ( tstp_pid != -1 ) )
            {
                /* kill currently suspended process before storing a new one */
                kill( tstp_pid, SIGINT );
                tstp_pid = child_pid;
            }

            pid_count = add_pid_entry(child_pid, pidlist, pid_count);
        }
        else if ( ( strcmp( cmd, "\0" ) != 0 ) && ( cmd[0] == '!' ) )
        {
            /* determine if command from history to be executed is a valid entry */
            history_index = cmd[1] - 48;    // ASCII char to decimal value
            if( ( history_index >= 0 ) && ( history_index < history_count ) )
            {
                /* command from history is a valid entry */
                cmd_from_history = 1;
            }
            else
            {
                printf("Command not in history\n");
                history_index = -1;
            }
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
    /* add new entry if not the "!n" command */
    if( cmd_str[0] != '!' )
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
    }

    return history_count;
}

/*
 * Name: add_pid_entry
 * Parameters:
 *      pid_t pid (pid to add to list)
 *      pid_t *pidlist (array to store pid entries in)
 *      int pid_count (running count of pid entries - stops at 10)
 * Return Value:
 *      returns int with # of pid entries
 * Description: Manages adding entries into pid history log. After 10 entries,
 *              entries are all shifted up one (oldest entry is overwritten)
 *              and the new one is added to the bottom of the log
 */
int add_pid_entry( pid_t pid, pid_t *pidlist, int pid_count )
{
    if( pid_count < MAX_HISTORY_SIZE )
    {
        pidlist[pid_count] = pid;
        pid_count++;
    }
    else
    /* shift entries up 1 entry to make room for the new entry */
    {
        int i;
        for( i = 1; i < MAX_HISTORY_SIZE; i++ )
        {
            pidlist[i-1] = pidlist[i];
        } 
        pidlist[MAX_HISTORY_SIZE - 1] = pid;
    }

    return pid_count;
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
