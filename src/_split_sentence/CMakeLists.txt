include_directories (./
    ${utils_DIR}
    $ENV{JAVA_HOME}/include )

if(WIN32)
include_directories ($ENV{JAVA_HOME}/include/win32)
else(WIN32)
include_directories ($ENV{JAVA_HOME}/include/linux)
endif(WIN32)

SET(splitsnt_source
    define.h
    Reader.cpp
    Reader.h
    SentenceIterator.h
    SplitSentence.cpp
    SplitSentence.h)

ADD_LIBRARY(splitsnt ${splitsnt_source})
