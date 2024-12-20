
#include <iostream>

#include "codp.hh"

using namespace shilos;
using namespace cod::project;

int main(int argc, char **argv) {
  std::cout << "codp is the cmdl UI to edit Project.codp file with a text editor" << std::endl;

  if (argc > 2) {

    const DBMR<CodProject> prj = DBMR<CodProject>::read("cod.project");
    const memory_region<CodProject> *mr = prj.region();
    std::cout << mr->free_capacity() << std::endl;
  } else {

    DBMR<CodProject> prj = DBMR<CodProject>("cod.project", 10 * 1024 * 1024).constrict_on_close();
    memory_region<CodProject> *mr = prj.region();
    std::cout << mr->free_capacity() << std::endl;
  }

  return 0;
}
