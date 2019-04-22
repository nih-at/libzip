
ANDROID_NDK_ROOT=/home/android/android-ndk-r19c

INSTALL_DIR=install
BUILD_DIR=build
START_DIR=$(pwd)

rm -rf $INSTALL_DIR
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR #"${ANDROID_TARGET_PLATFORM}"

for ANDROID_TARGET_PLATFORM in armeabi-v7a arm64-v8a x86 x86_64
do
	echo "Building libzip for ${ANDROID_TARGET_PLATFORM}" 
	


	
	#run configuration for target platform
	cmake -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake \
		-DCMAKE_INSTALL_PREFIX:PATH=../../${INSTALL_DIR}/${ANDROID_TARGET_PLATFORM} \
		-DANDROID_ABI=${ANDROID_TARGET_PLATFORM} \
		-DENABLE_OPENSSL:BOOL=OFF \
		-DENABLE_COMMONCRYPTO:BOOL=OFF \
		-DENABLE_GNUTLS:BOOL=OFF \
		-DENABLE_MBEDTLS:BOOL=OFF \
		-DENABLE_OPENSSL:BOOL=OFF \
		-DENABLE_WINDOWS_CRYPTO:BOOL=OFF \
		-DBUILD_TOOLS:BOOL=OFF \
		-DBUILD_REGRESS:BOOL=OFF \
		-DBUILD_EXAMPLES:BOOL=OFF \
		-DBUILD_SHARED_LIBS:BOOL=ON \
		-DBUILD_DOC:BOOL=OFF \
		-DANDROID_TOOLCHAIN=clang  cmake -H.. -B$BUILD_DIR/${ANDROID_TARGET_PLATFORM}
	
	if [ $? -ne 0 ]; then
		echo "Error executing: cmake"
		exit 1
	fi
	
	#run make with all system threads and install to output dir
	cd $BUILD_DIR/${ANDROID_TARGET_PLATFORM}
	make install -j$(nproc --all)
	
	
	cd $START_DIR
	
	if [ $? -ne 0 ]; then
		echo "Error executing make install for platform: ${ANDROID_TARGET_PLATFORM}"
		exit 1
    fi
    
done    
