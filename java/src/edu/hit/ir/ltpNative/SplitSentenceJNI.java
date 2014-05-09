package edu.hit.ir.ltpNative;
import java.util.List;
public class SplitSentenceJNI{
  static{
    System.loadLibrary("split_sentence_jni");
  }
  public static native void splitSentence(String sent,List<String> sents);
}
