package com.ktxsoftware.kore;

public class KoreLib {
    static {
        System.loadLibrary("Kore");
    }

    public static native void init(int width, int height, String apkPath);
    public static native void step();
}
