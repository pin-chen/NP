#include <fstream>
#include <iomanip>
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

  string line;
  unsigned line_count = 0;

  while (getline(cin, line))
    cout << setw(4) << setfill(' ') << ++line_count << ' ' << line << endl;
  return 0;
}
