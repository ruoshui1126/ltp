package edu.hit.ir.ltpNative;
import java.util.List;


public class SegmentJNI {
	static{
		System.loadLibrary("segmentor_jni");
	}
	public static native int segmentorCreate(String modelPath);
	public static native int segmentorCreate(String modelPath,String lexiconPath);
	public static native int segmentorSegment(String sent,List<String> words);
	public static native void segmentorRelease();
}

