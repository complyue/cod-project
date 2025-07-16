## Unified Test Entry-Point (`run-all.sh`)

Each component directory under `tests/` contains a `run-all.sh` script that
• builds its test artefacts (if needed) and
• executes every test case for that component.

### Quick examples

```bash
# Build and run shilos C++ tests (default: using build/ toolchain)
./tests/shilos/run-all.sh

# Build yaml-cmp tool and run its self-check
./tests/yaml-cmp/run-all.sh

# Run CoDP shell tests (automatically builds yaml-cmp first)
./tests/codp/run-all.sh

# Use pre-release toolchain from built/ directory
COD_TEST_TOOLCHAIN=built ./tests/shilos/run-all.sh
```

`run-all.sh` scripts automatically:

* create a local `build/` directory and configure CMake (first invocation only);
* perform incremental builds (`cmake --build ./build`);
* select the appropriate toolchain based on the `COD_TEST_TOOLCHAIN` environment variable;
* prepend required tool-chain paths to `PATH` so that tools such as `codp`, `shilos` and `yaml-cmp` are resolved;
* forward the exit status of each individual test, failing fast on error **so the script's own exit code accurately reflects pass/fail status**.

> **Tip** – execute the entire test suite in one go from the project root:
>
> ```bash
> ./tests/shilos/run-all.sh && ./tests/codp/run-all.sh
> ```

All test folders **must** include a `run-all.sh` script to preserve this uniform
interface.

---

## Running Tests After Code Changes

### Development Workflow (Default)

For rapid development with test verification, use the default `build` toolchain mode:

```bash
# Build main sources once
cmake --build ./build

# Run tests directly against build/ directory (no install needed)
./tests/shilos/run-all.sh
./tests/codp/run-all.sh
```

The tests will automatically use the toolchain from `<project>/build/` which contains the latest compiled code. This eliminates the need to run `ninja install` after every change.

### Pre-Release Testing

For final verification before release, use the installed toolchain:

```bash
# Build and install main sources
cmake --build ./build && ninja -C ./build install

# Run tests against installed toolchain in built/
COD_TEST_TOOLCHAIN=built ./tests/shilos/run-all.sh
COD_TEST_TOOLCHAIN=built ./tests/codp/run-all.sh
```

### Toolchain Selection

The `COD_TEST_TOOLCHAIN` environment variable controls which toolchain tests use:

- **`build`** (default): Use `<project>/build/` - for development workflow
- **`built`**: Use `<project>/built/` - for pre-release testing

All test scripts respect this variable and default to `build` mode when not set.

---

## Authoring New Tests

Follow the patterns below when adding functionality:

### 1. C++ / CMake-based tests

* Create `tests/<component>/` (or add to an existing one).
* Provide source files and a `CMakeLists.txt` that respects the `COD_TEST_TOOLCHAIN` environment variable (see `tests/shilos/CMakeLists.txt` for reference).
* Mirror the `tests/shilos` build helper if you need multiple executables.
* Ensure your new directory contains a `run-all.sh` script that
  – sets `COD_TEST_TOOLCHAIN` default to "build",
  – configures `build/` on first run,
  – builds with `cmake --build`, and
  – executes **all** produced executables.
  – terminates immediately on the first failure (use `set -euo pipefail`).
  – keeps test sources in a dedicated `src/` folder and lists them from `CMakeLists.txt`.

*Add a proper `.gitignore` inside each test component dir, e.g. ignore `build*/` directory*

### 2. Shell / CLI-driven tests

* Place scripts inside `tests/<component>/<scenario>/`.
* The scenario directory **must** expose a `run.sh` entry point executed by the
  parent component's `run-all.sh`.
* Scripts should respect `COD_TEST_TOOLCHAIN` for setting up `PATH` (see `tests/codp/local_single/run.sh` for reference).
* Scripts should exit non-zero on failure and print minimal diagnostic output.
* Use the `yaml-cmp` utility (built in `tests/yaml-cmp`) for YAML comparisons
  instead of ad-hoc `grep`/`sed` logic.
* Scripts must be committed with the executable bit set (`chmod +x`) so they can be invoked directly by CI and other `run-all.sh` wrappers.

### 3. Cross-component dependencies

If your tests depend on another component's artifacts (e.g. CoDP tests requiring
`yaml-cmp`), invoke the other component's `run-all.sh` first **and rely on
`set -e` to abort if it fails**, then add its `build/` folder to `PATH` – see
`tests/codp/run-all.sh` for reference.

---

Following these conventions keeps the developer experience predictable and
ensures CI can run the full suite with a single command.
