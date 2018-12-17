#include <stdexcept>
#include <stack>

template <class T>
Database<T>::Database(string dataFile, string treeFile, T defaultValue) {
  const char* dataFileChr = dataFile.c_str();
  this->dataName = strdup(dataFileChr);
  this->dataFile.open(dataFileChr, ios::out | ios::in | ios::binary);
  if (!this->dataFile.is_open()) {
    this->dataFile.open(dataFileChr, ios::out);
    this->dataFile.close();

    this->dataFile.open(dataFileChr, ios::out | ios::in | ios::binary);
  } 

  const char* treeFileChr = treeFile.c_str();
  this->treeName = strdup(treeFileChr);
  this->treeFile.open(treeFileChr, ios::out | ios::in | ios::binary);
  if (!this->treeFile.is_open()) {
    this->treeFile.open(treeFileChr, ios::out);
    this->treeFile.close();

    this->treeFile.open(treeFileChr, ios::out | ios::in | ios::binary);
  } 

  this->defaultValue = new T(defaultValue);
}

template <class T>
Database<T>::~Database() {
  this->dataFile.close();
  this->treeFile.close();
  delete this->dataName;
  delete this->treeName;
}

template <class T>
void Database<T>::insert(T data) {
  this->dataFile.seekg(0, this->dataFile.end);
  int amount = this->dataFile.tellg() / sizeof(T);

  this->writeData(data, amount);
  Node n(amount);

  int next = 0;
  stack<int> nodeStack;

  this->treeFile.seekg(0, this->treeFile.end);
  int treeAmount = this->treeFile.tellg() / sizeof(Node);
  if (treeAmount > 0) {
    T tmp;
    Node current;

    while (next != -1) {
      nodeStack.push(next);

      this->readNode(current, next);
      this->readData(tmp, current.data);
      
      if (data > tmp)
        if (current.right == -1) {
          current.right = amount;
          break;
        } else
          next = current.right;
      else if (data < tmp)
        if (current.left == -1) {
          current.left = amount;
          break;
        } else
          next = current.left;
      else
        throw invalid_argument("Data already exists");
    }

    this->writeNode(current, next);
  }

  this->writeNode(n, treeAmount);
  while (!nodeStack.empty()) {
    int node = nodeStack.top();
    nodeStack.pop();

    if (abs(this->calculateFactor(node)) > 1)
      this->balance(node);
  }
}

template <class T>
T Database<T>::select(T data) {
  this->treeFile.seekg(0, this->treeFile.end);

  if (this->treeFile.tellg() > 0) {
    T tmp;
    Node current;

    int next = 0;
    while (next != -1) {
      this->readNode(current, next);
      this->readData(tmp, current.data);
      
      if (data > tmp)
        next = current.right;
      else if (data < tmp)
        next = current.left;
      else
        return tmp;
    }
  }

  return T(*this->defaultValue);
}

template <class T>
void Database<T>::update(T data) {
  this->treeFile.seekg(0, this->treeFile.end);

  int next = 0;
  if (this->treeFile.tellg() > 0) {
    T tmp;
    Node current;
    
    while (next != -1) {
      this->readNode(current, next);
      this->readData(tmp, current.data);
      
      if (data > tmp)
          next = current.right;
      else if (data < tmp)
          next = current.left;
      else {
        this->writeData(data, current.data);
        break;
      }
    }
  }
}

template <class T>
void Database<T>::remove(T data) {
  stack<int> nodeStack;

  this->treeFile.seekg(0, this->treeFile.end);
  int next = 0, prev = 0;
  if (this->treeFile.tellg() > 0) {
    T tmp;
    Node current;

    while (next != -1) {
      nodeStack.push(next);

      this->readNode(current, next);
      this->readData(tmp, current.data);
      
      if (data > tmp) {
        prev = next;
        next = current.right;
      } else if (data < tmp) {
        prev = next;
        next = current.left;
      } else {
        if (current.left == -1 && current.right == -1) {
          Node prevNode;
          this->readNode(prevNode, prev);
          this->readData(tmp, prevNode.data);

          if (data > tmp)
            prevNode.right = -1;
          else
            prevNode.left = -1;

          this->writeNode(prevNode, prev);
          this->removeBytes(this->treeFile, this->treeName, next * sizeof(Node), sizeof(Node));
          this->removeBytes(this->dataFile, this->dataName, current.data * sizeof(T), sizeof(T));

          this->fixIndexes(next, current.data);
          break;
        } else if (current.right == -1) {
          int prevBig = next;
          int nextBig = current.left;

          Node biggest;
          this->readNode(biggest, nextBig);
          while (biggest.right != -1) {
            prevBig = nextBig;
            nextBig = biggest.right;

            this->readNode(biggest, nextBig);
          }

          int removedData = current.data;
          current.data = biggest.data;
          if (prevBig == next)
            current.left = -1;
          else {
            Node biggestPrev;
            this->readNode(biggestPrev, prevBig);

            biggestPrev.right = -1;
            this->writeNode(biggestPrev, prevBig);
          }

          this->writeNode(current, next);

          this->removeBytes(this->treeFile, this->treeName, nextBig * sizeof(Node), sizeof(Node));
          this->removeBytes(this->dataFile, this->dataName, removedData * sizeof(T), sizeof(T));

          this->fixIndexes(nextBig, removedData);
        } else {
          int prevSmall = next;
          int nextSmall = current.right;

          Node smallest;
          this->readNode(smallest, nextSmall);
          while (smallest.left != -1) {
            prevSmall = nextSmall;
            nextSmall = smallest.left;

            this->readNode(smallest, nextSmall);
          }

          int removedData = current.data;
          current.data = smallest.data;
          if (prevSmall == next)
            current.right = -1;
          else {
            Node smallestPrev;
            this->readNode(smallestPrev, prevSmall);

            smallestPrev.left = -1;
            this->writeNode(smallestPrev, prevSmall);
          }

          this->writeNode(current, next);
          this->removeBytes(this->treeFile, this->treeName, nextSmall * sizeof(Node), sizeof(Node));
          this->removeBytes(this->dataFile, this->dataName, removedData * sizeof(T), sizeof(T));

          this->fixIndexes(nextSmall, removedData);
        }
      }
    }

    while (!nodeStack.empty()) {
      int node = nodeStack.top();
      nodeStack.pop();

      if (abs(this->calculateFactor(node)) > 1)
        this->balance(node);
    }
  }
}

template <class T>
int Database<T>::calculateFactor(int nodeIndex) {
  Node current;
  this->readNode(current, nodeIndex);

  int lHeight = 0;
  if (current.left != -1)
    lHeight = calculateHeight(current.left);

  int rHeight = 0;
  if (current.right != -1)
    rHeight = calculateHeight(current.right);

  current.factor = rHeight -lHeight;
  this->writeNode(current, nodeIndex);

  return current.factor;
}

template <class T>
int Database<T>::calculateHeight(int nodeIndex) {
  Node current;
  this->readNode(current, nodeIndex);

  int lHeight = 0;
  if (current.left != -1)
    lHeight = calculateHeight(current.left);

  int rHeight = 0;
  if (current.right != -1)
    rHeight = calculateHeight(current.right);

  return max(lHeight, rHeight) + 1;
}

template <class T>
void Database<T>::balance(int nodeIndex) {
  Node root;
  this->readNode(root, nodeIndex);

  if (root.factor > 1) {
    Node right;
    this->readNode(right, root.right);

    if (right.right == -1)
      this->rightRotation(root.right);
    
    this->leftRotation(nodeIndex);
  } else if (root.factor < -1) {
    Node left;
    this->readNode(left, root.left);

    if (left.left == -1)
      this->leftRotation(root.left);
      
    this->rightRotation(nodeIndex);
  }

  this->calculateFactor(nodeIndex);
}

template <class T>
void Database<T>::leftRotation(int nodeIndex) {
  Node current;
  this->readNode(current, nodeIndex);
  int rightIndex = current.right;

  Node right;
  this->readNode(right, current.right);

  current.right = right.left;
  right.left = rightIndex;

  this->writeNode(current, rightIndex);
  this->writeNode(right, nodeIndex);

  this->calculateFactor(rightIndex);
}

template <class T>
void Database<T>::rightRotation(int nodeIndex) {
  Node current;
  this->readNode(current, nodeIndex);
  int leftIndex = current.left;

  Node left;
  this->readNode(left, current.left);

  current.left = left.right;
  left.right = leftIndex;

  this->writeNode(current, leftIndex);
  this->writeNode(left, nodeIndex);

  this->calculateFactor(leftIndex);
}

template <class T>
void Database<T>::readNode(Node& node, int nodeIndex) {
  char* nodeBytes = new char[sizeof(Node)];

  this->treeFile.seekg(nodeIndex * sizeof(Node), this->treeFile.beg);
  this->treeFile.read(nodeBytes, sizeof(Node));
  node = *reinterpret_cast<Node*>(nodeBytes);

  delete nodeBytes;
}

template <class T>
void Database<T>::writeNode(Node& node, int nodeIndex) {
  char* bytes = new char[sizeof(Node)];

  bytes = reinterpret_cast<char*>(&node);
  this->treeFile.seekg(nodeIndex * sizeof(Node), this->treeFile.beg);
  this->treeFile.write(bytes, sizeof(Node));

  delete bytes;
}

template <class T>
void Database<T>::readData(T& data, int dataIndex) {
  char* dataBytes = new char[sizeof(T)];

  this->dataFile.seekg(dataIndex * sizeof(T), this->dataFile.beg);
  this->dataFile.read(dataBytes, sizeof(T));
  data = *reinterpret_cast<T*>(dataBytes);

  delete dataBytes;
}

template <class T>
void Database<T>::writeData(T& data, int dataIndex) {
  char* bytes = new char[sizeof(T)];

  bytes = reinterpret_cast<char*>(&data);
  this->dataFile.seekg(dataIndex * sizeof(T), this->dataFile.beg);
  this->dataFile.write(bytes, sizeof(T));

  delete bytes;
}

template <class T>
void Database<T>::fixIndexes(int removedNode, int removedData) {
  Node current;

  this->treeFile.seekg(0, this->treeFile.end);
  int last = this->treeFile.tellg() / sizeof(Node);
  for (int i = 0; i < last; i++) {
    this->readNode(current, i);

    if (current.right > removedNode)
      current.right--;
    
    if (current.left > removedNode)
      current.left--;

    if (current.data > removedData)
      current.data--;

    this->writeNode(current, i);
  }
}

template <class T>
void Database<T>::removeBytes(fstream& file, const char* filename, int firstBytes, int skip) {
  file.seekg(0, file.end);
  int fileSize = file.tellg();
  int lastBytes = fileSize - firstBytes - skip;
  //cout << fileSize << " " << firstBytes << " " << lastBytes << endl;

  char* first = new char[firstBytes];
  char* last = new char[lastBytes];

  file.seekg(0, file.beg);
  file.read(first, firstBytes);
  file.seekg(-lastBytes, file.end);
  file.read(last, lastBytes);
  file.close();

  file.open(filename, ios::out | ios::binary | ios::trunc);
  file.write(first, firstBytes);
  file.write(last, lastBytes);
  file.close();

  file.open(filename, ios::out | ios::in | ios::binary);

  delete first;
  delete last;
}

template <class T>
void Database<T>::printInOrder(ostream& os, int nodeIndex) {
  Node current;
  this->readNode(current, nodeIndex);

  os << "(";
  if (current.left != -1)
    this->printInOrder(os, current.left);
  
  T data;
  this->readData(data, current.data);
  os << data;

  if (current.right != -1)
    this->printInOrder(os, current.right);
  os << ")";
}

template <class T>
void Database<T>::print(ostream& os) {
  T tmp;
  os << "dataFile:" << endl;
  this->dataFile.seekg(0, this->dataFile.end);
  int dataLength = this->dataFile.tellg() / sizeof(T);
  for (int i = 0; i < dataLength; i++) {
    this->readData(tmp, i);
    os << "  " << tmp << endl;
  }

  Node tmpNode;
  os << endl;
  os << "treeFile:" << endl;
  this->treeFile.seekg(0, this->dataFile.end);
  dataLength = this->treeFile.tellg() / sizeof(Node);
  for (int i = 0; i < dataLength; i++) {
    this->readNode(tmpNode, i);
    os << "  " << tmpNode << endl;
  }

  if (dataLength > 0) {
    os << endl;
    this->printInOrder(os, 0);
    os << endl;
  }
}