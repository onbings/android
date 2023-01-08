REM C:\pro\android\VcpkgBuildAndInstallBofstd.bat bofstd x86_64 32 x64

cls
set CPP_PRJ=%1
echo BUILD '%CPP_PRJ%'
set pwd=%cd%
set ANDROID_NDK_HOME=C:\Android\Sdk\ndk\25.1.8937393
set VCPKG_ROOT=C:/pro/vcpkg
set CPP_ANDROID_PORT_FILE=C:/pro/github/onbings-vcpkg-registry/ports
set CPP_ANDROID_ROOT=C:\Android
REM x86_64
set CPP_ANDROID_ABI=%2
REM 30
set CPP_ANDROID_API=%3

REM x64
set CPP_ANDROID_TRIPLET=%4-android-%CPP_ANDROID_API%
mkdir %CPP_ANDROID_REPO%\%CPP_PRJ%

%VCPKG_ROOT%/vcpkg install %CPP_PRJ%:%CPP_ANDROID_TRIPLET% --overlay-ports=C:/pro/github/onbings-vcpkg-registry/ports/

cd %pwd%
