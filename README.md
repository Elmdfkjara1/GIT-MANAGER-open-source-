# Git Manager

Git Manager is a lightweight, high-performance alternative command-line interface for Git. Designed with a strong emphasis on speed and efficiency, it is written in C++ while deliberately bypassing heavy abstractions like `std::ostream` in favor of direct C standard library (`glibc`) system calls to eliminate hidden runtime overheads.

Current Features:
* __Commits__: Rapidly package and record changes to your local repository.
* __Staging__: Efficiently manage and stage modifications before committing.
* __Branching__: Streamline branch creation and navigation.

## Roadmap

* __Remotes__: Full integration with remote repositories.
* __Sync Operations__: Native support for pull and push workflows.

## Installation

Make sure you have `OpenSSL` and `zlib` installed, then configure and build the project using your preferred compiler:
```Bash

git clone https://github.com/Elmdfkjara1/GIT-MANAGER-open-source-.git

cmake -B build
cmake --build build
```
Run the application with:

```Bash
./build/gitManager
```

Or move the binary to your directory of choice.

---

## License
GPLv3