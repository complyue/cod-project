
#include <iostream>

#include "codp.hh"

using namespace shilos;
using namespace cod::project;

int main(int argc, char **argv) {
  std::cout << "codp is the cmdl UI to edit Project.codp file with a text editor" << std::endl;

  if (argc > 2) {

    const DBMR<CodProject> prj = DBMR<CodProject>::read("cod.project");
    const memory_region<CodProject> &mr = prj.region();
    std::cout << mr.occupation() << " + " << mr.free_capacity() << " / " << mr.capacity() << std::endl;
    std::cout << mr.root()->name() << std::endl;
  } else if (argc > 1) {

    DBMR<CodProject> prj = DBMR<CodProject>("cod.project", 10 * 1024 * 1024);
    prj.constrict_on_close();
    memory_region<CodProject> &mr = prj.region();
    std::cout << mr.occupation() << " + " << mr.free_capacity() << " / " << mr.capacity() << std::endl;
    std::cout << mr.root()->name() << std::endl;
  } else {

    DBMR<CodProject> prj = DBMR<CodProject>::create("cod.project", 10 * 1024 * 1024, "some cool project name");
    prj.constrict_on_close();
    memory_region<CodProject> &mr = prj.region();
    std::cout << mr.occupation() << " + " << mr.free_capacity() << " / " << mr.capacity() << std::endl;
    std::cout << mr.root()->name() << std::endl;
  }

  return 0;
}
