QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HJ-Editor
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    Assembler_Simulator/Assembler_main.cpp \
    Compile_main.cpp \
    Intermediate_Code_Generator/c_to_mips.cpp \
    Intermediate_Code_Generator/translate.cpp \
    Lexer_Parser/Lexer.cpp \
    Lexer_Parser/Parser.cpp \
    Lexer_Parser/env.cpp \
        main.cpp \
        mainwindow.cpp \
    codeeditor.cpp \
    highlighter.cpp \
    completelistwidget.cpp \
    console.cpp

HEADERS += \
    Assembler_Simulator/Assembler_main.h \
    Assembler_Simulator/Itype.h \
    Assembler_Simulator/Jtype.h \
    Assembler_Simulator/Rtype.h \
    Compile_main.h \
    Global.h \
    Intermediate_Code_Generator/c_to_mips.h \
    Intermediate_Code_Generator/defines.h \
    Lexer_Parser/Lexer.h \
    Lexer_Parser/Parser.h \
    Lexer_Parser/Program.h \
    Lexer_Parser/Token.h \
    Lexer_Parser/env.h \
        mainwindow.h \
    codeeditor.h \
    highlighter.h \
    completelistwidget.h \
    console.h

FORMS += \
        mainwindow.ui

RESOURCES += \
    image.qrc

ICON = icon.icns
