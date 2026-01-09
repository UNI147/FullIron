cmake_minimum_required(VERSION 3.10)
project(FullIron LANGUAGES C CXX)

# компиляторы если они не найдены
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(NOT CMAKE_C_COMPILER)
        set(CMAKE_C_COMPILER "cl")
    endif()
    if(NOT CMAKE_CXX_COMPILER)
        set(CMAKE_CXX_COMPILER "cl")
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Включаем поддержку статической линковки для MSVC
if(MSVC)
    add_compile_options(/Zm200)
    add_compile_options("/bigobj")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

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
    # Основной файл
    main.cpp
    
    # Core
    Core/Engine.cpp
    Core/ResourceManager.cpp
    
    # GameObject
    GameObject/GameObject.cpp
    GameObject/Component.cpp
    
    # Scene
    Scene/Scene.cpp
    Scene/SceneManager.cpp
    Scene/MainScene.cpp
    Scene/ConfigParser.cpp
    Scene/SceneLoader.cpp
    
    # Components
    Components/Camera.cpp
    Components/Rotator.cpp
    Components/CubeRenderer.cpp
    
    # Graphics
    Graphics/Shader.cpp
    Graphics/CLUTTexture.cpp
    Graphics/LowResFramebuffer.cpp
    Graphics/PaletteManager.cpp
    Graphics/ZFightingManager.cpp
    Graphics/Cube.cpp
    Graphics/GraphicsSettings.cpp
)

# В заголовочные файлы
set(HEADERS
    # Core
    Core/Engine.h
    Core/ResourceManager.h
    
    # GameObject
    GameObject/GameObject.h
    GameObject/Component.h
    
    # Scene
    Scene/Scene.h
    Scene/SceneManager.h
    Scene/MainScene.h
    Scene/ConfigParser.h
    Scene/SceneLoader.h
    
    # Components
    Components/Camera.h
    Components/Rotator.h
    Components/CubeRenderer.h
    
    # Graphics
    Graphics/Shader.h
    Graphics/CLUTTexture.h
    Graphics/LowResFramebuffer.h
    Graphics/PaletteManager.h
    Graphics/GraphicsConfig.h
    Graphics/ZFightingManager.h
    Graphics/Cube.h
    Graphics/GraphicsSettings.h
    Graphics/shader_config.h
)

# Создаем исполняемый файл
add_executable(FullIron WIN32 ${SOURCES} ${HEADERS})

# Настройки компилятора для MSVC
if(MSVC)
    target_compile_definitions(FullIron PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
        GLM_FORCE_RADIANS
    )

    if(NOT TARGET glm)
        # Добавьте GLM как external dependency
        add_subdirectory(include/glm)
    endif()

    target_compile_options(FullIron PRIVATE 
        /W4 /EHsc /WX- 
        /Zm200
        /bigobj
    )
    
    # Используем статическую линковку времени выполнения
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Основные директории включения
target_include_directories(FullIron PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/GameObject
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene
    ${CMAKE_CURRENT_SOURCE_DIR}/Components
    ${CMAKE_CURRENT_SOURCE_DIR}/Graphics
    ${CMAKE_CURRENT_SOURCE_DIR}/glad
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
)

# Выходная директория для исполняемого файла
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Debug)

# Копирование шейдеров в ту же директорию, где будет исполняемый файл
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
    file(COPY shaders/ DESTINATION ${CMAKE_BINARY_DIR}/Debug/shaders)
    message(STATUS "Shaders copied to: ${CMAKE_BINARY_DIR}/Debug/shaders")
else()
    message(WARNING "Shaders directory not found at: ${CMAKE_CURRENT_SOURCE_DIR}/shaders")
endif()

# Копирование файла настроек графики
set(GRAPHICS_SETTINGS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/graphics_settings.ini")
if(EXISTS "${GRAPHICS_SETTINGS_FILE}")
    file(COPY "${GRAPHICS_SETTINGS_FILE}" DESTINATION ${CMAKE_BINARY_DIR}/Debug)
    message(STATUS "Graphics settings copied: ${GRAPHICS_SETTINGS_FILE} -> ${CMAKE_BINARY_DIR}/Debug")
else()
    message(WARNING "Graphics settings file not found at: ${GRAPHICS_SETTINGS_FILE}")
endif()

# Линковка библиотек
target_link_libraries(FullIron 
    ${OPENGL_LIBRARIES}
    opengl32
    glu32
    gdi32
    glad
)

# Копирование ресурсов в ту же директорию, где будет исполняемый файл
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources")
    file(COPY resources/ DESTINATION ${CMAKE_BINARY_DIR}/Debug/resources)
    message(STATUS "Resources copied: ${CMAKE_CURRENT_SOURCE_DIR}/resources -> ${CMAKE_BINARY_DIR}/Debug/resources")
else()
    message(WARNING "Resources directory not found at: ${CMAKE_CURRENT_SOURCE_DIR}/resources")
endif()

# Копируем папку сценариев
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/scenes")
    file(COPY scenes/ DESTINATION ${CMAKE_BINARY_DIR}/Debug/scenes)
    message(STATUS "Scene configurations copied")
else()
    message(WARNING "Scenes directory not found")
endif()

# Создаем выходную директорию если она не существует
if(NOT EXISTS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

# Для Windows: предотвращаем появление консоли в релизной сборке
if(WIN32)
    set_target_properties(FullIron PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()