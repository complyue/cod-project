
#include <iostream>

#include "codp.hh"

using namespace shilos;
using namespace cod::project;

int main(int argc, char **argv) {
  std::cout << "codp is the cmdl UI to edit Project.codp file with a text editor" << std::endl;

  DBMR<CodProject> prj("cod.project", 0);
  memory_region<CodProject> *mr = prj.region();

  std::cout << mr->free_capacity() << std::endl;

  return 0;
}
