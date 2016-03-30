#ifdef _MSC_VER
#define inline __inline
#define snprintf _snprintf
#endif

//引入SDL库的头文件
#ifdef __cplusplus
extern "C"
{
#endif
#include "../SDL2.X/include/SDL.h"		   //SDL库
#ifdef __cplusplus
};
#endif
//引入SDL导入库
#pragma comment(lib,"../SDL2.X/lib/SDL2.lib")
#pragma comment(lib,"../SDL2.X/lib/SDL2main.lib")
#ifdef __cplusplus
extern "C"
{
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/file.h"
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



struct buffer_data {
	uint8_t *ptr;
	size_t size; ///< size left in the buffer
};

static int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
	struct buffer_data *bd = (struct buffer_data *)opaque;
	buf_size = FFMIN(buf_size, bd->size);

	printf("ptr:%p size:%zu\n", bd->ptr, bd->size);

	/* copy internal buffer data to buf */
	memcpy(buf, bd->ptr, buf_size);
	bd->ptr += buf_size;
	bd->size -= buf_size;

	return buf_size;
}

int main(int argc, char *argv[])
{
	AVFormatContext *fmt_ctx = NULL;
	AVIOContext *avio_ctx = NULL;
	uint8_t *buffer = NULL, *avio_ctx_buffer = NULL;
	size_t buffer_size, avio_ctx_buffer_size = 4096;
	char *input_filename = NULL;
	int ret = 0;
	struct buffer_data bd = { 0 };

	if (argc != 2) {
		fprintf(stderr, "usage: %s input_file\n"
			"API example program to show how to read from a custom buffer "
			"accessed through AVIOContext.\n", argv[0]);
		return 1;
	}
	input_filename = argv[1];

	/* register codecs and formats and other lavf/lavc components*/
	av_register_all();

	/* slurp file content into buffer */
	ret = av_file_map(input_filename, &buffer, &buffer_size, 0, NULL);
	if (ret < 0)
		goto end;

	/* fill opaque structure used by the AVIOContext read callback */
	bd.ptr = buffer;
	bd.size = buffer_size;

	if (!(fmt_ctx = avformat_alloc_context())) {
		ret = AVERROR(ENOMEM);
		goto end;
	}

	avio_ctx_buffer = av_malloc(avio_ctx_buffer_size);
	if (!avio_ctx_buffer) {
		ret = AVERROR(ENOMEM);
		goto end;
	}
	avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
		0, &bd, &read_packet, NULL, NULL);
	if (!avio_ctx) {
		ret = AVERROR(ENOMEM);
		goto end;
	}
	fmt_ctx->pb = avio_ctx;

	ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL);
	if (ret < 0) {
		fprintf(stderr, "Could not open input\n");
		goto end;
	}

	ret = avformat_find_stream_info(fmt_ctx, NULL);
	if (ret < 0) {
		fprintf(stderr, "Could not find stream information\n");
		goto end;
	}

	av_dump_format(fmt_ctx, 0, input_filename, 0);

end:
	avformat_close_input(&fmt_ctx);
	/* note: the internal buffer could have changed, and be != avio_ctx_buffer */
	if (avio_ctx) {
		av_freep(&avio_ctx->buffer);
		av_freep(&avio_ctx);
	}
	av_file_unmap(buffer, buffer_size);

	if (ret < 0) {
		fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
		return 1;
	}

	system("pause");
	return 0;
}