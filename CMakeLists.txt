cmake_minimum_required(VERSION 4.1)
project(marbles)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(.)
include_directories(lib)
include_directories(lib/FreeImage)
include_directories(lib/FreeImage/inc)
include_directories(lib/OpenGL)
include_directories(lib/SDL2-2.0.3)
include_directories(lib/SDL2-2.0.3/include)
include_directories(tmpl8)

add_library(freeimg SHARED IMPORTED)
set_property(TARGET freeimg PROPERTY IMPORTED_IMPLIB C:/Users/eloyr/Projects/marbles/lib/FreeImage/lib64/freeimage.lib)

find_package(OpenGL32 REQUIRED CONFIG REQUIRED COMPONENTS OpenGL32)

add_library(glew SHARED IMPORTED)
set_property(TARGET glew PROPERTY IMPORTED_LOCATION C:/Users/eloyr/Projects/marbles/dlls_x64/glew32.dll)
set_property(TARGET glew PROPERTY IMPORTED_IMPLIB C:/Users/eloyr/Projects/marbles/lib/glew-2.3.1/lib/Release/x64/glew32s.lib)

add_library(SDL2 SHARED IMPORTED)
add_library(SDL2main SHARED IMPORTED)
set_property(TARGET SDL2 PROPERTY IMPORTED_LOCATION C:/Users/eloyr/Projects/marbles/lib/SDL2-2.0.3/lib/x64/SDL2.dll)
set_property(TARGET SDL2 PROPERTY IMPORTED_IMPLIB C:/Users/eloyr/Projects/marbles/lib/SDL2-2.0.3/lib/x64/SDL2.lib)
set_property(TARGET SDL2main PROPERTY IMPORTED_IMPLIB C:/Users/eloyr/Projects/marbles/lib/SDL2-2.0.3/lib/x64/SDL2main.lib)

# Create your game executable target as usual
add_executable(marbles
        lib/FreeImage/inc/FreeImage.h
        lib/OpenGL/gl.h
        lib/OpenGL/glew.h
        lib/OpenGL/wglew.h
        lib/OpenGL/wglext.h
        lib/SDL2-2.0.3/include/begin_code.h
        lib/SDL2-2.0.3/include/close_code.h
        lib/SDL2-2.0.3/include/SDL.h
        lib/SDL2-2.0.3/include/SDL_assert.h
        lib/SDL2-2.0.3/include/SDL_atomic.h
        lib/SDL2-2.0.3/include/SDL_audio.h
        lib/SDL2-2.0.3/include/SDL_bits.h
        lib/SDL2-2.0.3/include/SDL_blendmode.h
        lib/SDL2-2.0.3/include/SDL_clipboard.h
        lib/SDL2-2.0.3/include/SDL_config.h
        lib/SDL2-2.0.3/include/SDL_cpuinfo.h
        lib/SDL2-2.0.3/include/SDL_endian.h
        lib/SDL2-2.0.3/include/SDL_error.h
        lib/SDL2-2.0.3/include/SDL_events.h
        lib/SDL2-2.0.3/include/SDL_filesystem.h
        lib/SDL2-2.0.3/include/SDL_gamecontroller.h
        lib/SDL2-2.0.3/include/SDL_gesture.h
        lib/SDL2-2.0.3/include/SDL_haptic.h
        lib/SDL2-2.0.3/include/SDL_hints.h
        lib/SDL2-2.0.3/include/SDL_joystick.h
        lib/SDL2-2.0.3/include/SDL_keyboard.h
        lib/SDL2-2.0.3/include/SDL_keycode.h
        lib/SDL2-2.0.3/include/SDL_loadso.h
        lib/SDL2-2.0.3/include/SDL_log.h
        lib/SDL2-2.0.3/include/SDL_main.h
        lib/SDL2-2.0.3/include/SDL_messagebox.h
        lib/SDL2-2.0.3/include/SDL_mouse.h
        lib/SDL2-2.0.3/include/SDL_mutex.h
        lib/SDL2-2.0.3/include/SDL_name.h
        lib/SDL2-2.0.3/include/SDL_opengl.h
        lib/SDL2-2.0.3/include/SDL_opengles.h
        lib/SDL2-2.0.3/include/SDL_opengles2.h
        lib/SDL2-2.0.3/include/SDL_pixels.h
        lib/SDL2-2.0.3/include/SDL_platform.h
        lib/SDL2-2.0.3/include/SDL_power.h
        lib/SDL2-2.0.3/include/SDL_quit.h
        lib/SDL2-2.0.3/include/SDL_rect.h
        lib/SDL2-2.0.3/include/SDL_render.h
        lib/SDL2-2.0.3/include/SDL_revision.h
        lib/SDL2-2.0.3/include/SDL_rwops.h
        lib/SDL2-2.0.3/include/SDL_scancode.h
        lib/SDL2-2.0.3/include/SDL_shape.h
        lib/SDL2-2.0.3/include/SDL_stdinc.h
        lib/SDL2-2.0.3/include/SDL_surface.h
        lib/SDL2-2.0.3/include/SDL_system.h
        lib/SDL2-2.0.3/include/SDL_syswm.h
        lib/SDL2-2.0.3/include/SDL_test.h
        lib/SDL2-2.0.3/include/SDL_test_assert.h
        lib/SDL2-2.0.3/include/SDL_test_common.h
        lib/SDL2-2.0.3/include/SDL_test_compare.h
        lib/SDL2-2.0.3/include/SDL_test_crc32.h
        lib/SDL2-2.0.3/include/SDL_test_font.h
        lib/SDL2-2.0.3/include/SDL_test_fuzzer.h
        lib/SDL2-2.0.3/include/SDL_test_harness.h
        lib/SDL2-2.0.3/include/SDL_test_images.h
        lib/SDL2-2.0.3/include/SDL_test_log.h
        lib/SDL2-2.0.3/include/SDL_test_md5.h
        lib/SDL2-2.0.3/include/SDL_test_random.h
        lib/SDL2-2.0.3/include/SDL_thread.h
        lib/SDL2-2.0.3/include/SDL_timer.h
        lib/SDL2-2.0.3/include/SDL_touch.h
        lib/SDL2-2.0.3/include/SDL_types.h
        lib/SDL2-2.0.3/include/SDL_version.h
        lib/SDL2-2.0.3/include/SDL_video.h
        tmpl8/surface.cpp
        tmpl8/surface.h
        tmpl8/template.cpp
        tmpl8/template.h
        game.cpp
        game.h
)

target_link_libraries(marbles freeimg)
target_link_libraries(marbles OpenGL32)
target_link_libraries(marbles glew)

# SDL2::SDL2main may or may not be available. It is e.g. required by Windows GUI applications
if(TARGET SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(marbles SDL2main)
endif()

# Link to the actual SDL2 library. SDL2::SDL2 is the shared SDL library, SDL2::SDL2-static is the static SDL libarary.
target_link_libraries(marbles SDL2)
