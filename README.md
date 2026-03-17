# common
C++ Common Libraries and utilities

## Purpose
TODO:

## Example Usage

## BUild Instructions

### 1. Install Conan (v2)

- On Windows or Linux, use pip:
  ```sh
  pip install conan
  ```
- Or see the official docs: https://docs.conan.io/2/installation.html


### 2. Install dependencies with Conan

  ```sh
  conan install ./conan/conanfile.py --output-folder=build/conan --build=missing --profile:all=./conan/profile/linux -s:a build_type=Release"
  ```

  Replace `--profile:all=./conan/profile/linux` with `--profile:all=./conan/profile/windows` for Windows builds.

  **Note for Linux debug builds:**
If your application or tests are built with `_GLIBCXX_DEBUG` (common for debug builds with GCC), you must also build all Conan dependencies with this define to avoid ABI incompatibility and runtime errors. Run Conan with:

```
conan install ./conan/conanfile.py --output-folder=build/conan --build=missing --profile:all=./conan/profile/linux -s:a build_type=Debug -c tools.build:defines=["_GLIBCXX_DEBUG"]
```

### 3. Configure build and run tests with CMake

- **Linux**
  ```sh
  cmake --preset linux
  cmake --build --preset linux-release
  ctest --preset linux-test
  ```
  
  ### 4. Generate documentation
- **Linux**
  ```sh
  cmake --build ./build/linux --target doc
  ``` 

- **Windows**
  ```sh
  cmake --build ./build/windows --target doc
  ``` 

---
For more details, see the CMakePresets.json and conanfile.py files.