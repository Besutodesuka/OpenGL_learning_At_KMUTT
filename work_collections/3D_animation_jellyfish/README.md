# about this project
this is 3D Animation that wou can walk and watch them. I create library for rendering mesh (mesh.h) from input vertice, indice and normal vector for compute howlight shoul reflect. I made class in facade pattern to render all 3 mesh which are part of jellyfish all together and enable its animation.

to generate vertices and indices for some mesh, I used tools below to assist me on crafting equations and visualize it in blender program first from generated python script. script are stored in **matrix_model** folder

# tools used for create mesh
- to map vertice using matrix transformation
    - video tutorial: https://www.youtube.com/watch?v=W4ViqzbF96Q&list=LL&index=26&t=175s
    - tool link: https://otvinta.com/matrix.html
- to create fluffy crossection I use Epicycloid to create flower like surface: http://xahlee.info/SpecialPlaneCurves_dir/EpiHypocycloid_dir/epiHypocycloid.html

# demo video
[![Video Title](https://img.youtube.com/vi/2AGqep__qQY/0.jpg)](https://www.youtube.com/watch?v=2AGqep__qQY)


# file heirachy
```
|--includes
|   |--go
|   |   |--mesh.h
|   |--learnopengl
|       |--camera.h
|--resources
|    |--textures
|       |--reflected_water.jpg
|       |--sand.jpg
|--src
    |--2.lighting
        |--6.multiple_lights
            |--6.light_cube.fs
            |--6.light_cube.vs
            |--6.multiple_lights.fs    
            |--6.multiple_lights.fs          
            |--multiple_lights.cpp 
```

# texture used credit
- jellyfish body texture from sea picture:https://www.istockphoto.com/th/%E0%B8%A3%E0%B8%B9%E0%B8%9B%E0%B8%96%E0%B9%88%E0%B8%B2%E0%B8%A2/%E0%B8%A3%E0%B8%B0%E0%B8%A5%E0%B8%AD%E0%B8%81%E0%B8%84%E0%B8%A5%E0%B8%B7%E0%B9%88%E0%B8%99%E0%B8%99%E0%B9%89%E0%B9%8D%E0%B8%B2%E0%B9%80%E0%B8%AB%E0%B8%99%E0%B8%B7%E0%B8%AD%E0%B8%AB%E0%B8%B2%E0%B8%94%E0%B8%97%E0%B8%A3%E0%B8%B2%E0%B8%A2-gm157572574-12271117
- sand ground: chatgpt generated with prompt 
```
generate tileable sand texture
```