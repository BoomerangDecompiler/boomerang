
set(boomerang-cli_SRC
	main.cpp
	commandlinedriver.cpp
	commandlinedriver.h
)

add_executable(boomerang-cli
	${boomerang-cli_SRC}
)

target_link_libraries(boomerang-cli
	boomerang
	boomerang-frontend
	${CMAKE_DL_LIBS}
)

qt5_use_modules(boomerang-cli Core)

install(TARGETS boomerang-cli
	RUNTIME DESTINATION bin
)

