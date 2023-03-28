if (EPOLL_EXISTS)
    set(npd_lib
            error.c
            read.c
            )
else ()
    set(npd_lib
            error.c
            read.c
            )
endif ()


add_library(npd ${npd_lib})
target_link_libraries(npd pthread)

install(TARGETS npd DESTINATION lib)

#file(GLOB HEADERS "*.h")
#install(FILES ${HEADERS} DESTINATION include/)

