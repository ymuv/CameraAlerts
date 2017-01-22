#pragma once

struct ImageType
{
    enum EType {
        IMAGE_GRAY_RESIZED = 0,
        IMAGE_GRAY_FULL = 1,
        IMAGE_RGB_FULL = 2,
        IMAGE_RGB_RESIZED = 3,

        IMAGE_RGB_FULL_WITH_TEXT = 4,

        IMAGE_GRAY_NET_RESIZE = 5,

        MAX_VALUE
    };
};

