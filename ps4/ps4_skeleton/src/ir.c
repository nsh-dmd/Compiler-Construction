#include <vslc.h>

tlhash_t *global_names = NULL;
tlhash_t **scope_table = NULL ; //local scopes
char **string_list= NULL; //string table
size_t n_string_list = 8, stringc = 0;

size_t n_scope_table = 1;
size_t d = 0;

void
find_globals ( void )
{
    global_names = malloc( sizeof(tlhash_t) );
    tlhash_init ( global_names, 32 );
    string_list = malloc( sizeof(char*) * n_string_list );
    node_t *globals = root->children[0];
    size_t n_func = 0;

    for (uint64_t i = 0; i < globals->n_children; i++) {

        symbol_t *symbol = NULL;// = malloc(sizeof(symbol_t));
        node_t *name_list = NULL;
        node_t *global = globals->children[i];

        if (global->type == FUNCTION) {
            // printf ( "FIND_GLOBAL-->FUNCTION:**************\n" );
            symbol = malloc(sizeof(symbol_t));
            // function
            // symbol = malloc(sizeof(symbol_t));
            symbol->nparms = 0;
            symbol->seq = n_func;
            symbol->name = global->children[0]->data;
            symbol->locals = malloc(sizeof(tlhash_t));
            symbol->type = SYM_FUNCTION;
            symbol->node = global->children[2];

            n_func++;

            // local table for parameters
            tlhash_init(symbol->locals, 32);
            if (global->children[1]) {

                symbol->nparms = global->children[1]->n_children;

                for (uint64_t j = 0; j < symbol->nparms; j++) {

                    node_t *parameter =  global->children[1]->children[j];

                    symbol_t *parameter_symbol = malloc(sizeof(symbol_t));
                    parameter_symbol->nparms = 0;
                    parameter_symbol->seq = j;
                    parameter_symbol->name = parameter->data;
                    parameter_symbol->locals = NULL;
                    parameter_symbol->type = SYM_PARAMETER;
                    parameter_symbol->node = NULL;

                    tlhash_insert(symbol->locals, parameter_symbol->name, strlen(parameter_symbol->name), parameter_symbol);
                }
            }
            tlhash_insert(global_names, symbol->name, strlen(symbol->name), symbol);
        }

        else if(global->type == DECLARATION) {
            // printf ( "FIND_GLOBAL-->DECLARATION:**************\n" );

            name_list = global->children[0];

            for (size_t j = 0; j < name_list->n_children; j++) {
                symbol = malloc(sizeof(symbol_t));
                symbol->nparms = 0;
                symbol->seq = 0;
                symbol->name = name_list->children[j]->data;
                symbol->locals = NULL;
                symbol->type = SYM_GLOBAL_VAR;
                symbol->node = NULL;
            }
            tlhash_insert(global_names, symbol->name, strlen(symbol->name), symbol);
        }
    }
}


void
bind_names ( symbol_t *function, node_t *root )
{
    // printf ( "BIND_NAMES:**************\n" );

    if (root == NULL) return;
    else {

        node_t *name_list=NULL;
        symbol_t *symbol_entry=NULL;

        // scope_table = malloc( sizeof(tlhash_t*) * n_scope_table );

        if (root->type == BLOCK) {
            // printf ( "BIND_NAMES-->BLOCK:**************\n" );
            if (scope_table==NULL) {
                scope_table = malloc ( n_scope_table * sizeof(tlhash_t*) );
            }
            /* for nested scopes */

            tlhash_t *scope = malloc(sizeof(tlhash_t));
            tlhash_init(scope, 32);
            scope_table[d] = scope;
            //add a new scope to table
            d++;
            if ( d >= n_scope_table ) {
                n_scope_table *= 2;
                scope_table = realloc( scope_table, n_scope_table * sizeof(tlhash_t**) );
            }

            for (size_t i = 0; i < root->n_children; i++) {
                bind_names(function, root->children[i]);
            }
            //pop
            d--;
            tlhash_finalize(scope_table[d]);
            free(scope_table[d]);
            scope_table[d]=NULL;
        }

        else if( root->type == DECLARATION ) {
            // printf ( "BIND_NAMES-->DECLARATION:**************\n" );

            name_list = root->children[0];

            for (uint64_t i = 0; i < name_list->n_children; i++) {
                // printf ( "BIND_NAMES-->loop0:**************\n" );
                size_t local_n = tlhash_size(function->locals) - function->nparms;
                node_t *variable_name = name_list->children[i];

                symbol_t *symbol = malloc(sizeof(symbol_t));
                symbol->nparms = 0;
                symbol->seq = local_n;
                symbol->name = variable_name->data;
                symbol->locals = NULL;
                symbol->type = SYM_LOCAL_VAR;
                symbol->node = NULL;
                // printf ( "BIND_NAMES-->loop1:**************\n" );

                tlhash_insert(function->locals, &local_n, sizeof(size_t), symbol);
                // printf ( "BIND_NAMES-->loop2:**************\n" );

                tlhash_insert(scope_table[d-1], symbol->name, strlen(symbol->name), symbol);
                // printf ( "BIND_NAMES-->loop3:**************\n" );

            }
        }

        else if (root->type == IDENTIFIER_DATA) {
            // printf ( "BIND_NAMES-->IDENTIFIER_DATA:**************\n" );
            symbol_t *s = NULL;
        //    lookup in the table
            // for (size_t i = d-1; i <= 0; i--) {
            //     if (symbol_entry!=NULL) break;
            //     tlhash_lookup(scope_table[i], root->data, strlen(root->data), (void **)&s);
            // }
            size_t d_loc = d;
            while ( s == NULL && d_loc > 0) {
                // printf ( "BIND_NAMES-->lookup:**************\nd_loc=%d\n",d_loc );
                d_loc--;
                // printf ( "BIND_NAMES-->lookup:**************\nd_loc=%d\n",d_loc );

                tlhash_lookup(scope_table[d_loc], root->data, strlen(root->data), (void **)&s);
                // printf ( "BIND_NAMES-->lookup:**************\nn2=%d\n",d_loc );
            }

            symbol_entry = s;

            // a parameter
            if (symbol_entry==NULL) {
                // printf ( "BIND_NAMES-->parameter:**************\n" );
                tlhash_lookup(function->locals, root->data, strlen(root->data), (void**)&symbol_entry);
            }
            // a global name
            if (symbol_entry==NULL) {
                // printf ( "BIND_NAMES-->global name:**************\n" );
                tlhash_lookup(global_names, root->data, strlen(root->data), (void**)&symbol_entry);
            }
            // exit on failure
            // if (symbol_entry==NULL) {
            //     // printf ( "BIND_NAMES-->failure:**************\n" );
            //     exit(0);
            // }
            root->entry = symbol_entry;
        }

        else if (root->type == STRING_DATA) {
            // printf ( "BIND_NAMES-->STRING_DATA:**************\n" );

            //legge til string og index
            string_list[stringc] = root->data;
            root->data = malloc( sizeof(size_t) );
            *((size_t*)root->data) = stringc;
            stringc++;

            if (n_string_list <= stringc) {
                n_string_list *= 2;
                string_list = realloc(string_list, n_string_list * sizeof(char*));
            }
        }

        else {
            for (size_t i = 0; i < root->n_children; i++) {
                bind_names(function, root->children[i]);
            }
        }
    }
}


void
destroy_symtab ( void )
{
    // printf ( "DESTROY:**************\n" );

    // destroy string_list
    for (size_t i = 0; i < stringc; i++) {
        free(string_list[i]);
    }
    free(string_list);
    size_t n = tlhash_size(global_names);
    symbol_t *globs[n];// = malloc( tlhash_size(global_names) * sizeof(symbol_t *) );

    tlhash_values(global_names, (void**)&globs);

    for (size_t i = 0; i < n; i++) {
        symbol_t *g =  globs[i];
        // printf ( "DESTROY-->loop:**************\n" );

        if (g->locals!=NULL) {
            // printf ( "DESTROY-->loop-->locals:**************\n" );
            size_t n_g = tlhash_size(g->locals);
            symbol_t *local_symbols[n_g];// = malloc (tlhash_size(g->locals) * sizeof(symbol_t *));
            tlhash_values(g->locals, (void**)&local_symbols);

            for (size_t j = 0; j < n_g; j++) {
                free(local_symbols[j]);
            }
            tlhash_finalize(g->locals);
            free(g->locals);
        }
        free(g);
    }

    tlhash_finalize ( global_names );
    free ( global_names );
    free(scope_table);
}


void
print_symbols ( void )
{
    printf ( "String table:\n" );
    for ( size_t s=0; s<stringc; s++ )
        printf  ( "%zu: %s\n", s, string_list[s] );
    printf ( "-- \n" );

    printf ( "Globals:\n" );
    size_t n_globals = tlhash_size(global_names);
    symbol_t *global_list[n_globals];
    tlhash_values ( global_names, (void **)&global_list );
    for ( size_t g=0; g<n_globals; g++ )
    {
        switch ( global_list[g]->type )
        {
            case SYM_FUNCTION:
                printf (
                    "%s: function %zu:\n",
                    global_list[g]->name, global_list[g]->seq
                );
                if ( global_list[g]->locals != NULL )
                {
                    size_t localsize = tlhash_size( global_list[g]->locals );
                    printf (
                        "\t%zu local variables, %zu are parameters:\n",
                        localsize, global_list[g]->nparms
                    );
                    symbol_t *locals[localsize];
                    tlhash_values(global_list[g]->locals, (void **)locals );
                    for ( size_t i=0; i<localsize; i++ )
                    {
                        printf ( "\t%s: ", locals[i]->name );
                        switch ( locals[i]->type )
                        {
                            case SYM_PARAMETER:
                                printf ( "parameter %zu\n", locals[i]->seq );
                                break;
                            case SYM_LOCAL_VAR:
                                printf ( "local var %zu\n", locals[i]->seq );
                                break;
                        }
                    }
                }
                break;
            case SYM_GLOBAL_VAR:
                printf ( "%s: global variable\n", global_list[g]->name );
                break;
        }
    }
    printf ( "-- \n" );
}


void
print_bindings ( node_t *root )
{
    if ( root == NULL )
        return;
    else if ( root->entry != NULL )
    {
        switch ( root->entry->type )
        {
            case SYM_GLOBAL_VAR:
                printf ( "Linked global var '%s'\n", root->entry->name );
                break;
            case SYM_FUNCTION:
                printf ( "Linked function %zu ('%s')\n",
                    root->entry->seq, root->entry->name
                );
                break;
            case SYM_PARAMETER:
                printf ( "Linked parameter %zu ('%s')\n",
                    root->entry->seq, root->entry->name
                );
                break;
            case SYM_LOCAL_VAR:
                printf ( "Linked local var %zu ('%s')\n",
                    root->entry->seq, root->entry->name
                );
                break;
        }
    } else if ( root->type == STRING_DATA ) {
        size_t string_index = *((size_t *)root->data);
        if ( string_index < stringc )
            printf ( "Linked string %zu\n", *((size_t *)root->data) );
        else
            printf ( "(Not an indexed string)\n" );
    }
    for ( size_t c=0; c<root->n_children; c++ )
        print_bindings ( root->children[c] );
}
