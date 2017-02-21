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
      printf("nodetype%s\n", node_string[root->type] );
      return true;
      break;
    default:
      printf("no list: %s\n", node_string[root->type] );
      return false;
  }
}

node_t prone_and_connect_child ( node_t *node, node_t *parent ) {

  nodet_t *child = node->children[0];
  node_finalize(node);
  return child;
}

// Returns true if the node has only 1 child and no meaningful data
bool is_syntactic ( node_t *node ) {
  return node->n_children == 1 && !node->data ;
}

void flatten_list () {
  return;
}
void traverse ( node_t *root ) {

  node_t *parent = root;

}

void
simplify_tree ( node_t **simplified, node_t *root )
{
    for (size_t i = 0; i < root->n_children; i++) {
      simplify_tree( &root->children[i], root->children[i] );
    }

    if ( is_syntactic(root) ) {
        root = prone_and_switch_parents( root, parent )
    }


    *simplified = root;
}
