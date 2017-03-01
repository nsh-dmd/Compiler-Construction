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

void
simplify_tree ( node_t **simplified, node_t *root )
{
    if (!root) return;

   // Recursion loop, move pointer to tree bottom
    for (uint64_t i = 0; i < root->n_children; i++) {
        simplify_tree( &root->children[i], root->children[i] );
    }

    // Prone singular nodes with one child and no useful data
    if (is_syntactical(root)) {
        root = prone_and_connect(root);
    }
    //simplify lists
    else if( list_detected(root) && root->n_children > 1 )
        root = flatten_list(root);

    else if( root->type == PRINT_STATEMENT ) {
        prone_printlists(root->children[0], root);
        prone_printitems(root);
    }

    else if( root->type == EXPRESSION )
        root = resolve_constant_expressions(root);

    *simplified = root;
}

bool list_detected ( node_t *node) {

  switch (node->type) {
    case GLOBAL_LIST:
    case STATEMENT_LIST:
    case PRINT_LIST:
    case EXPRESSION_LIST:
    case VARIABLE_LIST:
    case DECLARATION_LIST:
    //   printf("nodetype%s\n", node_string[node->type] );
      return true;
      break;
    default:
    //   printf("no list: %s\n", node_string[node->type] );
      return false;
  }
}

// Returns true if the node has only 1 child and no meaningful data
bool is_syntactical ( node_t *node ) {
  // return (node->n_children == 1 || node->type == node->children[0]->type) && !node->data ;
  switch (node->type) {
      case GLOBAL:
      case STATEMENT:
      case PRINT_ITEM:
      case PARAMETER_LIST:
      case ARGUMENT_LIST:
        return true;
  }
  return false;
}

node_t* flatten_list ( node_t *node ) {
  //delete internal nodes of list structures
  node_t *child = node->children[0];
  node_t **extended = (node_t **) realloc ( child->children, (child->n_children+1)*sizeof(node_t*) );
  extended[child->n_children] = node->children[1];
  child->n_children = child->n_children + 1;
  child->children = extended;
  node_finalize(node);
  return child;
}

node_t* prone_and_connect ( node_t *node) {
    node_t *child = node->children[0];
    node_finalize(node);
    node = child;
    node_finalize(child);
    return node;
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
//
// void prone_global(node_t *node) {
//     for ( uint64_t i=0; i<node->children[0]->n_children; i++ ) {
//         node_t *child = node->children[0]->children[i];
//         node->children[0]->children[i] = child->children[0];
//         node_finalize ( child );
//     }
// }

// bool is_simplifiable( node_t *node ) {
//     return  node->n_children > 0 && node->children[0]->type == node->type;
// }

node_t* resolve_constant_expressions(node_t *node) {

    int64_t *result;

    if (node->n_children == 2 &&
        node->children[0]->type == NUMBER_DATA &&
        node->children[1]->type == NUMBER_DATA) {

        switch (*(char*)node->data) {
            case '*':
                *result = *(int64_t *) node->children[0]->data * *(int64_t *)node->children[1]->data;
                 break;
            case '/':
                *result = *(int64_t *)node->children[0]->data / *(int64_t *)node->children[1]->data;
                 break;
            case '+':
                *result = *(int64_t *)node->children[0]->data + *(int64_t *)node->children[1]->data;
                 break;
            case '-':
                *result = *(int64_t *)node->children[0]->data - *(int64_t *)node->children[1]->data;
                 break;
        }
        node->children[0]->data = result;
        node->children[0]->type = NUMBER_DATA;
        node_finalize ( node->children[1] );
        return node->children[0];
    }

    else if (node->n_children == 1 &&
        node->children[0]->type == NUMBER_DATA && node->data != NULL ) {
            *result = *((int64_t *)node->children[0]->data) * -1;
            node->children[0]->data = result;
            return node->children[0];
        }
}
