#include <iostream>
#include <fstream>
#include <string.h>
#include "Database.hpp"

using namespace std;

struct Aluno {
  int ra;
  int idade;

  Aluno(int ra, int age) {
    this->ra = ra;
    this->idade = age;
  }

  Aluno(int ra) {
    this->ra = ra;
    this->idade = 0;
  }

  Aluno() {
    this->ra = 0;
    this->idade = 0;
  }

  friend ostream& operator<<(ostream& os, const Aluno& s) {
    os << "{ ra: " << s.ra << ", idade: " << s.idade << " }";
    return os;
  }

  bool operator<(const Aluno& s2) {
    return this->ra < s2.ra;
  }

  bool operator>(const Aluno& s2) {
    return this->ra > s2.ra;
  }
};

int main() {
  Database<Aluno> bd ("data.dat", "tree.dat", Aluno(-1));

  //bd.insert(Aluno(5, 19));
  //bd.insert(Aluno(3, 18));
  //bd.insert(Aluno(7, 18));
  //bd.insert(Aluno(1, 16));
  //bd.insert(Aluno(4, 16));
  //bd.insert(Aluno(6, 16));
  //bd.insert(Aluno(8, 16));


  cout << bd.select(Aluno(5)) << endl;
  cout << bd.select(Aluno(3)) << endl;
  cout << bd.select(Aluno(7)) << endl;
  cout << bd.select(Aluno(1)) << endl;
  cout << bd.select(Aluno(4)) << endl;
  cout << bd.select(Aluno(6)) << endl;
  cout << bd.select(Aluno(8)) << endl;

  //bd.update(Aluno(15174, 19));
  //cout << bd.select(Aluno(15174)) << endl << endl;
  bd.remove(Aluno(1));
  bd.remove(Aluno(4));
  bd.remove(Aluno(3));

  cout << endl;
  bd.print(cout);

  //bd.~Database();
  //remove("data.dat");
  //remove("tree.dat");
  return 0;
}
