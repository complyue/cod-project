
#include <iostream>

#include "codp.hh"

class SthInShilo {
public:
  static constexpr shilos::UUID TYPE_UUID = shilos::UUID("9B27863B-8997-4158-AC34-38512484EDFB");
};

int main(int argc, char **argv) {
  std::cout << "codp is the cmdl UI to edit Project.codp file with a text editor" << std::endl;

  shilos::memory_region<SthInShilo> *mr;

  return 0;
}
