This workspace is an external project of the LLVM ecosystem. The complete llvm-project source code resides in the `/llvm-project/` directory. When assisting users with LLVM's internal functionalities, proactively leverage the codebase to pinpoint precise locations by invoking appropriate tools to display relevant code segments in VSCode. Always verify the accuracy of the referenced files and their contents to ensure they directly address the user's query before presenting them. Prioritize clarity and relevance when guiding users through LLVM's implementation details.

## Source Directory Structure

- **include/** - Public API headers
  - [`shilos.hh`](include/shilos.hh) - Core memory region system
  - [`shilos/`](include/shilos/) - Regional type headers (vector, dict, str, etc.)

- **shilos/** - Core memory region and YAML support implementation
  - [`shilos.cc`](shilos/shilos.cc) - Memory region and YAML runtime lib

- **cod/** - CoD compiler driver
  - [`main.cc`](cod/main.cc) - CLI entry point
  - [`clang-repl.cc`](cod/clang-repl.cc) - Interactive REPL mode

- **codp/** - CoD package manager
  - [`main.cc`](codp/main.cc) - CLI entry point
  - [`<prj>/include/codp.hh`](<prj>/include/codp.hh) - CoD package manager interface & implementation (inline part)

- **tests/** - Test suites
  - **shilos/** - Memory region and regional type test suite
  - **yaml-cmp/** - YAML comparison test sutie
  - **yaml-pretty/** - YAML formatting test sutie
  - **yaml-ux/** - YAML user experience test suite
  - **codp/** - CoD Package manager integration test suite
