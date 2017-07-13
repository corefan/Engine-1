SET(SOURCES_PUBLIC 
	"dll.h"
	"manager.h"
	"task_graph.h"
	"types.h"
)

SET(SOURCES_PRIVATE 
	"private/internal_types.h"
	"private/manager.cpp"
	"private/task_graph.cpp"
)

SET(SOURCES_TESTS
	"tests/test_entry.cpp"
	"tests/job_tests.cpp"
)

ADD_ENGINE_LIBRARY(job ${SOURCES_PUBLIC} ${SOURCES_PRIVATE} ${SOURCES_TESTS})
TARGET_LINK_LIBRARIES(job core)
