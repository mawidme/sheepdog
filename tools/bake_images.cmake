# Renders the SVG sources in img/ to PNGs in assets/img/<scale>x/ for every image scale.
# The generated PNGs are committed; re-run this after changing or adding an SVG.
#
#   cmake [-DRESVG=<path to resvg>] -P tools/bake_images.cmake
#
# Requires resvg (https://github.com/linebender/resvg).
cmake_minimum_required(VERSION 3.22)

get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

# must match the scales known to the game (src/gfx/texture_cache.cpp)
set(SCALES 1 2 4)

# not used by the game (grass is drawn as lines), and huge at 4x
set(EXCLUDE grass.svg)

if(NOT RESVG)
    find_program(RESVG resvg HINTS "$ENV{USERPROFILE}/tools/resvg" "$ENV{HOME}/.cargo/bin")
endif()
if(NOT RESVG)
    message(FATAL_ERROR "resvg not found; install it or pass -DRESVG=<path>")
endif()

file(GLOB_RECURSE SVGS RELATIVE "${ROOT}/img" "${ROOT}/img/*.svg")
list(REMOVE_ITEM SVGS ${EXCLUDE})
list(SORT SVGS)

# start clean so renamed or deleted SVGs don't leave stale PNGs behind
file(REMOVE_RECURSE "${ROOT}/assets/img")

foreach(scale IN LISTS SCALES)
    foreach(svg IN LISTS SVGS)
        string(REGEX REPLACE "\\.svg$" ".png" png "${svg}")
        set(out "${ROOT}/assets/img/${scale}x/${png}")
        get_filename_component(dir "${out}" DIRECTORY)
        file(MAKE_DIRECTORY "${dir}")
        execute_process(
            COMMAND "${RESVG}" --zoom ${scale} "${ROOT}/img/${svg}" "${out}"
            RESULT_VARIABLE rc
            ERROR_VARIABLE err)
        if(NOT rc EQUAL 0)
            message(FATAL_ERROR "resvg failed for ${svg} at ${scale}x: ${err}")
        endif()
        if(err)
            message(WARNING "${svg} (${scale}x): ${err}")
        endif()
    endforeach()
    list(LENGTH SVGS count)
    message(STATUS "${scale}x: ${count} images")
endforeach()
