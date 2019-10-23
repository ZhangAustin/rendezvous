#include "ImplementationFactory.h"

#include <iostream>

#ifdef NO_CUDA
#include "detection/DarknetDetector.h"
#include "dewarping/CpuFisheyeDewarper.h"
#include "dewarping/CpuDarknetFisheyeDewarper.h"
#include "stream/input/CameraReader.h"
#include "stream/input/ImageFileReader.h"
#include "utils/alloc/HeapObjectFactory.h"
#include "utils/images/ImageConverter.h"
#include "utils/threads/sync/NopSynchronizer.h"
#else
#include "cuda_runtime.h"
#include "detection/cuda/CudaDarknetDetector.h"
#include "dewarping/cuda/CudaFisheyeDewarper.h"
#include "dewarping/cuda/CudaDarknetFisheyeDewarper.h"
#include "stream/input/cuda/CudaCameraReader.h"
#include "stream/input/cuda/CudaImageFileReader.h"
#include "utils/alloc/cuda/DeviceCudaObjectFactory.h"
#include "utils/alloc/cuda/ManagedMemoryCudaObjectFactory.h"
#include "utils/alloc/cuda/ZeroCopyCudaObjectFactory.h"
#include "utils/images/cuda/CudaImageConverter.h"
#include "utils/threads/sync/CudaSynchronizer.h"

namespace
{
    cudaStream_t stream;
    cudaStream_t detectionStream;
}

#endif

ImplementationFactory::ImplementationFactory(bool useZeroCopyIfSupported)
    : useZeroCopyIfSupported_(useZeroCopyIfSupported)
    , isZeroCopySupported_(false)
{
    std::string message;
#ifdef NO_CUDA
    message = "Application was compiled without CUDA, calculations will be executed on the CPU.";
#else
    const int deviceNumber = 0;
    cudaDeviceProp deviceProp;
    cudaGetDeviceProperties(&deviceProp, deviceNumber);
    isZeroCopySupported_ = deviceProp.integrated;
    message = isZeroCopySupported_ ? "Graphic card supports zero copy" : "Graphic card doesn't support zero copy";

    cudaStreamCreate(&stream);
    cudaStreamCreate(&detectionStream);
#endif
    std::cout << message << std::endl;
}

ImplementationFactory::~ImplementationFactory()
{
#ifndef NO_CUDA
    cudaStreamDestroy(stream);
    cudaStreamDestroy(detectionStream);    
#endif
}

std::unique_ptr<IDetector> ImplementationFactory::getDetector(const std::string& configFile, const std::string& weightsFile, 
                                                              const std::string& metaFile)
{
    std::unique_ptr<IDetector> detector = nullptr;

#ifdef NO_CUDA
    detector = std::make_unique<DarknetDetector>(configFile, weightsFile, metaFile);
#else
    detector = std::make_unique<CudaDarknetDetector>(configFile, weightsFile, metaFile);
#endif

    return detector;
}

std::unique_ptr<IObjectFactory> ImplementationFactory::getObjectFactory()
{
    std::unique_ptr<IObjectFactory> objectFactory = nullptr;

#ifdef NO_CUDA
    objectFactory = std::make_unique<HeapObjectFactory>();
#else
    if (useZeroCopyIfSupported_ && isZeroCopySupported_)
    {
        objectFactory = std::make_unique<ZeroCopyCudaObjectFactory>();
    }
    else
    {
        objectFactory = std::make_unique<ManagedMemoryCudaObjectFactory>(stream);
    }     
#endif

    return objectFactory;
}

std::unique_ptr<IObjectFactory> ImplementationFactory::getDetectionObjectFactory()
{
     std::unique_ptr<IObjectFactory> objectFactory = nullptr;

#ifdef NO_CUDA
    objectFactory = std::make_unique<HeapObjectFactory>();
#else
    if (useZeroCopyIfSupported_ && isZeroCopySupported_)
    {
        objectFactory = std::make_unique<ZeroCopyCudaObjectFactory>();
    }
    else
    {
        objectFactory = std::make_unique<DeviceCudaObjectFactory>();
    }     
#endif

    return objectFactory;
}

std::unique_ptr<IFisheyeDewarper> ImplementationFactory::getFisheyeDewarper()
{
    std::unique_ptr<IFisheyeDewarper> fisheyeDewarper = nullptr;

#ifdef NO_CUDA
    fisheyeDewarper = std::make_unique<CpuFisheyeDewarper>();
#else
    fisheyeDewarper = std::make_unique<CudaFisheyeDewarper>(stream);
#endif

    return fisheyeDewarper;
}

std::unique_ptr<IDetectionFisheyeDewarper> ImplementationFactory::getDetectionFisheyeDewarper(float aspectRatio)
{
    std::unique_ptr<IDetectionFisheyeDewarper> normalizedFisheyeDewarper = nullptr;

#ifdef NO_CUDA
    normalizedFisheyeDewarper = std::make_unique<CpuDarknetFisheyeDewarper>(aspectRatio);
#else
    normalizedFisheyeDewarper = std::make_unique<CudaDarknetFisheyeDewarper>(detectionStream, aspectRatio);
#endif

    return normalizedFisheyeDewarper;
}

std::unique_ptr<ISynchronizer> ImplementationFactory::getSynchronizer()
{
    std::unique_ptr<ISynchronizer> synchronizer = nullptr;

#ifdef NO_CUDA
    synchronizer = std::make_unique<NopSynchronizer>();
#else
    synchronizer = std::make_unique<CudaSynchronizer>(stream);
#endif

    return synchronizer;
}

std::unique_ptr<ISynchronizer> ImplementationFactory::getDetectionSynchronizer()
{
    std::unique_ptr<ISynchronizer> synchronizer = nullptr;

#ifdef NO_CUDA
    synchronizer = std::make_unique<NopSynchronizer>();
#else
    synchronizer = std::make_unique<CudaSynchronizer>(detectionStream);
#endif

    return synchronizer;
}

std::unique_ptr<IImageConverter> ImplementationFactory::getImageConverter()
{
    std::unique_ptr<IImageConverter> imageConverter = nullptr;

#ifdef NO_CUDA
    imageConverter = std::make_unique<ImageConverter>();
#else
    imageConverter = std::make_unique<CudaImageConverter>(stream);
#endif

    return imageConverter;
}

std::unique_ptr<IVideoInput> ImplementationFactory::getFileImageReader(const std::string& imageFilePath, ImageFormat format)
{
    std::unique_ptr<IVideoInput> fileImageReader = nullptr;

#ifdef NO_CUDA
    fileImageReader = std::make_unique<ImageFileReader>(imageFilePath, format);
#else
    fileImageReader = std::make_unique<CudaImageFileReader>(imageFilePath, format);
#endif

    return fileImageReader;
}

std::unique_ptr<IVideoInput> ImplementationFactory::getCameraReader(const VideoConfig& videoConfig)
{
    std::unique_ptr<IVideoInput> cameraReader = nullptr;

#ifdef NO_CUDA
    cameraReader = std::make_unique<CameraReader>(videoConfig, 2);
#else
    cameraReader = std::make_unique<CudaCameraReader>(videoConfig);
#endif

    return cameraReader;
}