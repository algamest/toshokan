cmake_minimum_required(VERSION 3.1.0)

project(Toshokan)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Widgets Sql REQUIRED)

add_executable(${PROJECT_NAME}
	bookshow.cpp
	bookshow.ui
	maddwidget.cpp
	maddwidget.ui
	main.cpp
	saddwidget.cpp
	saddwidget.ui
	toshadd.cpp
	toshview.cpp
	toshview.ui
	toshores.qrc
)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Sql)
