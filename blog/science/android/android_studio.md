# Android Studio configure

## Android Studio Version

Android Studio Hedgehog | 2023.1.1 Patch 2
Build #AI-231.9392.1.2311.11330709, built on January 19, 2024
Runtime version: 17.0.7+0-b2043.56-10550314 amd64
VM: OpenJDK 64-Bit Server VM by JetBrains s.r.o.
Windows 10.0
GC: G1 Young Generation, G1 Old Generation
Memory: 2048M
Cores: 4
Registry:
    external.system.auto.import.disabled=true
    debugger.new.tool.window.layout=true
    ide.text.editor.with.preview.show.floating.toolbar=false
    ide.experimental.ui=true

## 环境配置

### gradle

- Look into *project-dir/gradle/wrapper/gradle-wrapper.properties*, line like *distributionUrl=https\://services.gradle.org/distributions/gradle-8.2-bin.zip*. Gradle version is gradle-8.2. Change it's name to gradle-8.2-all.zip.
- Download and stop it.
- Download gradle-8.2-all.zip from https://mirrors.cloud.tencent.com/gradle/.
- CD into C:\Users\user-name\.gradle\wrapper\dists\gradle-8.2-all\6mxqtxovn2faat1idb7p6lxsa. Clean this directory. Copy gradle-8.2-all.zip into this directory. Restart download process.

### settings.gradle.kts

- Open *project-dir/settings.gradle[.kts]*
- Change its content like followings:

```bash

pluginManagement {
    repositories {
        maven { setUrl("https://maven.aliyun.com/repository/central") }
        maven { setUrl("https://maven.aliyun.com/repository/jcenter") }
        maven { setUrl("https://maven.aliyun.com/repository/google") }
        maven { setUrl("https://maven.aliyun.com/repository/gradle-plugin") }
        maven { setUrl("https://maven.aliyun.com/repository/public") }
        maven { setUrl("https://jitpack.io") }
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        maven { setUrl("https://maven.aliyun.com/repository/central") }
        maven { setUrl("https://maven.aliyun.com/repository/jcenter") }
        maven { setUrl("https://maven.aliyun.com/repository/google") }
        maven { setUrl("https://maven.aliyun.com/repository/gradle-plugin") }
        maven { setUrl("https://maven.aliyun.com/repository/public") }
        maven { setUrl("https://jitpack.io") }
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}

rootProject.name = "myapp"
include(":app")

```

- Restart downloading process.