The build is configured to use xmake and gradle wrapper for android, the internal configuration is somewhat complex to support multiple platforms, the scripts in this directory allow you to run builds without fully learning the whole setup.

### Desktop (Windows & Linux)
> Requirements: [xmake](https://xmake.io/) installed

| platform | command                     |
| -------- | --------------------------- |
| windows  | `build_desk.bat [--debug]`  |
| linux    | `./build_desk.sh [--debug]` |

`--debug`: switches compilation to debug mode, which might be needed for diagnosing certain issues

### Android
> Requirements: [xmake](https://xmake.io/), the [Android SDK & NDK](https://developer.android.com/tools/sdkmanager).

| platform | command                                    |
| -------- | ------------------------------------------ |
| windows  | `build_android.bat [--debug] [--install]`  |
| linux    | `./build_android.sh [--debug] [--install]` |

`--debug`: switches compilation to debug mode, which might be needed for diagnosing certain issues
> [!NOTE] 
> Due to some bugs, native code is not fully built in debug mode in android builds but will include additional debug info.

`--install`: tells gradle to install the resulting `.apk` on an android device connected via adb.