macx {
    # Python configuration
    PYTHON_INCLUDE = /usr/include/python2.7
    PYTHON_LIB = /usr/lib/python2.7/config
}

macx | ios | android {
    # Boost configuration
    BOOST_INCLUDE = /usr/local/include
    BOOST_LIB = /usr/local/lib
}

win32 {
    # Python configuration
    PYTHON_INCLUDE = /usr/include/python2.7
    PYTHON_LIB = /usr/lib/python2.7/config

    # Boost configuration
    BOOST_INCLUDE = C:/DevTools/Boost/include/boost-1_63
    BOOST_LIB = C:/DevTools/Boost/lib
}
