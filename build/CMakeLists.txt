
cmake_minimum_required(VERSION 3.5)


project(FileSystemReImagined VERSION 1.0)
add_executable(
    SmartFileMapper 
    ../main.c 
    ../Statistics/Clusterer.c
    ../CustomDS/ArrayList.c
    ../FancyTerminal/Renderer/FancyTerminal.c
    ../FancyTerminal/Keyboard/keyboard.c
) 

    