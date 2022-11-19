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

  string line;
  int lineno = 0;
  while (getline(cin, line)) {
    cout << setw(4) << setfill(' ') << ++lineno << ' ' << line << endl;
  }
  return 0;
}