TEMPLATE = app
CONFIG += qt warn_on debug embed_manifest_exe c++11
CONFIG -= app_bundle  
QT += gui opengl xml widgets 
FORMS += cmsc427.ui 
SOURCES += GLview.cpp cmsc427.cpp
HEADERS += GLview.hpp cmsc427.hpp
QMAKE_CXXFLAGS += -I/usr/local/include
unix:macx {
QMAKE_LFLAGS += -stdlib=libc++
QMAKE_CXXFLAGS += -stdlib=libc++
}
RESOURCES += resources.qrc
OTHER_FILES += simple.frag simple.vert
