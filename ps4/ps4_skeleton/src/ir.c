#include <vslc.h>

tlhash_t *global_names;
char **string_list;
size_t n_string_list = 8, stringc = 0;

void find_globals ( void );
void print_symbols ( void );
void destroy_symtab ( void );
void bind_names ( symbol_t *function, node_t *root );

void
find_globals ( void )
{
    global_names = malloc ( sizeof(tlhash_t) );
    tlhash_init ( global_names, 32 );
    string_list = malloc( sizeof(char) * n_string_list );

    global_list = root->children[0];
    size_t n_func = 0;

    for (uint64_t i = 0; i < global_list->n_children; i++) {

        symbol_t *symbol;
        node_t *name_list;
        node_t *global = global_list->children[i];

        if (global->type == FUNCTION) {
            // function
            symbol = malloc(sizeof(symbol_t));
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
                    node_t *parameter =  global->children[1]->n_children;
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
            insert_symbol(global_names, symbol);
        }

        else if(global->type == DECLARATION) {

            name_list = global->children[0];

            for (size_t j = 0; j < name_list->n_children; j++) {
                symbol = malloc(sizeof(symbol_t));
                symbol = malloc(sizeof(symbol_t));
                symbol->nparms = 0;
                symbol->seq = 0;
                symbol->name = name_list->children[j]->data;
                symbol->locals = NULL;
                symbol->type = SYM_GLOBAL_VAR;
                symbol->node = NULL;
            }
            insert_symbol(global_names, symbol);
        }
    }

}


void
bind_names ( symbol_t *function, node_t *root )
{
    if (root != NULL) {

        node_t *name_list;
        symbol_t *symbols;
        tlhash_t **scope_table; = malloc( sizeof(tlhash_t*));
        symbol_t *symbol;
        int n_scope_table = 1, d = 0;

        if (root->type == BLOCK) {
            tlhash_t *scope = malloc(sizeof(tlhash_t));
            tlhash_init(scope, 32);
            scope_table[d] = scope;
            d++;

            if ( d >= n_scope_table ) {
                n_scope_table *= 2;
                scope_table = realloc( scope_table, n_scope_table * sizeof(tlhash_t**) );
            }

            for (size_t i = 0; i < root->n_children; i++) {
                bind_names(function, root->children[i]);
            }

            d--;
            tlhash_finalize(scope_table[d]);
            free(scope_table[d]);

        }

        else if( root->type == DECLARATION ) {

            name_list = root->children[0];

            for (uint64_t i = 0; i < name_list->n_children; i++) {

                node_t *variable_name = name_list->children[i];
                size_t local_n = tlhash_size(function->locals) - function->nparms;

                symbol = malloc(sizeof(symbol_t));
                symbol->nparms = 0;
                symbol->seq = local_n;
                symbol->name = variable_name->data;
                symbol->locals = NULL;
                symbol->type = SYM_LOCAL_VAR;
                symbol->node = NULL;

                tlhash_insert(function->locals, &local_n, sizeof(size_t), symbol);
                tlhash_insert(scope_table[d-1], strlen(symbol->name), symbol);
            }
        }

        else if (root->type == IDENTIFIER_DATA) {
            // lookup in the table
            while ( !symbols && d > 0) {
                d--;
                symbols = tlhash_lookup(scope_table[d], root->data, strlen(root->data), (void **)&symbols);
            }
            // a parameter
            if (symbols == NULL)
                tlhash_lookup(function->locals, root->data, strlen(root->data), (void**)&symbols);
            // a global name
            if (symbols == NULL)
                tlhash_lookup(global_names, root->data, strlen(root->data), (void**)&symbols);
            // exit on failure
            if (symbols == NULL)
                exit(0);

            root->entry = symbols;
        }

        else if (root->type == STRING_DATA) {

            string_list[stringc] = root->data;
            root->data = malloc( sizeof(size_t) );
            

        }

        else {
            for (size_t i = 0; i < root->n_children; i++) {
                bind_names(function, root->children);
            }
        }

    }


}


void
destroy_symtab ( void )
{
    tlhash_finalize ( global_names );
    free ( global_names );
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
