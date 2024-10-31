# add hardcoded guesses for the location of Vmb to CMAKE_PREFIX_PATH
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/../..)
if(DEFINED ENV{VMB_HOME})
    list(APPEND CMAKE_PREFIX_PATH $ENV{VMB_HOME}/api)
endif()
if(DEFINED ENV{VIMBA_X_HOME})
    list(APPEND CMAKE_PREFIX_PATH $ENV{VIMBA_X_HOME}/api)
endif()