TEMPLATE	=app
CONFIG	+= qt warn_on release
LANGUAGE	= C++
HEADERS = mainform.h detailswidget.h filemonitor.h util.h
SOURCES = main.cpp mainform.cpp detailswidget.cpp filemonitor.cpp ../util/util.cpp
INCLUDEPATH = ../include
