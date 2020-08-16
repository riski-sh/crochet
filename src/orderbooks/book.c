#include "book.h"

void
book_print(struct generic_book *b)
{
  if (!b) {
    return;
  }

  if (b->left) {
    printf("%lu -> %lu\n", b->price, b->left->price);
  }

  if (b->right) {
    printf("%lu -> %lu\n", b->price, b->right->price);
  }

  book_print(b->left);
  book_print(b->right);
}

static int
_max_depth(struct generic_book *node)
{
  if (node == NULL) {
    return 0;
  } else {
    int lDepth = _max_depth(node->left);
    int rDepth = _max_depth(node->right);

    if (lDepth > rDepth) {
      return lDepth + 1;
    } else {
      return rDepth + 1;
    }
  }
}

/*
 * Performs an ll rotation and returns the new root
 */
static void
_ll_rotation(struct generic_book **node)
{
  struct generic_book *a = *node;
  struct generic_book *b = (*node)->left;
  struct generic_book *x = b->right;

  b->right = a;
  b->parent = a->parent;

  if (a->parent) {
    if (a->parent->left == a) {
      a->parent->left = b;
    } else {
      a->parent->right = b;
    }
  }

  a->parent = b;

  if (x) {
    a->left = x;
    x->parent = a;
  } else {
    a->left = NULL;
  }

  *node = b;
}

/*
 * Performs a right rotation and returns the new root
 */
static void
_rr_rotation(struct generic_book **node)
{
  struct generic_book *a = *node;
  struct generic_book *b = (*node)->right;
  struct generic_book *x = b->left;

  b->left = a;
  b->parent = a->parent;

  if (a->parent) {
    if (a->parent->left == a) {
      a->parent->left = b;
    } else {
      a->parent->right = b;
    }
  }

  a->parent = b;

  if (x) {
    a->right = x;
    x->parent = a;
  } else {
    a->right = NULL;
  }
  *node = b;
}

/*
 * Performs an lr rotation
 */
static void
_lr_rotation(struct generic_book **node)
{
  struct generic_book *b = (*node)->left;
  struct generic_book *c = b->right;

  struct generic_book *x = c->left;

  c->parent = b->parent;
  b->parent = c;
  c->left = b;
  (*node)->left = c;

  if (x) {
    b->right = x;
    x->parent = b;
  } else {
    b->right = NULL;
  }
}

/*
 * Performs an rl rotation
 */
static void
_rl_rotation(struct generic_book **node)
{
  struct generic_book *b = (*node)->right;
  struct generic_book *c = b->left;

  struct generic_book *x = c->right;

  c->parent = b->parent;
  b->parent = c;
  c->right = b;
  (*node)->right = c;

  if (x) {
    b->left = x;
    x->parent = b;
  } else {
    b->left = NULL;
  }
}

static void
_balance(struct generic_book **node)
{
  while (*node) {
    // compute the balance score
    int lDepth = _max_depth((*node)->left);
    int rDepth = _max_depth((*node)->right);

    int balance_score = (lDepth - rDepth);
    if (balance_score == -1 || balance_score == 0 || balance_score == 1) {
      if ((*node)->parent) {
        (*node) = (*node)->parent;
        continue;
      } else {
        break;
      }
    }

    if (balance_score == 2) {
      if (_max_depth((*node)->left->left) > _max_depth((*node)->left->right)) {
        _ll_rotation(node);
        continue;
      } else {
        _lr_rotation(node);
        _ll_rotation(node);
        continue;
      }
    } else if (balance_score == -2) {
      if (_max_depth((*node)->right->right) >
          _max_depth((*node)->right->left)) {
        _rr_rotation(node);
        continue;
      } else {
        _rl_rotation(node);
        _rr_rotation(node);
        continue;
      }
    } else {
      book_print(*node);
      abort();
    }
  }
}

static struct generic_book *
_book_query(struct generic_book **_root, uint64_t price)
{

  struct generic_book *root = *_root;

  // Find where to insert or where to return if the value already exists
  while (root) {
    if (root->price == price) {
      // found the price no need to insert
      return root;
    } else if (price > root->price) {
      if (root->right) {
        // tree still exists go down the tree
        root = root->right;
        continue;
      } else {
        // insert to the right of this node
        struct generic_book *new = malloc(sizeof(struct generic_book));
        new->left = new->right = new->data = NULL;
        new->parent = root;
        new->total = 0;
        new->price = price;
        root->right = new;

        struct generic_book *ret = new;
        _balance(&new);

        *_root = new;

        return ret;
      }
    } else if (price < root->price) {
      if (root->left) {
        // tree still exists go down the tree
        root = root->left;
        continue;
      } else {
        // insert to the left of this node
        struct generic_book *new = malloc(sizeof(struct generic_book));
        new->left = new->right = new->data = NULL;
        new->parent = root;
        new->total = 0;
        new->price = price;
        root->left = new;

        struct generic_book *ret = new;
        _balance(&new);
        *_root = new;
        return ret;
      }
    }
  }

  // should never get here
  return NULL;
}

struct generic_book *
book_query(struct generic_book **root, uint64_t price)
{
  if (*root) {
    // a tree is already defined insert/query
    return _book_query(root, price);
  } else {
    // an empty tree, create then return the node
    *root = malloc(sizeof(struct generic_book));
    (*root)->left = (*root)->right = (*root)->parent = (*root)->data = NULL;
    (*root)->total = 0;
    (*root)->price = price;
    return *root;
  }
}

void
book_remove(
    struct generic_book **root, uint64_t price, book_free_data free_func)
{
  // Find the price
  struct generic_book *level = book_query(root, price);

  if (level->price != price) {
    pprint_error("requested price level %lu but got %lu", __FILE_NAME__,
        __func__, __LINE__, price, level->price);
    abort();
  }

  if (level->left == NULL && level->right == NULL) {
    // leaf node just delete the leaf
    struct generic_book *parent = level->parent;
    free_func(level->data);
    free(level);
    *root = parent;
    _balance(root);
  } else if (level->left == NULL && level->right != NULL) {
    // one child to the right
    // take the right child and make it the parent
    struct generic_book *n = level->right;
    struct generic_book *l = level;

    free_func(l->data);
    l->right = n->right;
    l->left = n->left;
    l->data = n->data;
    l->price = n->price;
    l->total = n->total;
    free(n);
    *root = l;
    _balance(root);
  } else if (level->right == NULL && level->left != NULL) {
    // one child to the left
    struct generic_book *n = level->left;
    struct generic_book *l = level;

    free_func(l->data);
    l->right = n->right;
    l->left = n->left;
    l->data = n->data;
    l->price = n->price;
    l->total = n->total;
    free(n);
    *root = l;
    _balance(root);
  } else if (level->right != NULL && level->left != NULL) {
    // two children
    pprint_error("can't do this yet", __FILE_NAME__, __func__, __LINE__);
    abort();
  } else {
    pprint_error("this case is impossible unless level == NULL which should"
                 "never happen",
        __FILE_NAME__, __func__, __LINE__);
    abort();
  }
}

void
book_free(struct generic_book *node, book_free_data free_func)
{
  if (!node) {
    return;
  }
  book_free(node->left, free_func);
  book_free(node->right, free_func);

  free_func(node->data);
  free(node);
}
