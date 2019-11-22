# CMake boilerplate project

Boilerplate project for application built with CMake.

## Features 
- Testing with GTest
- Sample for download external dependencies using `ExternalProject_Add` 
- Sample for pure CMake project `Assimp` as external dependency
- Sample for non-CMake single-header library `stb` as external dependency

## Usage 

Configure project
```
mkdir build
cmake ../
```

Execute target `ExternalDependencies` to download and compile all the dependencies 

```
cmake --build ./ --target ExternalDependencies --config Debug
```

And / or compile and install the rest of the project optimized for release with `MinSizeRel`

```
cmake --build ./ --target Install --config MinSizeRel
```

## Testing 

```
TBD
```