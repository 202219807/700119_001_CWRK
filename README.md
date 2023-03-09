### 001_CWRK:ACW1: Procedural Visual Effects in HLSL

# PROCEDRUAL SEA LIFE GRAPHICS EFFECTS IN HLSL

The aim of the assignment is to allow you to gain practical experience of programming graphics effects in HLSL targeting graphics hardware supporting shader model 5 profile and above. The main tools and development environment for the completion of the coursework will be Microsoft Visual Studio 2019 or 2022 and DirectX SDK (a part of Windows SDK).  


## EFFECTS TO BE IMPLEMENTED

In this assignment, you are required to develop a Direct3D 11 (or Direct3D 12) UWP app by writing a set of shaders in HLSL to implement a set of procedurally generated visual effects which can be used in a sea life-based game. All the objects rendered in the scene are assumed to be created procedurally either as explicit triangle meshes and rendered by using vertex shaders, geometry shaders and tessellation shaders, or in implicit form as real functions and rendered with ray tracing or ray marching technique implemented in pixel shaders. Each graphics object needs to be properly textured, illuminated. For dynamic objects, such as sea bubbles and different types of fish, they should also be realistically animated.


The effects to be implemented may include the following:

## 1. BASIC EFFECTS (60%)

(1) Underwater environment. This effect can be implemented in a number of different ways, varying from texture mapping, cube mapping to ray tracing. The marks you earn for this effect depends on the level of effort and the level of difficulty of the method you use to implement the effect. You may implement the effect based on texture mapping or cube mapping techniques, but must present sufficient evidence of effort in texture processing to create a realistic sea’s underwater atmosphere. (5%-10%).


(2) Procedural sea floor. This effect can be either implemented in explicit form as a triangle mesh generated using hull shader and domain shader or implemented in implicit form as a kind of distance function and visualized using a pixel shader. It is up to you to decide which method to use and you marks will be awarded entirely based on the visual quality of the effect (10% )

(3) Coral objects. Create FOUR different types of coral objects

- Vertex shader-based coral object. Generate a coral object procedurally using a vertex shader. (5% )

- Pixel shader-based coral object. Model a coral object implicitly as an implicit function and visualize the object based on ray tracing/marching. (7% )

- Geometry shader-based coral object. Input into a geometry shader a simple triangle mesh and refine the input triangle mesh using a geometry shader to turn it into a certain coral object with relatively complex surface details. (8% )

- Hull and domain shader-based coral objects. Model and generate two or three coral objects as parametric surfaces directly using hull and domain shaders. For these objects, the option to view  them as wireframe needs to be introduced to visually observe how these objects are tessellated with the change of tessellation details. (10%) 
There is a variety of coral objects. It is up to you to decide what kind of coral objects are to be modelled.


(4) Reflective bubbles. A collection of shiny bubbles emitted from the coral floor. This effect can be implemented in a pixel shader by modelling the bubbles as implicit functions and visualized using the ray tracing/marching method. You will earn extra marks as own effects if some non-spherical bubbles are implemented. (10%)


## 2. ADVANCED AND OWN EFFECTS (35%)

(1) A shoal of colourful coral reef fish created as a particle system.
- Create a list of vertices in your C++ program and write a geometry shader to turn each vertex in the list into a quad centred at the original vertex position with a size of [-1, 1] x [-1, 1] (5%)

- Regard the collection of quads generated from the geometry shader as a collection of quad particles and use them to animate a shoal of colourful coral reef fish. (5%)

(2) Implicit sea plant modelling and animation. Model and animate some sea plants either as an explicit object in vertex shader or geometry shaders,  or as a kind of distance function and visualized using the ray marching technique. (10%)

(3) Novelty and own effects. Implement some novel and advanced effects of your own in HLSL to enhance the overall visual quality of the scene you have created. (15%)
