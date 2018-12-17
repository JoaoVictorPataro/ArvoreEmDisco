#ifndef NODE_INCLUDED
#define NODE_INCLUDED

#include <iostream>

struct Node {
  int data;
  int left;
  int right;
  int factor;

  Node(int i) {
    this->data = i;
    this->left = -1;
    this->right = -1;
    this->factor = 0;
  }

  Node() {
    this->data = -1;
    this->left = -1;
    this->right = -1;
    this->factor = 0;
  }

  friend std::ostream& operator<<(std::ostream& os, const Node& n) {
    os << "{ data: " << n.data << ", left: " << n.left << ", right: " << n.right << ", factor:" << n.factor << " }";
    return os;
  }
};

#endif