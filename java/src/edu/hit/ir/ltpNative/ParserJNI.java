package edu.hit.ir.ltpNative;
import java.util.List;

public class ParserJNI {

	static {
		System.loadLibrary("parser_jni");
	}

	public static native int parserCreate(String modelPath);

	public static native int parserParse(List<String> words,
			List<String> tags, List<Integer> heads,
			List<String> deprels);

	public static native void parserRelease();
}

