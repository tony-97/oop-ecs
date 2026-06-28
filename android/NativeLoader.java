package com.{APP_COMPANY_NAME}.{APP_PRODUCT_NAME};

public class NativeLoader extends android.app.NativeActivity {
    static {
        // TODO iterate over an array of lib names and load them only if the
        // libraries are compiled as shared libraries .so load the lib names from the makefile template
        // System.loadLibrary("raylib"); // Uncomment if raylib is compiled as a shared library (.so)
        System.loadLibrary("{PROJECT_LIBRARY_NAME}"); 
    }
}