cmake_minimum_required(VERSION 3.10)
project(notepadpp_gtk_proto LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
# Revert explicit inclusion of gdk-3 in pkg_check_modules
pkg_check_modules(GTK3 REQUIRED gtk+-3.0 gmodule-no-export-2.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${GTK3_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/scintilla/include ${CMAKE_SOURCE_DIR}/scintilla/gtk ${CMAKE_SOURCE_DIR}/lexilla ${CMAKE_SOURCE_DIR}/lexilla/access ${CMAKE_SOURCE_DIR}/lexilla/include ${CMAKE_SOURCE_DIR}/lexilla/lexlib)
link_directories(${GTK3_LIBRARY_DIRS} ${CMAKE_SOURCE_DIR}/scintilla/bin ${CMAKE_SOURCE_DIR}/lexilla/bin)
add_compile_definitions(GTK)

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/main_gui.cxx ${CMAKE_SOURCE_DIR}/lexilla/access/LexillaAccess.cxx)
add_executable(gtk-proto ${SRC})

target_compile_options(gtk-proto PRIVATE ${GTK3_CFLAGS_OTHER})
## Link to Scintilla (if present). GTK libraries will be appended afterwards to ensure proper ordering.

set(SCINTILLA_STATIC ${CMAKE_SOURCE_DIR}/scintilla/bin/scintilla.a)
set(SCINTILLA_SO ${CMAKE_SOURCE_DIR}/scintilla/bin/libscintilla.so)
if(EXISTS ${SCINTILLA_STATIC})
    target_link_libraries(gtk-proto PRIVATE ${SCINTILLA_STATIC})
elseif(EXISTS ${SCINTILLA_SO})
    target_link_libraries(gtk-proto PRIVATE ${SCINTILLA_SO})
else()
    message(STATUS "Scintilla library not found; will attempt to build it before building the prototype")
    add_custom_target(build_scintilla
        COMMAND ${CMAKE_MAKE_PROGRAM} -C ${CMAKE_SOURCE_DIR}/scintilla/gtk GTK3=1
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_dependencies(gtk-proto build_scintilla)
    # After building, try linking to static if present
    if(EXISTS ${SCINTILLA_STATIC})
        target_link_libraries(gtk-proto PRIVATE ${SCINTILLA_STATIC})
    elseif(EXISTS ${SCINTILLA_SO})
        target_link_libraries(gtk-proto PRIVATE ${SCINTILLA_SO})
    endif()
endif()

## Optionally link Lexilla if present so MakeLexer/CreateLexer are available statically
set(LEXILLA_STATIC ${CMAKE_SOURCE_DIR}/lexilla/bin/liblexilla.a)
set(LEXILLA_SO ${CMAKE_SOURCE_DIR}/lexilla/bin/liblexilla.so)
if(EXISTS ${LEXILLA_STATIC})
    target_link_libraries(gtk-proto PRIVATE ${LEXILLA_STATIC})
elseif(EXISTS ${LEXILLA_SO})
    target_link_libraries(gtk-proto PRIVATE ${LEXILLA_SO})
endif()

## Now append GTK libraries so they appear after Scintilla on the link line
target_link_libraries(gtk-proto PRIVATE ${GTK3_LIBRARIES} gdk-3 gdk_pixbuf-2.0 cairo gio-2.0 pangocairo-1.0 gobject-2.0 glib-2.0)

# Create notepad++ executable from the same sources
add_executable(notepad++ ${SRC})

## Link Scintilla to notepad++ target
if(EXISTS ${SCINTILLA_STATIC})
    target_link_libraries(notepad++ PRIVATE ${SCINTILLA_STATIC})
elseif(EXISTS ${SCINTILLA_SO})
    target_link_libraries(notepad++ PRIVATE ${SCINTILLA_SO})
endif()

## Link Lexilla to notepad++ target
if(EXISTS ${LEXILLA_STATIC})
    target_link_libraries(notepad++ PRIVATE ${LEXILLA_STATIC})
elseif(EXISTS ${LEXILLA_SO})
    target_link_libraries(notepad++ PRIVATE ${LEXILLA_SO})
endif()

## Link GTK libraries
target_link_libraries(notepad++ PRIVATE ${GTK3_LIBRARIES} gdk-3 gdk_pixbuf-2.0 cairo gio-2.0 pangocairo-1.0 gobject-2.0 glib-2.0)

# Install the binary to /usr/local/bin
install(TARGETS notepad++
        RUNTIME DESTINATION /usr/local/bin)

# Add linker flag to ensure all libraries are linked
target_link_options(gtk-proto PRIVATE -Wl,--no-as-needed)
target_link_options(notepad++ PRIVATE -Wl,--no-as-needed)

