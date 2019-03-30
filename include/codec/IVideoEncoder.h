#ifndef __IVIDEO_ENCODER_H__
#define __IVIDEO_ENCODER_H__

#include <common/typedefs.h>

enum enFrameType {
    UNKOWN_FRAME = -1,
    FRAME_I,
    FRAME_P,
    FRAME_B
};

class CORE_EXPORT IVideoEncodeCallback {

public:
    virtual ~IVideoEncodeCallback(void) {}
    virtual int OnVideoEncode(uint8_t * data, int size, enFrameType type) = 0;
};

enum enEncodeType {
    H264,
    H265
};
/*
 * I420: YYYYYYYY UU VV    =>YUV420P
 * YV12: YYYYYYYY VV UU    =>YUV420P
 * NV12: YYYYYYYY UVUV     =>YUV420SP
 * NV21: YYYYYYYY VUVU     =>YUV420SP
 * YUYV:
 */
enum enPixFormat {
    PIX_YUV420p = 0,
    PIX_YV12,
    PIX_NV12,
    PIX_NV21,
    PIX_YUYV,
};

class CORE_EXPORT IVideoEncoder {
public:
    virtual ~IVideoEncoder() {}

    virtual int Open(enPixFormat ifmt, enEncodeType ofmt,
        int picw, int pich, int gopsize, int bitrate,
        IVideoEncodeCallback * cb) = 0;

    virtual int Encode(uint8_t * data, int size) = 0;

    virtual int Start(void) = 0;

    virtual int Stop(void) = 0;

    virtual int Close(void) = 0;

};

#endif // !__IVIDEO_ENCODER_H__

