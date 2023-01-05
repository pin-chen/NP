#include <fstream>
#include <iostream>
using namespace std;

int main(int argc, char* const argv[]) {
  ifstream fin;
  if (argc == 2) {
    fin.open(argv[1]);
    if (fin)
      cin.rdbuf(fin.rdbuf());
    else {
      cerr << "Unable to open file \"" << argv[1] << "\"" << endl;
      exit(EXIT_FAILURE);
    }
  } else if (argc > 2) {
    cerr << "Usage: " << argv[0] << ' ' << "[file]" << endl;
    exit(EXIT_FAILURE);
  }

  bool in_tag = false;
  char c;
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
        break;
    }
  }
  return 0;
}
