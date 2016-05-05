#include <iostream>
#include <string>
#include <cassert>
#include <fstream>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
}

static int clamp(int x)
{
	if (x < 0)
		return 0;
	if (x > 255)
		return 255;
	return x;
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " <url>" << std::endl;
		return 1;
	}

	int ret;
	AVIOContext *ioCtx = nullptr;
	AVInputFormat *inFmt = nullptr;
	AVFormatContext *frmtCtx = nullptr;
	AVCodec *videoCodec = nullptr;

	av_register_all();

	if ((ret = avio_open(&ioCtx, argv[1], AVIO_FLAG_READ)) < 0)
	{
		std::cerr << "Could not open input!" << std::endl;
		return 1;
	}

	if ((ret = av_probe_input_buffer(ioCtx, &inFmt, argv[1], nullptr, 0, 0)) < 0)
	{
		std::cerr << "Could not probe format" << std::endl;
		return 1;
	}

	std::clog << "Input stream format: " << inFmt->name << std::endl;

	if ((ret = avformat_open_input(&frmtCtx, argv[1], inFmt, nullptr)) < 0)
	{
		std::cerr << "Failed to create context" << std::endl;
		return 1;
	}

	if ((ret = avformat_find_stream_info(frmtCtx, nullptr)) < 0)
	{
		std::cerr << "Failed to find stream info" << std::endl;
		return 1;
	}

	for (int i = 0; i < frmtCtx->nb_streams; i++)
		std::clog << "Stream " << i << " has format " << frmtCtx->streams[i]->codec->codec_id << std::endl;

	AVPacket pkt;
	pkt.buf = nullptr;

	AVCodecContext *videoContext = frmtCtx->streams[0]->codec;
	videoCodec = avcodec_find_decoder(videoContext->codec_id);

	if ((ret = avcodec_open2(videoContext, videoCodec, nullptr)) < 0)
	{
		std::cerr << "Could not open context" << std::endl;
		return 1;
	}

	AVFrame *frame = av_frame_alloc();
	int got_picture = 0;

	for (int i = 0; i < 192; i++, got_picture = 0)
		while (!got_picture)
		{
			if ((ret = av_read_frame(frmtCtx, &pkt)) < 0)
			{
				std::cerr << "Failed to read frame" << std::endl;
				return 1;
			}
			if (pkt.stream_index != 0)
				continue;

			if ((ret = avcodec_decode_video2(videoContext, frame, &got_picture, &pkt)) < 0)
			{
				std::cerr << "Could not decode packet" << std::endl;
				return 1;
			}
		}

	std::clog << "Video frame size: " << frame->width << "x" << frame->height << std::endl;
	std::clog << "Video format: " << av_get_pix_fmt_name((AVPixelFormat) frame->format) << std::endl;
	AVRational pos = frmtCtx->streams[0]->time_base;
	pos.num *= pkt.pts;
	std::clog << "Frame timestamp: " << pos.num << "/" << pos.den << "=" << pos.num/pos.den << "s" << std::endl;

	{
		std::ofstream out("frame.ppm");
		int width = frame->width;
		int hwidth = width/2;
		int height = frame->height;

		out << "P3" << std::endl;
		out << width << " " << height << std::endl;
		out << 255 << std::endl;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int yp = frame->data[0][y*width+x];
				int u = frame->data[1][y/2*hwidth+x/2];
				int v = frame->data[2][y/2*hwidth+x/2];

				yp -= 16;
				u -= 128;
				v -= 128;
				int r = clamp(yp*1.164 + 1.598*v);
				int g = clamp(yp*1.164 - 0.391*u - 0.813*v);
				int b = clamp(yp*1.164 - 2.018*u);
				//int r = clamp(yp+1.402*(v-128));
				//int g = clamp(yp-0.344*(u-128)-0.714*(v-128));
				//int b = clamp(yp+1.772*(u-128));

				out << r << " " << g << " " << b << " ";
			}
			out << std::endl;
		}
	}

	av_frame_free(&frame);
	avcodec_close(videoContext);

	av_packet_unref(&pkt);

	avformat_close_input(&frmtCtx);

	avio_close(ioCtx);
	return 0;
}
