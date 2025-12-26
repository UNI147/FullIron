cmake_minimum_required(VERSION 3.10)
project(FullIron)

set(CMAKE_CXX_STANDARD 11)

# Исправляем пути
set(GLFW_DIR "C:/FullIron/include/glfw-3.3.8.bin.WIN64")

# Ищем OpenGL
find_package(OpenGL REQUIRED)

# Проверяем GLAD
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/glad/glad.c")
    message(STATUS "GLAD found at: ${CMAKE_CURRENT_SOURCE_DIR}/glad")
    add_library(glad STATIC glad/glad.c)
    target_include_directories(glad PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
else()
    message(FATAL_ERROR "GLAD not found. Please place glad.c and glad.h in glad/ directory")
endif()

# Исходные файлы
set(SOURCES
    src/main.cpp
    src/Shader.cpp
    src/Texture.cpp
    src/Tetrahedron.cpp
)

# Добавляем заголовочные файлы
set(HEADERS
    src/Shader.h
    src/Texture.h
    src/Tetrahedron.h
)

# Создаем исполняемый файл
add_executable(FullIron WIN32 ${SOURCES} ${HEADERS})

# Настройки компилятора для MSVC
if(MSVC)
    target_compile_definitions(FullIron PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX  # Отключаем min/max макросы Windows
    )
    target_compile_options(FullIron PRIVATE /W4 /EHsc /WX-)
    
    # Используем статическую линковку времени выполнения
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Проверяем и настраиваем GLFW
if(EXISTS "${GLFW_DIR}")
    message(STATUS "Found GLFW at: ${GLFW_DIR}")
    
    # Добавляем include директории GLFW
    target_include_directories(FullIron PRIVATE 
        ${GLFW_DIR}/include
    )
    
    # Добавляем библиотечные директории
    target_link_directories(FullIron PRIVATE
        ${GLFW_DIR}/lib-vc2022
    )
    
    # Ссылаемся на библиотеку GLFW
    target_link_libraries(FullIron glfw3)
    
    # Определяем, что используем GLFW
    target_compile_definitions(FullIron PRIVATE USE_GLFW)
else()
    # Если GLFW нет, используем простое OpenGL окно
    message(STATUS "GLFW not found, using simple OpenGL context")
    target_compile_definitions(FullIron PRIVATE USE_SIMPLE_OPENGL)
endif()

# Основные директории включения
target_include_directories(FullIron PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/glad  # добавляем директорию glad
)

# Линковка библиотек
target_link_libraries(FullIron 
    ${OPENGL_LIBRARIES}
    opengl32
    glu32
    gdi32
    glad
)

# Копирование ресурсов
file(COPY resources/ DESTINATION ${CMAKE_BINARY_DIR}/bin/resources)

# Выходная директория
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)