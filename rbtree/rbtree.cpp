#include<iostream>
#include<cstring>
#include<cstdio>
#include<cstdlib>
using namespace std;

#define RED 1
#define BLACK 2

typedef int KEY_TYPE;

typedef struct _rbtree_node
{
    _rbtree_node* left;
    _rbtree_node* right;
    _rbtree_node* parent;
    unsigned char color;
    KEY_TYPE key;
    void* value;
}rbtree_node;

typedef struct _rbtree
{
    rbtree_node* root;
    rbtree_node* nil;
}rbtree;

rbtree_node* find_min(rbtree* T,rbtree_node* x)
{
    while(x->left!=T->nil){
        x=x->left;
    }
    return x;
}

rbtree_node* find_max(rbtree* T,rbtree_node* x)
{
    while(x->right!=T->nil){
        x=x->right;
    }
    return x;
}

rbtree_node *rbtree_successor(rbtree *T, rbtree_node *x) {//寻找x点树形结构的最小点或者最大点
	rbtree_node *y = x->parent;

	if (x->right != T->nil) {
		return find_min(T, x->right);
	}

	while ((y != T->nil) && (x == y->right)) {
		x = y;
		y = y->parent;
	}
	return y;
}


void rbtree_left_rotate(rbtree* T,rbtree_node* x)
{
    rbtree_node* y=x->right;//由于调用左旋函数的前提就是x节点会有字节点，所以这里不需要判断子节点是否为空
    ////先处理x节点和y节点左儿子之间的关系
    x->right=y->left;
    if(y->left!=T->nil){
        y->left->parent=x;
    }
    ////处理y节点的父节点之间的关系
    y->parent=x->parent;
    if(x->parent==T->nil){
        T->root=y;
    }else if(x==x->parent->left){
        x->parent->left=y;
    }else{
        x->parent->right=y;
    }
    ////最后处理x与y节点之间的关系
    y->left=x;
    x->parent=y;
}

void rbtree_right_rotate(rbtree* T,rbtree_node* x)
{
    rbtree_node *y=x->left;
    x->left=y->right;
    if(y->right!=T->nil){
        y->right->parent=x;
    }

    y->parent=x->parent;
    if(x->parent==T->nil){
        T->root=y;
    }else if(x==x->parent->left){
        x->parent->left=y;
    }else{
        x->parent->right=y;
    }

    y->right=x;
    x->parent=y;
}

void rbtree_insert_fixup(rbtree* T,rbtree_node* z)
{
    while(z->parent->color==RED){
        if(z->parent==z->parent->parent->left){
            rbtree_node* y=z->parent->parent;
            if(y->right->color==RED){
                z->parent->color=BLACK;
                y->color=RED;
                y->right->color=BLACK;

                z=y;
            }else{
                if(z==z->parent->right){
                    z=z->parent;
                    rbtree_left_rotate(T,z);
                }
                z->parent->color=BLACK;
                z->parent->parent->color=RED;
                rbtree_right_rotate(T,z->parent->parent);
            }
        }else{
            rbtree_node* y=z->parent->parent;
            if(y->left->color==RED){
                z->parent->color=BLACK;
                y->left->color=BLACK;
                y->color=RED;

                z=y;
            }else{
                if(z==z->parent->left){
                    z=z->parent;
                    rbtree_right_rotate(T,z);
                }
                z->parent->color=BLACK;
                z->parent->parent->color=RED;
                rbtree_left_rotate(T,z->parent->parent);
            }
        }
    }
    T->root->color=BLACK;
}

void rbtree_insert(rbtree* T,rbtree_node* z)
{
    rbtree_node* y=T->nil;
    rbtree_node* x=T->root;
    while(x!=T->nil){
        y=x;
        if(z->key<x->key){
            x=x->left;
        }else if(z->key>x->key){
            x=x->right;
        }else{
            return ;
        }
    }
    z->parent=y;
    if(y==T->nil){
        T->root=z;
    }else if(z->key<y->key){
        y->left=z;
    }else{
        y->right=z;
    }
    z->left=T->nil;
    z->right=T->nil;
    z->color=RED;
    rbtree_insert_fixup(T,z);
}

void rbtree_travel(rbtree* T,rbtree_node* x)
{
    if(x!=T->nil){
        rbtree_travel(T,x->left);
        printf("key:%d  color:%d\n",x->key,x->color);
        rbtree_travel(T,x->right);
    }
}

void rbtree_delete_fixup(rbtree *T, rbtree_node *x) {//红黑树删除后调整

	while ((x != T->root) && (x->color == BLACK)) {
		if (x == x->parent->left) {

			rbtree_node *w= x->parent->right;
			if (w->color == RED) {
				w->color = BLACK;
				x->parent->color = RED;

				rbtree_left_rotate(T, x->parent);
				w = x->parent->right;
			}

			if ((w->left->color == BLACK) && (w->right->color == BLACK)) {
				w->color = RED;
				x = x->parent;
			} else {

				if (w->right->color == BLACK) {
					w->left->color = BLACK;
					w->color = RED;
					rbtree_right_rotate(T, w);
					w = x->parent->right;
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->right->color = BLACK;
				rbtree_left_rotate(T, x->parent);

				x = T->root;
			}

		} else {

			rbtree_node *w = x->parent->left;
			if (w->color == RED) {
				w->color = BLACK;
				x->parent->color = RED;
				rbtree_right_rotate(T, x->parent);
				w = x->parent->left;
			}

			if ((w->left->color == BLACK) && (w->right->color == BLACK)) {
				w->color = RED;
				x = x->parent;
			} else {

				if (w->left->color == BLACK) {
					w->right->color = BLACK;
					w->color = RED;
					rbtree_left_rotate(T, w);
					w = x->parent->left;
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->left->color = BLACK;
				rbtree_right_rotate(T, x->parent);

				x = T->root;
			}

		}
	}

	x->color = BLACK;
}

rbtree_node *rbtree_delete(rbtree *T, rbtree_node *z) {//红黑树删除

	rbtree_node *y = T->nil;
	rbtree_node *x = T->nil;

	if ((z->left == T->nil) || (z->right == T->nil)) {
		y = z;
	} else {
		y = rbtree_successor(T, z);
	}

	if (y->left != T->nil) {
		x = y->left;
	} else if (y->right != T->nil) {
		x = y->right;
	}

	x->parent = y->parent;
	if (y->parent == T->nil) {
		T->root = x;
	} else if (y == y->parent->left) {
		y->parent->left = x;
	} else {
		y->parent->right = x;
	}

	if (y != z) {
		z->key = y->key;
		z->value = y->value;
	}

	if (y->color == BLACK) {
		rbtree_delete_fixup(T, x);
	}

	return y;
}

rbtree_node *rbtree_search(rbtree *T, KEY_TYPE key) {//红黑树查找

	rbtree_node *node = T->root;
	while (node != T->nil) {
		if (key < node->key) {
			node = node->left;
		} else if (key > node->key) {
			node = node->right;
		} else {
			return node;
		}	
	}
	return T->nil;
}
int main()
{
    
    int keyArray[20] = {24,25,13,35,23, 26,67,47,38,98, 20,19,17,49,12, 21,9,18,14,15};
    rbtree* T=(rbtree*)malloc(sizeof(rbtree));
    if(T==NULL){
        printf("T初始化失败\n");
        return -1;
    }
    T->nil=(rbtree_node*)malloc(sizeof(rbtree_node));
    T->nil->color=BLACK;
    T->root=T->nil;

    rbtree_node *node=T->nil;
    for(int i=0;i<20;i++){
        node=(rbtree_node*)malloc(sizeof(rbtree_node));
        node->key=keyArray[i];
        node->value=NULL;
        printf("----------------------------------------\n");
        rbtree_insert(T,node);
    }
    rbtree_travel(T, T->root);
    printf("----------------------------------------\n");

	for (int i = 0;i < 20;i ++) {

		rbtree_node *node = rbtree_search(T, keyArray[i]);
		rbtree_node *cur = rbtree_delete(T, node);
		free(cur);

		rbtree_travel(T, T->root);
		printf("----------------------------------------\n");
	}
    return 0;
}