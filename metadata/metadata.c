/*
 * Copyright (c) 2011 Reinhard Tartler
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * Shows how the metadata API can be used in application programs.
 * @example metadata.c
 */

#ifdef _MSC_VER
#define inline __inline
#define snprintf _snprintf
#endif

//����SDL���ͷ�ļ�
#ifdef __cplusplus
extern "C"
{
#endif
#include "../SDL2.X/include/SDL.h"		   //SDL��
#ifdef __cplusplus
};
#endif
//����SDL�����
#pragma comment(lib,"../SDL2.X/lib/SDL2.lib")
#pragma comment(lib,"../SDL2.X/lib/SDL2main.lib")
#ifdef __cplusplus
extern "C"
{
#endif
#include "libavformat/avformat.h"
#include "libavutil/dict.h"
#ifdef __cplusplus
};
#endif

#pragma comment(lib,"../ffmpeg/lib/avcodec.lib")
#pragma comment(lib,"../ffmpeg/lib/avdevice.lib")
#pragma comment(lib,"../ffmpeg/lib/avfilter.lib")
#pragma comment(lib,"../ffmpeg/lib/avformat.lib")
#pragma comment(lib,"../ffmpeg/lib/avutil.lib")
#pragma comment(lib,"../ffmpeg/lib/postproc.lib")
#pragma comment(lib,"../ffmpeg/lib/swresample.lib")
#pragma comment(lib,"../ffmpeg/lib/swscale.lib")
#include <stdio.h>



int main (int argc, char **argv)
{
    AVFormatContext *fmt_ctx = NULL;
    AVDictionaryEntry *tag = NULL;
    int ret;

    if (argc != 2) {
        printf("usage: %s <input_file>\n"
               "example program to demonstrate the use of the libavformat metadata API.\n"
               "\n", argv[0]);
        return 1;
    }

    av_register_all();
    if ((ret = avformat_open_input(&fmt_ctx, argv[1], NULL, NULL)))
        return ret;

    while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
        printf("%s=%s\n", tag->key, tag->value);

    avformat_close_input(&fmt_ctx);
    return 0;
}
