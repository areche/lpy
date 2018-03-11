TARGET = Lpy
TEMPLATE = lib

QT -= gui

CONFIG += create_prl link_prl c++11

INCLUDEPATH += $$top_srcdir/plantgl/src/cpp
INCLUDEPATH += /usr/local/include
INCLUDEPATH += C:/DevTools/Boost/include/boost-1_63

win32 {
    build_pass:CONFIG(debug, debug|release) {
        LIBS += -L$$top_builddir/plantgl/src/cpp/plantgl/algo/debug -lAlgo
    } else {
        LIBS += -L$$top_builddir/plantgl/src/cpp/plantgl/algo/release -lAlgo
    }
} else {
    LIBS += -L$$top_builddir/plantgl/src/cpp/plantgl/algo -lAlgo
}

macx {
    QMAKE_APPLE_DEVICE_ARCHS="x86_64"
}

SOURCES = \
    axialtree.cpp \
    consider.cpp \
    debug_tool.cpp \
    error.cpp \
    interpretation.cpp \
    lpy_parser.cpp \
    lsyscontext.cpp \
    matching.cpp \
    module.cpp \
    moduleclass.cpp \
    modulevtable.cpp \
    nodemodule.cpp \
    predefinedmodules.cpp \
    tracker.cpp

# Module.cpp -> Tout sauf ParamModule, new ParamModule with boost::any

HEADERS = \
    abstractlstring.h \
    argcollector.h \
    argcollector_core.h \
    axialtree.h \
    axialtree_iter.h \
    axialtree_manip.h \
    consider.h \
    debug_tool.h \
    error.h \
    global.h \
    interpretation.h \
    lpy_config.h \
    lpy_parser.h \
    lsyscontext.h \
    matching.h \
    module.h \
    moduleclass.h \
    modulevtable.h \
    nodemodule.h \
    packedargs.h \
    tracker.h \
    lpy_variant.h \
    copy_on_write_ptr.h \
    mutex_flag.h

UnityModule {
    QT -= core
    CONFIG += static

    DEFINES += UNITY_MODULE

    DEFINES += LPY_WITHOUT_QT
    DEFINES += LPY_NODLL

    DEFINES += PGL_WITHOUT_QT
    DEFINES += ALGO_NODLL
    DEFINES += PGLMATH_NODLL
    DEFINES += SG_NODLL
    DEFINES += TOOLS_NODLL
} else {
    DEFINES += USING_PYTHON
    LIBS += -L/usr/local/lib -lboost_python
    INCLUDEPATH += /usr/include/python2.7
    LIBS += -L/usr/lib/python2.7/config -lpython2.7

    QT += concurrent
    LIBS += -L$$top_builddir/plantgl/src/cpp/plantgl/gui -lGui

    SOURCES += \
        compilation.cpp \
        execution.cpp \
        lstringmatcher.cpp \
        lsysoptions.cpp \
        lsysrule.cpp \
        lsystem.cpp \
        paramproduction.cpp \
        patternmodule.cpp \
        patternstring.cpp \
        plot.cpp \
        stringmatching.cpp

    HEADERS += \
        compilation.h \
        execution.h \
        lstringmatcher.h \
        lsysoptions.h \
        lsysrule.h \
        lsystem.h \
        matching_tmpl.h \
        paramproduction.h \
        patternmodule.h \
        patternstring.h \
        plot.h \
        stringmatching.h
}
