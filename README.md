# Vazteran

Vulkan based rendering engine for learning purposes. It main goal is to make available a simpler way 
to render my small projects related to computer graphics. 

Here is the roadmap :
| Feature                                                         | Progress  |
|-----------------------------------------------------------------|-----------|
| Render Graph                                                    | WIP       |
| Ui Library                                                      | WIP       |
| Geometry helping library                                        | To Do     |
| ECS                                                             | To Do     |

## Build 

To clone the project and its dependencies:
```
git clone --recurse-submodules -j8 https://github.com/PlathC/Udock.git
```

Most of the dependencies are embedded but you must install Python3 and Vulkan SDK on your own.

The project uses CMake, to build it you may use the following command or any IDE supporting it:
```
cd Vazteran
mkdir build && cd build
cmake ..
cmake --build . --target Vazteran --config Release
```

## Current state

Render graph-based deferred rendering can be easily implemented:

![](./readmefiles/Texture.png)

![](./readmefiles/MultipleObject.png)

## Samples 

- [The Crounching Boy Open source model by SMK - Statens Museum for Kunst Creative Commons Attribution 4.0 International License](https://www.myminifactory.com/object/3d-print-the-crouching-boy-104413)
- [Mori Knob Open source model by Yasutoshi "Mirage" Mori Creative Commons Attribution 4.0 International License](https://github.com/lighttransport/lighttransportequation-orb)
- [Viking Room Open source model by nigelgoh Creative Commons Attribution 4.0 International License](https://sketchfab.com/3d-models/viking-room-a49f1b8e4f5c4ecf9e1fe7d81915ad38)
