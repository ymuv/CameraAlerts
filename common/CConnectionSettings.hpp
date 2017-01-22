#pragma once

struct CConnectionSettings
{
    unsigned short mTCPPort; //for server; if 0 - server did't run
    unsigned short mUDPPort; //for server; if 0 - server did't run
    int mTimeOut;

    bool mIsCheckHash = true;
};
