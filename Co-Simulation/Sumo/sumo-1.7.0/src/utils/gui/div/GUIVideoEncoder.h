/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    GUIVideoEncoder.h
/// @author  Michael Behrisch
/// @date    Dec 2015
///
// A simple video encoder from RGBA pics to anything ffmpeg can handle.
// Tested with h264 only.
// Inspired by Lei Xiaohua, Philip Schneider and Fabrice Bellard, see
// https://github.com/leixiaohua1020/simplest_ffmpeg_video_encoder and
// https://github.com/codefromabove/FFmpegRGBAToYUV
/****************************************************************************/
#pragma once
#include <config.h>

#include <stdio.h>
#include <iostream>
#include <stdexcept>

#define __STDC_CONSTANT_MACROS

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4244) // do not warn about integer conversions
#endif
#if __GNUC__ > 3
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#endif
extern "C"
{
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#if __GNUC__ > 3
#pragma GCC diagnostic pop
#endif

#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
* @class GUIVideoEncoder
* @brief A simple video encoder from RGBA pics to anything ffmpeg can handle.
*/
class GUIVideoEncoder {
public:
    GUIVideoEncoder(const char* const out_file, const int width, const int height, double frameDelay) {
        av_register_all();
        avformat_alloc_output_context2(&myFormatContext, NULL, NULL, out_file);
        if (myFormatContext == nullptr) {
            throw ProcessError("Unknown format!");
        }

        // @todo maybe warn about default and invalid framerates
        int framerate = 25;
        if (frameDelay > 0.) {
            framerate = (int)(1000. / frameDelay);
            if (framerate <= 0) {
                framerate = 1;
            }
        }
        AVStream* const video_st = avformat_new_stream(myFormatContext, 0);
        video_st->time_base.num = 1;
        video_st->time_base.den = framerate;

        const AVCodec* const codec = avcodec_find_encoder(myFormatContext->oformat->video_codec);
        if (codec == nullptr) {
            throw ProcessError("Unknown codec!");
        }
        //Param that must set
        myCodecCtx = avcodec_alloc_context3(codec);
        if (myCodecCtx == nullptr) {
            throw ProcessError("Could not allocate video codec context!");
        }
        //pmyCodecCtx->codec_id =AV_CODEC_ID_HEVC;
        //pmyCodecCtx->codec_id = pFormatCtx->oformat->video_codec;
        //pmyCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
        myCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
        // @todo maybe warn about one missing line for odd width or height
        myCodecCtx->width = (width / 2) * 2;
        myCodecCtx->height = (height / 2) * 2;
        myCodecCtx->time_base.num = 1;
        myCodecCtx->time_base.den = framerate;
        myCodecCtx->framerate.num = framerate;
        myCodecCtx->framerate.den = 1;
        myCodecCtx->bit_rate = 4000000; // example has 400000
        myCodecCtx->gop_size = 10; // example has 10
        //H264
        //pmyCodecCtx->me_range = 16;
        //pmyCodecCtx->max_qdiff = 4;
        //pmyCodecCtx->qcompress = 0.6;
        //myCodecCtx->qmin = 10; // example does not set this
        //myCodecCtx->qmax = 51; // example does not set this
        myCodecCtx->max_b_frames = 1; // example has 1

        // Set codec specific options
        //H.264
        if (myCodecCtx->codec_id == AV_CODEC_ID_H264) {
            av_opt_set(myCodecCtx->priv_data, "preset", "slow", 0);
            //av_opt_set(myCodecCtx->priv_data, "tune", "zerolatency", 0);
            //av_opt_set(myCodecCtx->priv_data, "profile", "main", 0);
        }
        //H.265
        if (myCodecCtx->codec_id == AV_CODEC_ID_HEVC) {
            av_opt_set(myCodecCtx->priv_data, "preset", "ultrafast", 0);
            av_opt_set(myCodecCtx->priv_data, "tune", "zero-latency", 0);
        }
        if (avcodec_open2(myCodecCtx, codec, nullptr) < 0) {
            throw ProcessError("Could not open codec!");
        }

        myFrame = av_frame_alloc();
        if (myFrame == nullptr) {
            throw ProcessError("Could not allocate video frame!");
        }
        myFrame->format = myCodecCtx->pix_fmt;
        myFrame->width  = myCodecCtx->width;
        myFrame->height = myCodecCtx->height;
        if (av_frame_get_buffer(myFrame, 32) < 0) {
            throw ProcessError("Could not allocate the video frame data!");
        }
        mySwsContext = sws_getContext(myCodecCtx->width, myCodecCtx->height, AV_PIX_FMT_RGBA,
                                      myCodecCtx->width, myCodecCtx->height, AV_PIX_FMT_YUV420P,
                                      0, 0, 0, 0);
        //Open output URL
        if (avio_open(&myFormatContext->pb, out_file, AVIO_FLAG_WRITE) < 0) {
            throw ProcessError("Failed to open output file!");
        }

        //Write File Header
        if (avformat_write_header(myFormatContext, nullptr) < 0) {
            throw ProcessError("Failed to write file header!");
        }
        myFrameIndex = 0;
        myPkt = av_packet_alloc();
        if (myPkt == nullptr) {
            throw ProcessError("Could not allocate video packet!");
        }
    }

    ~GUIVideoEncoder() {
        int ret = 1;
        if (!(myCodecCtx->codec->capabilities & AV_CODEC_CAP_DELAY)) {
            ret = 0;
        }
        if (avcodec_send_frame(myCodecCtx, nullptr) < 0) {
            WRITE_WARNING("Error sending final frame!");
            ret = -1;
        }
        while (ret >= 0) {
            ret = avcodec_receive_packet(myCodecCtx, myPkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                WRITE_WARNING("Error during final encoding step!");
                break;
            }
            ret = av_write_frame(myFormatContext, myPkt);
            av_packet_unref(myPkt);
        }

        //Write file trailer
        av_write_trailer(myFormatContext);
        avio_closep(&myFormatContext->pb);

        //Clean
        avcodec_free_context(&myCodecCtx);
        av_frame_free(&myFrame);
        av_packet_free(&myPkt);
        avformat_free_context(myFormatContext);
    }

    void writeFrame(uint8_t* buffer) {
        if (av_frame_make_writable(myFrame) < 0) {
            throw ProcessError();
        }
        uint8_t* inData[1] = { buffer }; // RGBA32 has one plane
        int inLinesize[1] = { 4 * myCodecCtx->width }; // RGBA stride
        sws_scale(mySwsContext, inData, inLinesize, 0, myCodecCtx->height,
                  myFrame->data, myFrame->linesize);
        myFrame->pts = myFrameIndex;
        int r = avcodec_send_frame(myCodecCtx, myFrame);
        if (r < 0) {
            char errbuf[64];
            av_strerror(r, errbuf, 64);
            throw ProcessError("Error sending frame for encoding!");
        }
        int ret = 0;
        while (ret >= 0) {
            ret = avcodec_receive_packet(myCodecCtx, myPkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                throw ProcessError("Error during encoding!");
            }
            /* rescale output packet timestamp values from codec to stream timebase */
            av_packet_rescale_ts(myPkt, myCodecCtx->time_base, myFormatContext->streams[0]->time_base);
            myPkt->stream_index = 0;
            ret = av_write_frame(myFormatContext, myPkt);
            av_packet_unref(myPkt);
        }
        myFrameIndex++;
    }

private:
    AVFormatContext* myFormatContext;
    SwsContext* mySwsContext;
    AVCodecContext* myCodecCtx;
    AVFrame* myFrame;
    AVPacket* myPkt;
    int myFrameIndex;

};
