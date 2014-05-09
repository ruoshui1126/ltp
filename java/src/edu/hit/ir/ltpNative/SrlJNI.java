package edu.hit.ir.ltpNative;
import java.util.List;

public class SrlJNI {

	static {
		System.loadLibrary("srl_jni");
	}

	public static native int srlCreate(String modelPath);

	public static native int srlSrl(
			List<String> words,
			List<String> tags,
			List<String> ners,
			List<Integer> heads,
			List<String> deprels,
			List<Pair<Integer, List<Pair<String, Pair<Integer, Integer>>>>> srls);

	public static native void srlRelease();
}

