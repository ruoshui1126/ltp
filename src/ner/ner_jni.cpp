#include "edu_hit_ir_ltpNative_NerJNI.h"
#include <string>
#include <vector>
#include "ner_dll.h"
#include "utils/strutils.hpp"

static void * ner = NULL;

JNIEXPORT void JNICALL Java_edu_hit_ir_ltpNative_NerJNI_nerCreate
(JNIEnv * env, jclass obj, jstring model_path){
  const char * str = env->GetStringUTFChars( model_path , 0);
  if(!ner){
    ner = ner_create_recognizer(str);
  }
  else{
    ner_release_recognizer(ner);
    ner = ner_create_recognizer(str);
  }
  env->ReleaseStringUTFChars( model_path, str); 
}


JNIEXPORT jint JNICALL Java_edu_hit_ir_ltpNative_NerJNI_nerRecognize
(JNIEnv * env, jclass obj, jobject array_words, jobject array_tags, jobject array_ners){

  jclass array_list = env->GetObjectClass(array_words);

  jmethodID list_add = env->GetMethodID(array_list,"add","(Ljava/lang/Object;)Z");
  jmethodID list_get = env->GetMethodID(array_list,"get","(I)Ljava/lang/Object;");
  jmethodID list_size = env->GetMethodID(array_list,"size","()I");

  std::vector<std::string> words,tags,ners;

  int size_words = env->CallIntMethod(array_words,list_size);

  int size_tags = env->CallIntMethod(array_tags,list_size);

  if(size_words!=size_tags){
    return 0;
  }

  for(int i = 0;i<size_words;i++){
    jobject tmp = env->CallObjectMethod(array_words,list_get,i);
    jstring s = reinterpret_cast<jstring> (tmp);
    const char * st = env->GetStringUTFChars(s,0);
    std::string s_s(st);
    words.push_back(s_s);
    env->ReleaseStringUTFChars( s, st); 
  }

  for(int i = 0;i<size_tags;i++){
    jobject tmp = env->CallObjectMethod(array_tags,list_get,i);
    jstring s = reinterpret_cast<jstring> (tmp);
    const char * st = env->GetStringUTFChars(s,0);
    std::string s_s(st);
    tags.push_back(s_s);
    env->ReleaseStringUTFChars( s, st); 
  }

  int len = ner_recognize(ner,words,tags,ners);

  for(int i = 0;i<len;i++){
    jobject tmp =  ltp::strutils::stringToJstring(env,ners[i].c_str());
    env->CallBooleanMethod(array_ners,list_add,tmp);
  }
  return len;
}

JNIEXPORT void JNICALL Java_edu_hit_ir_ltpNative_NerJNI_nerRelease
(JNIEnv * env, jclass obj){
  ner_release_recognizer(ner);
  ner = NULL;
}


