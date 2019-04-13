#include <common/corelog.hpp>
#include <codec/FFVideoEncoder.h>
//#include <common/log.h>

FFVideoEncoder::FFVideoEncoder()
    : m_bOpen(false)
    , m_bStart(false)
    , m_iPicH(0)
    , m_iPicW(0)
    , m_enInputFmt(PIX_YUV420p)
    , m_enOutputFmt(H264)
    , m_cb(NULL)
{
}

FFVideoEncoder::~FFVideoEncoder()
{
}

int FFVideoEncoder::Open(enPixFormat ifmt, enEncodeType ofmt,
    int picw, int pich, int gopsize, int bitrate,
    IVideoEncodeCallback * cb)
{
    int ret = -1;
    m_iPicW = picw; m_iPicH = pich;
    m_enInputFmt = ifmt; m_enOutputFmt = ofmt;
    m_iBitrate = bitrate;
    m_cb = cb;

    _SelectCodecType(ofmt);

    LOG_F(INFO, "pic:%dx%d, enc:%dx%d, bitrate:%d, gopsize:%d",
        picw, pich, bitrate, gopsize);

    m_pAVCodec = avcodec_find_encoder(m_codecId);
    //m_pAVCodec = avcodec_find_encoder_by_name("libx264");
    if (m_pAVCodec == NULL) {
        LOG_F(ERROR, "avcodec_find_decoder error");
        goto error;
    }

    m_pAVPacket = av_packet_alloc();
    if (m_pAVPacket == NULL) {
		LOG_F(ERROR, "av_packet_alloc error");
        goto error;
    }

    m_pAVFrame = av_frame_alloc();
    if (m_pAVFrame == NULL) {
		LOG_F(ERROR, "av_frame_alloc error");
        goto error;
    }

    m_pAVContext = avcodec_alloc_context3(m_pAVCodec);
    if (m_pAVContext == NULL) {
		LOG_F(ERROR, "avcodec_alloc_context3 error");
        goto error;
    }

    m_pAVContext->profile = FF_PROFILE_H264_BASELINE;
    m_pAVContext->width = m_iPicW;
    m_pAVContext->height = m_iPicH;
    m_pAVContext->pix_fmt = AV_PIX_FMT_YUV420P;
    m_pAVContext->time_base.num = 1;
    m_pAVContext->time_base.den = 24;

    m_pAVContext->gop_size = m_iGopSize;
    //m_pAVContext->keyint_min = m_iGopSize;
    m_pAVContext->max_b_frames = 0;
    //m_pAVContext->qmin = 15;
    //m_pAVContext->qmax = 30;

    m_pAVContext->bit_rate = m_iBitrate;
    m_pAVContext->rc_max_rate = m_iBitrate;
    m_pAVContext->rc_min_rate = m_iBitrate;
    m_pAVContext->bit_rate_tolerance = m_iBitrate;
    m_pAVContext->rc_buffer_size = m_iBitrate;
    m_pAVContext->rc_initial_buffer_occupancy = m_iBitrate * 3 / 4;

    if (avcodec_open2(m_pAVContext, m_pAVCodec, NULL) < 0) {
		LOG_F(ERROR, "avcodec_open2 error");
        goto error;
    }

    m_pAVFrame->format = m_pAVContext->pix_fmt;
    m_pAVFrame->width = m_pAVContext->width;
    m_pAVFrame->height = m_pAVContext->height;

    ret = 0;
    m_bOpen = true;

error:
    if (ret < 0) {
        Close();
    }

    return ret;
}

int FFVideoEncoder::Encode(uint8_t * data, int size)
{
    if (!m_bOpen) {
		LOG_F(ERROR, "cann't encode before open");
        return -1;
    }
    if (!m_bStart) {
		LOG_F(ERROR, "the encoder has stopped");
        return -1;
    }
    int ret = -1;
    //av_frame_make_writable(m_pAVFrame);

    av_image_fill_linesizes(m_pAVFrame->linesize,
        m_pAVContext->pix_fmt,
        m_pAVContext->width);

    ret = av_image_fill_pointers(m_pAVFrame->data,
        m_pAVContext->pix_fmt,
        m_pAVContext->height,
        data,
        m_pAVFrame->linesize);

    if (ret < 0) {
		LOG_F(ERROR, "av_frame_make_writable error");
        return -1;
    }

    int ysize = m_iPicW * m_iPicH;
    int usize = ysize >> 2;

    m_pAVFrame->data[0] = data;
    m_pAVFrame->data[1] = data + ysize;
    m_pAVFrame->data[2] = data + ysize + usize;

    _Encode(m_pAVFrame);
    _Encode(NULL);

    return 0;
}

int FFVideoEncoder::Start(void)
{
    m_bStart = m_bOpen;
    return 0;
}

int FFVideoEncoder::Stop(void)
{
    m_bStart = false;
    return 0;
}

int FFVideoEncoder::Close(void)
{
    if (m_bOpen) {
        m_bOpen = false;
        avcodec_close(m_pAVContext);
    }

    if (m_pAVContext != NULL) {
        avcodec_free_context(&m_pAVContext);
        m_pAVContext = NULL;
    }

    if (m_pAVFrame != NULL) {
        av_frame_free(&m_pAVFrame);
        m_pAVFrame = NULL;
    }

    if (m_pAVPacket != NULL) {
        av_packet_free(&m_pAVPacket);
        m_pAVPacket = NULL;
    }
    m_pAVCodec = NULL;

    return 0;
}

int FFVideoEncoder::_SelectCodecType(enEncodeType ofmt)
{
    switch (ofmt) {
    case H264:
        m_codecId = AV_CODEC_ID_H264;
        break;
    case H265:
        m_codecId = AV_CODEC_ID_H265;
        break;
    default:
        break;
    }

    return 0;
}

int FFVideoEncoder::_Encode(AVFrame * frame)
{
    int ret = avcodec_send_frame(m_pAVContext, frame);
    if (ret < 0) {
		LOG_F(ERROR, "error sending a frame for encoding");
        return -1;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(m_pAVContext, m_pAVPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            continue;
        } else if (ret < 0) {
			LOG_F(ERROR, "error during encoding");
            return -1;
        }

        LOG_F(INFO, "pkt pts: %ld, pkt size: %d", m_pAVPacket->pts, m_pAVPacket->size);
        if (m_cb != NULL) {

            bool bIFrame = m_pAVPacket->flags | AV_PKT_FLAG_KEY;
            enFrameType type = bIFrame ? FRAME_I : FRAME_P;
			LOG_F(INFO, "encode output [%s] size: %d", bIFrame ? "I" : "P", m_pAVPacket->size);
            m_cb->OnVideoEncode(m_pAVPacket->data, m_pAVPacket->size, type);
        }
        av_packet_unref(m_pAVPacket);
    }

    return ret;
}

