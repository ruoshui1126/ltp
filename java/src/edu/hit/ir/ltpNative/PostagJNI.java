package edu.hit.ir.ltpNative;
import java.util.List;


public class PostagJNI {
	static {
		System.loadLibrary("postagger_jni");
	}

	public static native int postaggerCreate(String modelPath);

	public static native int postaggerPostag(List<String> words,
			List<String> tags);
	public static native void postaggerRelease();

}

