plugins {
    id("com.android.application")
}

android {
    namespace = "ch.erglim.sheepdog"
    compileSdk = 35
    ndkVersion = "25.1.8937393"

    defaultConfig {
        applicationId = "ch.erglim.sheepdog"
        minSdk = 26
        targetSdk = 35
        versionCode = 1
        versionName = "0.1"

        ndk {
            // arm64 for devices, x86_64 for the emulator
            abiFilters += listOf("arm64-v8a", "x86_64")
        }
        externalNativeBuild {
            cmake {
                arguments += "-DANDROID_STL=c++_static"
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    sourceSets {
        getByName("main") {
            // SDL's Java glue (SDLActivity etc.) is used straight from the submodule
            java.srcDir("../external/SDL/android-project/app/src/main/java")
            // game assets become the APK's asset root (see src/platform/assets.cpp)
            assets.srcDir("../assets")
        }
    }
    externalNativeBuild {
        cmake {
            path = file("../CMakeLists.txt")
            version = "3.22.1"
        }
    }
}
