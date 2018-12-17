#ifndef DATABASE_INCLUDED
#define DATABASE_INCLUDED

#include <iostream>
#include <cstdlib>
#include "Node.hpp"

using namespace std;

template <class T>
class Database {
  public:
    Database(string, string, T);
    ~Database();
    void insert(T);
    T select(T);
    void update(T);
    void remove(T);
    void print(ostream&);

  private:
    fstream dataFile;
    fstream treeFile;
    const char* dataName;
    const char* treeName;
    T* defaultValue;

    void printInOrder(ostream&, int);

    int calculateFactor(int);
    int calculateHeight(int);
    void balance(int);

    void leftRotation(int);
    void rightRotation(int);

    void readNode(Node&, int);
    void writeNode(Node&, int);
    void readData(T&, int);
    void writeData(T&, int);
    void fixIndexes(int, int);

    void removeBytes(fstream&, const char*, int, int);
};

#include "Database.inl"

#endif