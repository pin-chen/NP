#include <unistd.h>
#include <fstream>
#include <iostream>
using namespace std;

const size_t DELAYED_SECONDS = 1;

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
        if (!in_tag) {
          cout.put(c);
          if (c == '\n') {
            cout.flush();
            sleep(DELAYED_SECONDS);
          }
        }
        break;
    }
  }
  return 0;
}
