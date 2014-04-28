#include "edu_hit_ir_ltpNative_SrlJNI.h"
#include "SRL_DLL.h"
#include "strutils.hpp"
#include <vector>
#include <string>
#include <iostream>

JNIEXPORT void JNICALL Java_edu_hit_ir_ltpNative_SrlJNI_srlCreate
(JNIEnv * env, jclass obj, jstring model_path){
  const char * str = env->GetStringUTFChars( model_path , 0);
  std::string path(str);
  SRL_LoadResource(path);
  env->ReleaseStringUTFChars( model_path, str); 
}

JNIEXPORT jint JNICALL Java_edu_hit_ir_ltpNative_SrlJNI_srlSrl
(JNIEnv * env, jclass obj, jobject array_words, jobject array_tags, jobject array_ners, jobject array_heads, jobject array_deprels, jobject srl_result){
	
  jclass array_list = env->GetObjectClass(array_words);
  jmethodID list_construct = env->GetMethodID(array_list,"<init>","()V");
  jmethodID list_add = env->GetMethodID(array_list,"add","(Ljava/lang/Object;)Z");
  jmethodID list_get = env->GetMethodID(array_list,"get","(I)Ljava/lang/Object;");
  jmethodID list_size = env->GetMethodID(array_list,"size","()I");

  jclass integer = env->FindClass("java/lang/Integer");
  jmethodID integer_construct =env->GetMethodID(integer,"<init>","(I)V");
  jmethodID integer_int =env->GetMethodID(integer,"intValue","()I");

  jclass pair = env->FindClass("Pair");
  jmethodID pair_construct = env->GetMethodID(pair,"<init>","(Ljava/lang/Object;Ljava/lang/Object;)V");

  std::vector<std::string> words,tags,ners,deprels;
  std::vector<int> heads;
  std::vector<std::pair<int,std::string> > parsers;
  std::vector< std::pair< int, std::vector< std::pair<std::string, std::pair< int, int > > > > > srls;
	
  int size_words = env->CallIntMethod(array_words,list_size);
  for(int i = 0;i<size_words;i++){
    jobject tmp = env->CallObjectMethod(array_words,list_get,i);
    jstring s = reinterpret_cast<jstring> (tmp);
    const char * st = env->GetStringUTFChars(s,0);
    std::string s_s(st);
    words.push_back(s_s);
    env->ReleaseStringUTFChars( s, st); 
  }

  int size_tags = env->CallIntMethod(array_tags,list_size);
  for(int i = 0;i<size_tags;i++){
    jobject tmp = env->CallObjectMethod(array_tags,list_get,i);
    jstring s = reinterpret_cast<jstring> (tmp);
    const char * st = env->GetStringUTFChars(s,0);
    std::string s_s(st);
    tags.push_back(s_s);
    env->ReleaseStringUTFChars( s, st); 
  }

  int size_ners = env->CallIntMethod(array_ners,list_size);
  for(int i = 0;i<size_ners;i++){
    jobject tmp = env->CallObjectMethod(array_ners,list_get,i);
    jstring s = reinterpret_cast<jstring> (tmp);
    const char * st = env->GetStringUTFChars(s,0);
    std::string s_s(st);
    ners.push_back(s_s);
    env->ReleaseStringUTFChars( s, st); 
  }

  int size_heads = env->CallIntMethod(array_heads,list_size);
  for(int i = 0;i<size_heads;i++){
    jobject tmp = env->CallObjectMethod(array_heads,list_get,i);
    int digit = env->CallIntMethod(tmp,integer_int);
    heads.push_back(digit);
  }

  int size_deprels = env->CallIntMethod(array_deprels,list_size);
  for(int i = 0;i<size_deprels;i++){
    jobject tmp = env->CallObjectMethod(array_deprels,list_get,i);
    jstring s = reinterpret_cast<jstring> (tmp);
    const char * st = env->GetStringUTFChars(s,0);
    std::string s_s(st);
    deprels.push_back(s_s);
    env->ReleaseStringUTFChars( s, st); 
  }

  for(int i = 0;i<size_heads;i++){
    parsers.push_back(make_pair(heads.at(i),deprels.at(i)));
  }

  int len = DoSRL(words,tags,ners,parsers,srls);

  if(len<0)
    return -1;

  for(int i = 0;i<srls.size();++i) {
    jobject trigger = env->NewObject(integer,integer_construct,srls[i].first);
    jobject args = env->NewObject(array_list,list_construct);

    for(int j = 0;j<srls[i].second.size();++j) {
      jobject start = env->NewObject(integer,integer_construct,srls[i].second[j].second.first);
      jobject end = env->NewObject(integer,integer_construct,srls[i].second[j].second.second);

      jobject bound = env->NewObject(pair,pair_construct,start,end);

      jobject rels =  ltp::strutils::stringToJstring(env,srls[i].second[j].first.c_str());
      jobject inner = env->NewObject(pair,pair_construct,rels,bound);
      env->CallBooleanMethod(args,list_add,inner);
    }

    jobject outer = env->NewObject(pair,pair_construct,trigger,args);
    env->CallBooleanMethod(srl_result,list_add,outer);
  }

  return srls.size();
}

JNIEXPORT void JNICALL Java_edu_hit_ir_ltpNative_SrlJNI_srlRelease
(JNIEnv * env, jclass obj){
	SRL_ReleaseResource();
}

