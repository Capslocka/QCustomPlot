QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11
lessThan(QT_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -std=c++11

TARGET = sync-demo
TEMPLATE = app

INCLUDEPATH += ../../src ../../src/tools

SOURCES += main.cpp \
    mainwindow.cpp \
    $$files(../../src/*.cpp) \
    $$files(../../src/tools/*.cpp)

HEADERS += mainwindow.h \
    $$files(../../src/*.h) \
    $$files(../../src/tools/*.h)
