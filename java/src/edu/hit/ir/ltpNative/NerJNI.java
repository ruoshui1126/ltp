package edu.hit.ir.ltpNative;
import java.util.List;

public class NerJNI {

	static {
		System.loadLibrary("ner_jni");
	}

	public static native int nerCreate(String modelPath);

	public static native int nerRecognize(List<String> words,
			List<String> postags, List<String> ners);

	public static native void nerRelease();

}

