TEMPLATE = app
CONFIG += console warn_off release embed_manifest_exe
CONFIG -= app_bundle
QT += gui
SOURCES += cmsc427.cpp Image.cpp
HEADERS += Image.hpp
QMAKE_CXXFLAGS += -I/usr/local/include
unix:macx {
QMAKE_LFLAGS += -stdlib=libc++
QMAKE_CXXFLAGS += -stdlib=libc++
}
