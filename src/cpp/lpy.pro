include(../../config.pri)

TARGET = Lpy
TEMPLATE = lib

QT -= gui

CONFIG += create_prl link_prl c++14

DESTDIR = $$top_builddir/lib

INCLUDEPATH += $$top_srcdir/plantgl/src/cpp $$BOOST_INCLUDE
LIBS += -L$$top_builddir/plantgl/lib -lAlgo

!contains(CONFIG, static) {
    LIBS += -lTool -lMath -lSceneGraph
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
    LIBS += -L$$BOOST_LIB -lboost_python
    INCLUDEPATH += $$PYTHON_INCLUDE
    LIBS += -L$$PYTHON_LIB -lpython2.7

    QT += concurrent
    LIBS += -lGui

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
