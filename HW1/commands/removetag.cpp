#include <fstream>
#include <iomanip>
#include <iostream>
using namespace std;

int main(int argc, char* const argv[]) {
  ifstream file;
  if (argc == 2) {
    file.open(argv[1]);
    cin.rdbuf(file.rdbuf());
  } else if (argc > 2) {
    cerr << "usage: " << argv[0] << " [filename]" << endl;
  }

  char c;
  bool in_tag = false;
  while (cin.get(c)) {
    switch (c) {
      case '<':
        in_tag = true;
        break;
      case '>':
        in_tag = false;
        break;
      default:
        if (!in_tag) cout.put(c);
    }
  }
  return 0;
}