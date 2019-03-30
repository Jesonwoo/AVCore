#ifndef __FF_VIDEO_ENCODER_H__
#define __FF_VIDEO_ENCODER_H__
#include <codec/IVideoEncoder.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>

#ifdef __cplusplus
}
#endif

class CORE_EXPORT FFVideoEncoder : public IVideoEncoder {
public:
    FFVideoEncoder();
    ~FFVideoEncoder();

    virtual int Open(enPixFormat ifmt, enEncodeType ofmt,
        int picw, int pich, int gopsize, int bitrate,
        IVideoEncodeCallback * cb);

    virtual int Encode(uint8_t * data, int size);

    virtual int Start(void);

    virtual int Stop(void);

    virtual int Close(void);

private:
    int _SelectCodecType(enEncodeType ofmt);
    int _Encode(AVFrame * frame);

private:
    bool  m_bOpen;
    bool  m_bStart;
    int   m_iPicH; 
    int   m_iPicW;
    int   m_iBitrate;
    int   m_iGopSize;

    AVCodecID              m_codecId;
    AVPacket *             m_pAVPacket;
    AVFrame *              m_pAVFrame;
    AVCodec *              m_pAVCodec;
    AVCodecContext *       m_pAVContext;

    enPixFormat            m_enInputFmt;
    enEncodeType           m_enOutputFmt;
    IVideoEncodeCallback * m_cb;
};

#endif // !__FF_VIDEO_ENCODER_H__
