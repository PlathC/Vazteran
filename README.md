# Vazteran

Vazteran is my sandbox for graphics and compute projects. It aims at diminishing the boilerplate required for Vulkan project while maintaining a high level of features.

Here is the roadmap :
| Feature                                                         | Progress  |
|-----------------------------------------------------------------|-----------|
| Window                                                          | ✅        |
| Base Vulkan wrapper                                             | ✅        |
| Render graph                                                    | ✅        |
| Instancing                                                      | ✅        |
| Compute shader                                                  | ✅        |
| Indirect rendering                                              | ✅        |
| Raytracing extension                                            | ✅        |
| slang                                                           | ✅        |
| New simpler API                                                 |           |

## Current state

Since the RenderGraph, multiple projects were based on Vazteran and benefited from its simple configuration. 
However, I still feel that the API can be made simpler and more automatic for end users. 
I'm currently applying upgrades towards this direction.

## Build 

To clone the project and its dependencies:
```
git clone --recurse-submodules -j8 https://github.com/PlathC/Vazteran.git
```

Most of the dependencies are embedded but you must install Python3 and Vulkan SDK on your own.

The project uses CMake, to build it you may use the following command or any IDE supporting it:
```
cd Vazteran
mkdir build && cd build
cmake ..
cmake --build . --target Vazteran --config Release
```
