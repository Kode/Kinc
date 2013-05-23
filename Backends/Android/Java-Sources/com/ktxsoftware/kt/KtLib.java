package com.ktxsoftware.kt;

public class KtLib {
    static {
        System.loadLibrary("Kt");
    }

    public static native void init(int width, int height, String apkPath);
    public static native void step();
}