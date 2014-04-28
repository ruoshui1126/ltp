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
ADD_LIBRARY(splitsnt_jni SHARED split_sentence_jni.cpp ${splitsnt_source})
set_target_properties (splitsnt_jni PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY
${LIBRARY_OUTPUT_PATH}/jni/
    OUTPUT_NAME split_sentence_jni)
