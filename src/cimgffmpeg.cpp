/*

    pHash, the open source perceptual hash library
    Copyright (C) 2009 Aetilius, Inc.
    All rights reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Evan Klinger - eklinger@phash.org
    D Grant Starkweather - dstarkweather@phash.org

*/

#include "cimgffmpeg.h"

#include <climits>

using cimg_library::CImg;
using cimg_library::CImgList;

namespace {

int find_video_stream(AVFormatContext *pFormatCtx) {
    for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type ==
            AVMEDIA_TYPE_VIDEO) {
            return (int)i;
        }
    }
    return -1;
}

// Open the video stream named in st_info: locate the stream, find a decoder,
// allocate a codec context, copy parameters into it, and open the decoder.
// Returns 0 on success; the caller must call vfinfo_close on failure.
int open_video(VFInfo *st_info) {
    av_log_set_level(AV_LOG_QUIET);

    if (avformat_open_input(&st_info->pFormatCtx, st_info->filename, NULL,
                            NULL) != 0)
        return -1;

    if (avformat_find_stream_info(st_info->pFormatCtx, NULL) < 0) return -1;

    st_info->videoStream = find_video_stream(st_info->pFormatCtx);
    if (st_info->videoStream == -1) return -1;

    AVStream *stream = st_info->pFormatCtx->streams[st_info->videoStream];

    st_info->pCodec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (st_info->pCodec == NULL) return -1;

    st_info->pCodecCtx = avcodec_alloc_context3(st_info->pCodec);
    if (st_info->pCodecCtx == NULL) return -1;

    if (avcodec_parameters_to_context(st_info->pCodecCtx, stream->codecpar) <
        0)
        return -1;

    if (avcodec_open2(st_info->pCodecCtx, st_info->pCodec, NULL) < 0)
        return -1;

    if (st_info->width <= 0) st_info->width = st_info->pCodecCtx->width;
    if (st_info->height <= 0) st_info->height = st_info->pCodecCtx->height;

    return 0;
}

void push_converted(VFInfo *st_info, AVFrame *pFrame, AVFrame *pConvertedFrame,
                    SwsContext *sws, int channels,
                    CImgList<uint8_t> *pFrameList) {
    sws_scale(sws, pFrame->data, pFrame->linesize, 0,
              st_info->pCodecCtx->height, pConvertedFrame->data,
              pConvertedFrame->linesize);
    CImg<uint8_t> next_image(*pConvertedFrame->data, channels, st_info->width,
                             st_info->height, 1, true);
    next_image.permute_axes("yzcx");
    pFrameList->push_back(next_image);
}

// Pull frames from the open st_info until either nb_retrieval frames have been
// pushed into pFrameList, hi_index is exceeded, or EOF. Pass UINT_MAX for
// hi_index to disable the upper bound.
int decode_into_list(VFInfo *st_info, CImgList<uint8_t> *pFrameList,
                     unsigned int hi_index) {
    AVPixelFormat ffmpeg_pixfmt =
        (st_info->pixelformat == 0) ? AV_PIX_FMT_GRAY8 : AV_PIX_FMT_RGB24;
    int channels = (ffmpeg_pixfmt == AV_PIX_FMT_GRAY8) ? 1 : 3;

    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pConvertedFrame = av_frame_alloc();
    AVPacket *packet = av_packet_alloc();
    SwsContext *sws = NULL;
    uint8_t *buffer = NULL;
    int size = 0;
    int rc = -1;
    bool flushed = false;

    if (!pFrame || !pConvertedFrame || !packet) goto cleanup;

    {
        int numBytes = av_image_get_buffer_size(ffmpeg_pixfmt, st_info->width,
                                                st_info->height, 1);
        if (numBytes <= 0) goto cleanup;
        buffer = (uint8_t *)av_malloc((size_t)numBytes);
        if (buffer == NULL) goto cleanup;

        if (av_image_fill_arrays(pConvertedFrame->data,
                                 pConvertedFrame->linesize, buffer,
                                 ffmpeg_pixfmt, st_info->width,
                                 st_info->height, 1) < 0)
            goto cleanup;
    }

    sws = sws_getContext(st_info->pCodecCtx->width,
                         st_info->pCodecCtx->height,
                         st_info->pCodecCtx->pix_fmt, st_info->width,
                         st_info->height, ffmpeg_pixfmt, SWS_BICUBIC, NULL,
                         NULL, NULL);
    if (sws == NULL) goto cleanup;

    while (size < st_info->nb_retrieval && st_info->current_index <= hi_index) {
        int read_rc = av_read_frame(st_info->pFormatCtx, packet);
        if (read_rc < 0) {
            // No more packets; trigger flush mode.
            avcodec_send_packet(st_info->pCodecCtx, NULL);
            flushed = true;
        } else if (packet->stream_index != st_info->videoStream) {
            av_packet_unref(packet);
            continue;
        } else {
            int send_rc = avcodec_send_packet(st_info->pCodecCtx, packet);
            av_packet_unref(packet);
            if (send_rc < 0 && send_rc != AVERROR(EAGAIN)) {
                // Decoder error.
                goto cleanup;
            }
        }

        while (size < st_info->nb_retrieval &&
               st_info->current_index <= hi_index) {
            int recv_rc = avcodec_receive_frame(st_info->pCodecCtx, pFrame);
            if (recv_rc == AVERROR(EAGAIN)) break;
            if (recv_rc == AVERROR_EOF) {
                rc = size;
                goto cleanup;
            }
            if (recv_rc < 0) goto cleanup;

            if (st_info->current_index == st_info->next_index) {
                st_info->next_index += st_info->step;
                push_converted(st_info, pFrame, pConvertedFrame, sws, channels,
                               pFrameList);
                size++;
            }
            st_info->current_index++;
            av_frame_unref(pFrame);
        }

        if (flushed) break;
    }

    rc = size;

cleanup:
    if (sws) sws_freeContext(sws);
    if (buffer) av_free(buffer);
    if (packet) av_packet_free(&packet);
    if (pConvertedFrame) av_frame_free(&pConvertedFrame);
    if (pFrame) av_frame_free(&pFrame);
    return rc;
}

}  // namespace

void vfinfo_close(VFInfo *vfinfo) {
    if (vfinfo->pCodecCtx != NULL) {
        avcodec_free_context(&vfinfo->pCodecCtx);
        vfinfo->pCodecCtx = NULL;
    }
    if (vfinfo->pFormatCtx != NULL) {
        avformat_close_input(&vfinfo->pFormatCtx);
        vfinfo->pFormatCtx = NULL;
    }
    vfinfo->pCodec = NULL;
    vfinfo->width = -1;
    vfinfo->height = -1;
}

int ReadFrames(VFInfo *st_info, CImgList<uint8_t> *pFrameList,
               unsigned int low_index, unsigned int hi_index) {
    st_info->next_index = low_index;

    if (st_info->pFormatCtx == NULL) {
        st_info->current_index = 0;
        if (open_video(st_info) < 0) {
            vfinfo_close(st_info);
            return -1;
        }
    }

    int rc = decode_into_list(st_info, pFrameList, hi_index);
    if (rc < 0) {
        vfinfo_close(st_info);
    }
    return rc;
}

int NextFrames(VFInfo *st_info, CImgList<uint8_t> *pFrameList) {
    if (st_info->pFormatCtx == NULL) {
        st_info->current_index = 0;
        st_info->next_index = 0;
        st_info->videoStream = -1;
        if (open_video(st_info) < 0) {
            vfinfo_close(st_info);
            return -1;
        }
    }

    int rc = decode_into_list(st_info, pFrameList, UINT_MAX);
    if (rc < 0) {
        vfinfo_close(st_info);
    }
    return rc;
}

int GetNumberStreams(const char *file) {
    AVFormatContext *pFormatCtx = NULL;
    av_log_set_level(AV_LOG_QUIET);

    if (avformat_open_input(&pFormatCtx, file, NULL, NULL) != 0) return -1;

    int result = -1;
    if (avformat_find_stream_info(pFormatCtx, NULL) >= 0) {
        result = (int)pFormatCtx->nb_streams;
    }

    avformat_close_input(&pFormatCtx);
    return result;
}

long GetNumberVideoFrames(const char *file) {
    AVFormatContext *pFormatCtx = NULL;
    av_log_set_level(AV_LOG_QUIET);

    if (avformat_open_input(&pFormatCtx, file, NULL, NULL) != 0) return -1;

    long nb_frames = -1;

    if (avformat_find_stream_info(pFormatCtx, NULL) >= 0) {
        int videoStream = find_video_stream(pFormatCtx);
        if (videoStream >= 0) {
            AVStream *str = pFormatCtx->streams[videoStream];
            nb_frames = (long)str->nb_frames;
            if (nb_frames <= 0) {
                nb_frames = (long)av_index_search_timestamp(
                    str, str->duration,
                    AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD);
            }
            if (nb_frames <= 0 && str->time_base.num > 0) {
                int timebase = str->time_base.den / str->time_base.num;
                if (timebase > 0) nb_frames = str->duration / timebase;
            }
        }
    }

    avformat_close_input(&pFormatCtx);
    return nb_frames;
}

float fps(const char *filename) {
    AVFormatContext *pFormatCtx = NULL;
    av_log_set_level(AV_LOG_QUIET);

    if (avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0)
        return -1;

    float result = -1;

    if (avformat_find_stream_info(pFormatCtx, NULL) >= 0) {
        int videoStream = find_video_stream(pFormatCtx);
        if (videoStream >= 0) {
            AVRational r = pFormatCtx->streams[videoStream]->r_frame_rate;
            if (r.den > 0) result = (float)r.num / (float)r.den;
        }
    }

    avformat_close_input(&pFormatCtx);
    return result;
}
