#include <vslc.h>


void
node_print ( node_t *root, int nesting )
{
    if ( root != NULL )
    {
        printf ( "%*c%s", nesting, ' ', node_string[root->type] );
        if ( root->type == IDENTIFIER_DATA ||
             root->type == STRING_DATA ||
             root->type == RELATION ||
             root->type == EXPRESSION )
            printf ( "(%s)", (char *) root->data );
        else if ( root->type == NUMBER_DATA )
            printf ( "(%ld)", *((int64_t *)root->data) );
        putchar ( '\n' );
        for ( int64_t i=0; i<root->n_children; i++ )
            node_print ( root->children[i], nesting+1 );
    }
    else
        printf ( "%*c%p\n", nesting, ' ', root );
}


void
node_init (node_t *nd, node_index_t type, void *data, uint64_t n_children, ...)
{
    va_list child_list;
    *nd = (node_t) {
        .type = type,
        .data = data,
        .entry = NULL,
        .n_children = n_children,
        .children = (node_t **) malloc ( n_children * sizeof(node_t *) )
    };
    va_start ( child_list, n_children );
    for ( uint64_t i=0; i<n_children; i++ )
        nd->children[i] = va_arg ( child_list, node_t * );
    va_end ( child_list );
}


void
node_finalize ( node_t *discard )
{
    if ( discard != NULL )
    {
        free ( discard->data );
        free ( discard->children );
        free ( discard );
    }
}

void
destroy_subtree ( node_t *discard )
{
    if ( discard != NULL )
    {
        for ( uint64_t i=0; i<discard->n_children; i++ )
            destroy_subtree ( discard->children[i] );
        node_finalize ( discard );
    }
}
bool list_detected ( node_t *node) {

  switch (node->type) {
    case GLOBAL_LIST:
    case STATEMENT_LIST:
    case PRINT_LIST:
    case EXPRESSION_LIST:
    case VARIABLE_LIST:
    case ARGUMENT_LIST:
    case PARAMETER_LIST:
    case DECLARATION_LIST:
      printf("nodetype%s\n", node_string[node->type] );
      return true;
      break;
    default:
      printf("no list: %s\n", node_string[node->type] );
      return false;
  }
}

// Returns true if the node has only 1 child and no meaningful data
bool is_syntactic ( node_t *node ) {
  return (node->n_children == 1 || node->type == node->children[0]->type) && !node->data ;
}

void flatten_list ( node_t *node ) {
  //delete internal nodes of list structures
  // leave parent = list type, and children = list items
  node_t *child = node->children[0];
  node_t **extended = (node_t **) realloc ( child->children, (child->n_children+1)*sizeof(node_t*) );
  extended[child->n_children] = node->children[1];
  node->n_children = child->n_children + 1;
  node->children = extended;
  node_finalize(node);
}

// void prone_global (node_t *node) {
//   return;void prone_and_connect ( node_t *node ) {
// }

// void remove_print_list_items ( node_t *node ) {
//   for (uint64_t i = 0; i < node->n_children; i++) {
//
//   }
// }
void prone_and_connect ( node_t *node) {
    node_t *child = node->children[0];
    node_finalize(node);
    node = child;
    node_finalize(child);
}

void prone_printlists( node_t *node, node_t *parent ) {
    parent->n_children = node->n_children;
    free(parent->children);
    parent->children = node->children;
    node_finalize(node);
}

void prone_printitems( node_t *node ) {
    for (size_t i = 0; i < node->n_children; i++) {
        node_t *child = node->children[i];
        node->children[i] = child->children[0];
        node_finalize(child);
    }
}

void prone_global(node_t *node) {
    for ( uint64_t i=0; i<node->children[0]->n_children; i++ ) {
        node_t *child = node->children[0]->children[i];
        node->children[0]->children[i] = child->children[0];
        node_finalize ( child );
    }
}

bool is_simplifiable( node_t *node ) {
    return  node->n_children > 0 && node->children[0]->type == node->type;
}
void
resolve_constant_expressions(node_t *node) {
    size_t num_data = 1;
    for ( uint64_t i=0; i<root->n_children; i++ ) {
        if (root->children[i]->type != NUMBER_DATA)
            num_data = 0;
    }
    if (num_data) {
        int64_t *result = malloc( sizeof(int64_t) );
        switch (*(long*)node->data) {
            case NULL: *result = *(long*)node->children[0]->data; break;
            case "*":
                *result = *(long*)(node->children[0]->data) * *(long*)(node->children[1]->data);
                break;
            case "/":
                *result = *(long*)(node->children[0]->data) / *(long*)(node->children[1]->data);
                break;
            case "+":
                *result = *(long*)(node->children[0]->data) + *(long*)(node->children[1]->data);
                break;
            case "-":
                if (node->n_children == 1) {
                    *result = - *(long*)(node->children[0]->data);
                    break;
                }
                else {
                    *result = *(long*)(node->children[0]->data) - *(long*)(node->children[1]->data);
                    break;
                }
            }

            for ( size_t i=0; i < node->n_children; i++ ) {
                destroy_subtree( node->children[i] );
            }

            free ( node->children );
            free ( node->data );
            node->n_children = 0;
            node->data = result;
            node->type = NUMBER_DATA;
        }
}


void
simplify_tree ( node_t **simplified, node_t *root )
{
  // Recursion loop, move pointer to tree bottom
    for (uint64_t i = 0; i < root->n_children; i++) {
        if (root->children[i] != NULL) {
            simplify_tree( &root->children[i], root->children[i] );
        }
    }
    // Prone singular nodes with one child and no useful data
    // if ( is_syntactic(root) ) {
    //   prone_and_connect ( root ); //sjekk om den setter rozot like barnenoden !!!!
    // }

    //simplify lists
    if( list_detected(root) && is_simplifiable(root) )
        flatten_list(root);

    switch ( root->type ) {
        case PRINT_STATEMENT:
            prone_printlists(root->children[0], root);
            prone_printitems(root);
            break;
        case PROGRAM:
            prone_global(root);
            break;
        case PARAMETER_LIST:
        case ARGUMENT_LIST:
        case STATEMENT:
            prone_and_connect(root);
            break;
        case EXPRESSION:
            resolve_constant_expressions(root);

    }

    *simplified = root;
}
