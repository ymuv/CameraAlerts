#pragma once

struct CRunStatus
{
    enum class HomeStatus
    {
        HOME_IN  = (1 << 0),
        HOME_OUT = (1 << 1),
    };
    bool mIsRunMotionAlgo = true;
    bool mIsRunConditions = true;
    HomeStatus mHomeStatus = HomeStatus::HOME_OUT;
};
