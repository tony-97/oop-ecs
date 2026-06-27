package com.{APP_COMPANY_NAME}.{APP_PRODUCT_NAME};

public class NativeLoader extends android.app.NativeActivity {
    static {
        // System.loadLibrary("raylib"); // Uncomment if raylib is compiled as a shared library (.so)
        System.loadLibrary("{PROJECT_LIBRARY_NAME}"); 
    }
}