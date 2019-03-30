#include <common/log.h>
#include <codec/IVideoEncoder.h>
#include <codec/FFVideoEncoder.h>
#include <codec/IVideoEncoder.h>

const static char * TAG = "TestVideoEncoder";

class VideoEncodeCallback : public IVideoEncodeCallback
{
public:
    VideoEncodeCallback() {}
    ~VideoEncodeCallback() {}

    int OnVideoEncode(uint8_t * data, int size, enFrameType type)
    {
        LOGI(TAG, "packet size: %d, frame type: %d\n", size, type);
        FILE * pf = fopen("1080p.h264", "wb+");
        fwrite(data, size, 1, pf);
        fclose(pf);
        return 0;
    }
};

void TestFFVideoEncoder()
{
    int w = 1920;
    int h = 1080;
    VideoEncodeCallback cb;
    FFVideoEncoder encoder;

    encoder.Open(PIX_YUV420p, H264, w, h, 10, 600000, &cb);
    encoder.Start();

    int yuvsize = w * h * 3 >> 1;
    uint8_t * yuv = new uint8_t[yuvsize];
    FILE * fp = fopen("1080p.yuv", "rb");
    if (fp == NULL) {
        LOGE(TAG, "fopen error\n");
        return;
    }

    fread(yuv, yuvsize, 1, fp);

    encoder.Encode(yuv, yuvsize);

    encoder.Stop();
    encoder.Close();

    delete[] yuv;
}