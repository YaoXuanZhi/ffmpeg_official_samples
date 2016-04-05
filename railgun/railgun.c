// railgun.c

// A small sample program that shows how to use libavformat and libavcodec to
// read video from a file and write frame to a bmp file.
//
// Use
//
// gcc railgun.c -lavutil -lavformat -lavcodec -lswscale
//
// to build (assuming libavformat and libavcodec are correctly installed on
// your system).
//
// Run using
//
// ./a.out air.mpg 000130 000132 
//
// to write frames(00:01:30--00:01:32) from "air.mpg" to disk in BMP
// format.
//源码来源于：http://bbs.chinaunix.net/thread-1932536-1-1.html

#ifdef _MSC_VER
#define inline __inline
#define snprintf _snprintf
#endif

//引入SDL库的头文件
//#ifdef __cplusplus
//extern "C"
//{
//#endif
//#include "../SDL2.X/include/SDL.h"		   //SDL库
//#ifdef __cplusplus
//};
//#endif
////引入SDL导入库
//#pragma comment(lib,"../SDL2.X/lib/SDL2.lib")
//#pragma comment(lib,"../SDL2.X/lib/SDL2main.lib")
//#ifdef __cplusplus
//extern "C"
//{
//#endif
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


#undef sprintf
#undef uint8_t
#undef uint16_t
#undef uint32_t
#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned long

#pragma pack(2)


//兼容VC6.0和VS系列的fopen函数
int fopenEx(FILE* _File, const char *filename, const char* _Mode)
{
	int IsSucess = 0;
#if _MSC_VER<1400   //VS 2005版本以下                    
	if ((_File = fopen(filename, _Mode)) != NULL)
#else  //VS 2005版本以上
	errno_t Rerr = fopen_s(&_File, filename, _Mode);
	if (Rerr == 0)
#endif 
		IsSucess = 0;
	return IsSucess;
}


typedef struct BMPHeader
{
    uint16_t identifier;
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
} BMPHeader;

typedef struct BMPMapInfo
{
    uint32_t header_size;
    uint32_t width;
    uint32_t height;
    uint16_t n_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t data_size;
    uint32_t hresolution;
    uint32_t vresolution;
    uint32_t n_colors_used;
    uint32_t n_important_colors;
}BMPMapInfo;

int CreateBmpImg(AVFrame *pFrame, int width, int height, int iFrame)
{
    BMPHeader bmpheader;
    BMPMapInfo bmpinfo;
    FILE *fp=NULL;
    int y;
    char filename[32];
    
    // Open file
    memset(filename, 0x0, sizeof(filename));
    sprintf(filename, "%d.bmp", iFrame);
    //fp = fopen(filename, "wb");
	fopenEx(fp,filename,"wb");
    if(!fp)return -1;

    bmpheader.identifier = ('M'<<8)|'B';
    bmpheader.reserved = 0;
    bmpheader.data_offset = sizeof(BMPHeader) + sizeof(BMPMapInfo);
    bmpheader.file_size = bmpheader.data_offset + width*height*24/8;

    bmpinfo.header_size = sizeof(BMPMapInfo);
    bmpinfo.width = width;
    bmpinfo.height = height;
    bmpinfo.n_planes = 1;
    bmpinfo.bits_per_pixel = 24;
    bmpinfo.compression = 0;
    bmpinfo.data_size = height*((width*3 + 3) & ~3);
    bmpinfo.hresolution = 0;
    bmpinfo.vresolution = 0;
    bmpinfo.n_colors_used = 0;
    bmpinfo.n_important_colors = 0;

    fwrite(&bmpheader,sizeof(BMPHeader),1,fp);
    fwrite(&bmpinfo,sizeof(BMPMapInfo),1,fp);
    for(y=height-1; y>=0; y--)
        fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, fp);
    fclose(fp);

    return 0;
}

//解码指定videostream，并保存frame数据到pFrame上
//返回: 0--成功，非0--失败
int DecodeVideoFrame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, 
    int videoStream, int64_t endtime, AVFrame *pFrame)
{
    static AVPacket packet;
    static uint8_t *rawData;
    static int bytesRemaining = 0;
    int bytesDecoded;
    int frameFinished;
    static int firstTimeFlag = 1;

    if (firstTimeFlag)
    {
        firstTimeFlag = 0;
        packet.data = NULL;//第一次解frame，初始化packet.data为null
    }

    while (1)
    {
        do 
        {
            if (packet.data == NULL) av_free_packet(&packet); //释放旧的packet
            if (av_read_frame(pFormatCtx, &packet) < 0) 
            {
                //从frame读取数据保存到packet上，<0表明到了stream end
                printf("-->av_read_frame end\n");
                goto exit_decode;
            }
        } while (packet.stream_index != videoStream); //判断当前frame是否为指定的video stream

        //判断当前帧是否到达了endtime，是则返回false，停止取下一帧
        if (packet.pts >= endtime) return -1;
        
        bytesRemaining = packet.size;
        rawData = packet.data;

        while (bytesRemaining > 0)
        {
            bytesDecoded = avcodec_decode_video(pCodecCtx, pFrame, &frameFinished, rawData, bytesRemaining);
            if (bytesDecoded < 0) return -1;

            bytesRemaining -= bytesDecoded;
            rawData += bytesDecoded;

            if (frameFinished) return 0;
        }
    }

exit_decode:
    bytesDecoded = avcodec_decode_video(pCodecCtx, pFrame, &frameFinished, rawData, bytesRemaining);
    if(packet.data != NULL) av_free_packet(&packet);
    if (frameFinished != 0) return 0;
    return -1;
}

void usage(const char *function)
{
    printf("Usage: %s [File Name] [Start Time] [End Time]\n", function);
    printf("Ex: ./railgun panda.mpg 003005 003010\n");
    printf("Time Format: HrsMinsSecs. Ex 003005 means 00 hours 30 minutes 05 senconds\n");
    printf("\n");
}

void ParseTime(char strStartTime[], int64_t *pStartSec,
    char strEndTime[], int64_t *pEndSec)
{
    int64_t starttime = 0, endtime = 0;
    if (strStartTime && pStartSec)
    {
        starttime = atoi(strStartTime);
        *pStartSec = (3600*starttime/10000) + \
                (60*(starttime%10000)/100) + \
                (starttime%100);
    }

    if (strEndTime && pEndSec)
    {
        endtime = atoi(strEndTime);
        *pEndSec = (3600*endtime/10000) + \
                (60*(endtime%10000)/100) + \
                (endtime%100);
    }
}

int main(int argc, char *argv[])
{
    const char *filename;
    AVFormatContext *ic = NULL;
    AVCodecContext *dec = NULL;
    AVCodec *codec = NULL;
    AVFrame *frame = NULL;
    AVFrame *frameRGB = NULL;
    uint8_t *buffer = NULL;
    int numBytes;
    int i, videoStream;
    int64_t startTime = 0;
    int64_t endTime = 0;
    static struct SwsContext *img_convert_ctx = NULL;

    // Register all formats and codecs
    av_register_all();

    filename = argv[1];

    // parse begin time and end time
    if (argc == 3)
        ParseTime(argv[2], &startTime, NULL, NULL);
    else if (argc == 4)
        ParseTime(argv[2], &startTime, argv[3], &endTime);
    else
    {
        usage(argv[0]);
        return -1;
    }
    startTime *= AV_TIME_BASE;
    endTime *= AV_TIME_BASE;
    
    // Open video file
    if(av_open_input_file(&ic, filename, NULL, 0, NULL)!=0)
    {
        fprintf(stderr, "Cannt open input file\n");
        usage(argv[0]);
        goto exit_err;
    }

    // Retrieve stream information
    if(av_find_stream_info(ic)<0)
    {
        fprintf(stderr, "Cannt find stream info\n");
        goto exit_err;
    }

    // Dump information about file onto standard error
    dump_format(ic, 0, filename, 0);

    // Find the first video stream
    videoStream=-1;
    for(i=0; i<ic->nb_streams; i++)
	if (ic->streams[i]->codec->codec_type ==/*CODEC_TYPE_VIDEO*/AVMEDIA_TYPE_VIDEO)
        {
            videoStream=i;
            break;
        }
    if(videoStream==-1)
    {
        fprintf(stderr, "No video stream\n");
        goto exit_err;
    }

    // Get a pointer to the codec context for the video stream
    dec=ic->streams[videoStream]->codec;
    // Find the decoder for the video stream
    codec=avcodec_find_decoder(dec->codec_id);
    if(codec==NULL)
    {
        fprintf(stderr, "Found no codec\n");
        goto exit_err;
    }
    // Open codec
    if(avcodec_open(dec, codec)<0)
    {
        fprintf(stderr, "Cannt open avcodec\n");
        goto exit_err; 
    }

    // Allocate video frame
    frame=avcodec_alloc_frame();
    // Allocate an AVFrame structure
    frameRGB=avcodec_alloc_frame();
    if(frameRGB==NULL)
    {
        av_free(frame);
        fprintf(stderr, "Cannt alloc frame buffer for RGB\n");
        goto exit_err;
    }
    // Determine required buffer size and allocate buffer
    numBytes=avpicture_get_size(AV_PIX_FMT_RGB24, dec->width, dec->height);
    buffer=(uint8_t *)av_malloc(numBytes);
    if (!buffer) 
    {
        av_free(frame);
        av_free(frameRGB);
        fprintf(stderr, "Cannt alloc picture buffer\n");
        goto exit_err;
    }
    // Assign appropriate parts of buffer to image planes in pFrameRGB
    avpicture_fill((AVPicture *)frameRGB, buffer, AV_PIX_FMT_RGB24, dec->width, dec->height);

    img_convert_ctx = sws_getContext(dec->width, dec->height, dec->pix_fmt, dec->width, dec->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
    if (img_convert_ctx == NULL) {
        fprintf(stderr, "Cannot initialize the conversion context\n");
        goto exit_err;
    }

    // Seek frame
    startTime = av_rescale_q(startTime, AV_TIME_BASE_Q, ic->streams[videoStream]->time_base);
    endTime = av_rescale_q(endTime, AV_TIME_BASE_Q, ic->streams[videoStream]->time_base);
    avformat_seek_file(ic, videoStream, INT64_MIN, startTime, startTime, 0);
    
    // Read frames and save first five frames to dist
    i=0;
    while(!DecodeVideoFrame(ic, dec, videoStream, endTime, frame))
    {
        // Save the frame to disk
        i++;
        sws_scale(img_convert_ctx, (AVPicture*)frame->data, (AVPicture*)frame->linesize,
            0, dec->height, (AVPicture*)frameRGB->data, (AVPicture*)frameRGB->linesize);
        CreateBmpImg(frameRGB, dec->width, dec->height, i);
    }
    
exit_err:
    // Free the RGB image
    if (buffer)
        av_free(buffer);
    if (frameRGB)
        av_free(frameRGB);
    // Free the YUV frame
    if (frame)
        av_free(frame);
    // Close the codec
    if (dec)
        avcodec_close(dec);
    // Close the video file
    if (ic)
        av_close_input_file(ic);
    if (img_convert_ctx)
        sws_freeContext(img_convert_ctx);

    return 0;
}