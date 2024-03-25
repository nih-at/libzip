
# Author: Declan Moran
# www.silverglint.com 
# Thanks to damaex (https://github.com/damaex), for significant contributions

ANDROID_NDK_ROOT=/home/android/android-ndk-r19c

INSTALL_DIR=install
BUILD_DIR=build
START_DIR=$(pwd)

rm -rf $INSTALL_DIR
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR #"${ANDROID_TARGET_PLATFORM}"

#--------------------------------------------------------------------
build_it()
{
    # builds either a static or shared lib depending on parm passed (ON or OFF)
    want_shared=$1

	cmake -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake \
		-DCMAKE_INSTALL_PREFIX:PATH=$(pwd)/../../${INSTALL_DIR}/${ANDROID_TARGET_PLATFORM} \
		-DANDROID_ABI=${ANDROID_TARGET_PLATFORM} \
		-DLIBZIP_ENABLE_OPENSSL:BOOL=OFF \
		-DLIBZIP_ENABLE_COMMONCRYPTO:BOOL=OFF \
		-DLIBZIP_ENABLE_GNUTLS:BOOL=OFF \
		-DLIBZIP_ENABLE_MBEDTLS:BOOL=OFF \
		-DLIBZIP_ENABLE_OPENSSL:BOOL=OFF \
		-DLIBZIP_ENABLE_WINDOWS_CRYPTO:BOOL=OFF \
		-DLIBZIP_BUILD_TOOLS:BOOL=OFF \
		-DLIBZIP_BUILD_REGRESS:BOOL=OFF \
		-DLIBZIP_BUILD_EXAMPLES:BOOL=OFF \
		-DBUILD_SHARED_LIBS:BOOL=$want_shared \
		-DLIBZIP_BUILD_DOC:BOOL=OFF \
		-DANDROID_TOOLCHAIN=clang  cmake -H.. -B$BUILD_DIR/${ANDROID_TARGET_PLATFORM}
		   	
        #run make with all system threads and install
        cd $BUILD_DIR/${ANDROID_TARGET_PLATFORM}
        make install -j$(nproc --all)
        cd $START_DIR
    }

#--------------------------------------------------------------------
for ANDROID_TARGET_PLATFORM in armeabi-v7a arm64-v8a x86 x86_64
do
	echo "Building libzip for ${ANDROID_TARGET_PLATFORM}" 
	
	build_it ON
	build_it OFF
	
	if [ $? -ne 0 ]; then
		echo "Error executing: cmake"
		exit 1
	fi

	
	if [ $? -ne 0 ]; then
		echo "Error executing make install for platform: ${ANDROID_TARGET_PLATFORM}"
		exit 1
    fi
    
done    
