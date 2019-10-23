#ifndef DISPLAY_IMAGE_BUILDER_H
#define DISPLAY_IMAGE_BUILDER_H

#include <vector>

#include "utils/models/Dim2.h"
#include "utils/images/Images.h"

class DisplayImageBuilder
{
public:

    DisplayImageBuilder(const Dim2<int>& displayDimention);

    Dim2<int> getVirtualCameraDim(int virtualCameraCount);
    Dim2<int> getMaxVirtualCameraDim();
    void createDisplayImage(const std::vector<Image>& vcImages, const Image& outDisplayImage);
    void setDisplayImageColor(const Image& displayImage);
    void clearVirtualCamerasOnDisplayImage(const Image& displayImage);

private:

    void fillImage(int offset, const Dim2<int>& vcDim, const Dim2<int>& displayDim, const RGB* vcData, RGB* displayData);
    void fillImage(int offset, Dim2<int> vcDim, Dim2<int> displayDim, const UYVY* vcData, UYVY* displayData);
    void fillImage(int offset, Dim2<int> vcDim, Dim2<int> displayDim, const YUYV* vcData, YUYV* displayData);

    Dim2<int> displayDimention_;
    Dim2<int> maxVirtualCameraDim_;

};

#endif //!DISPLAY_IMAGE_BUILDER_H