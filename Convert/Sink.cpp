#include "stdafx.h"
#include "Sink.h"

using namespace Avf;

Sink::Sink(String^ label, String^ url, Avf::Parameters^ parameters )
{
	this->label = label;
	this->url = url;
	this->parameters = parameters;
}
Sink::Sink(String^ label, String^ url)
{
	this->label = label;
	this->url = url;
}
Sink::~Sink()
{
}
void Sink::Close(void)
{
	this->eof = false;
	this->codec = nullptr;
	this->filter = nullptr;
	this->header = false;
	this->stream = nullptr;

	if (this->frame) {
		AVFrame* frame = this->frame;
		this->frame = nullptr;
		av_frame_free(&frame);
	}
	if (this->packet) {
		AVPacket* packet = this->packet; 
		this->packet = nullptr;
		av_packet_free(&packet);
	}
	if (this->filter) {
		AVFilterContext* filter = this->filter;
		this->filter = nullptr;
		avfilter_free(filter);
	}
	if (this->encoder) {
		AVCodecContext* encoder = this->encoder;
		this->encoder = nullptr;
		avcodec_free_context(&encoder);
	}
	if (this->format) {
		if (this->format->pb) {
			AVIOContext* io = this->format->pb;
			this->format->pb = nullptr;
			avio_closep(&io);
		}
		avformat_free_context(this->format);
		this->format = nullptr;
	}
}
void Sink::Configure(void)
{
	av_buffersink_set_frame_size(this->filter, this->encoder->frame_size);
}
AVFilterInOut* Sink::Input(void)
{
	AVFilterInOut* input = avfilter_inout_alloc();
	if (!input) {
		THROW_SINK_EXCEPTION(this->url, "Cannot allocate link for output.");
	}
	std::string label = Interop::convert(this->label);

	input->name = av_strdup(label.c_str());
	input->filter_ctx = this->filter;
	input->pad_idx = 0;
	input->next = nullptr;
	return input;
}
void Sink::Open(void)
{
	this->format = avformat_alloc_context();
	if (!this->format) {
		THROW_SINK_EXCEPTION(this->url, "Cannot allocate format context for output.");
	}

	std::string url = Interop::convert(this->url);
	int ret = avio_open(&this->format->pb, url.c_str(), AVIO_FLAG_WRITE);
	if (ret < 0) {
		THROW_SINK_EXCEPTION(this->url, "Cannot open output.");
	}

	this->format->oformat = av_guess_format(NULL, url.c_str(), NULL);
	if (!this->format->oformat) {
		THROW_SINK_EXCEPTION(this->url, "Cannot guess format for output.");
	}

	this->format->url = av_strdup(url.c_str());
	if (!this->format->url) {
		THROW_SINK_EXCEPTION(this->url, "Cannot allocate url for output.");
	}

	this->stream = avformat_new_stream(this->format, nullptr);
	if (!this->stream) {
		THROW_SINK_EXCEPTION(this->url, "Cannot create new stream output.");
	}

	this->codec = avcodec_find_encoder(AV_CODEC_ID_MP3);
	if (!this->codec) {
		THROW_SINK_EXCEPTION(this->url, "Cannot find MP3 encoder for output.");
	}

	this->encoder = avcodec_alloc_context3(this->codec);
	if (!this->encoder) {
		THROW_SINK_EXCEPTION(this->url, "Cannot allocate new encoder context for output.");
	}

	this->encoder->channels = this->parameters->Channels;
	this->encoder->channel_layout = av_get_default_channel_layout(this->encoder->channels);
	this->encoder->sample_rate = this->parameters->SampleRate;
	this->encoder->sample_fmt = AV_SAMPLE_FMT_FLTP;
	this->encoder->bit_rate = this->parameters->BitRate;

	AVDictionary* options = nullptr;

	switch (this->parameters->BitRateMode) {
	case BitRateMode::Constant: {
	}break;
	case BitRateMode::Variable: {
		this->encoder->global_quality = 9 - (int)(this->parameters->Quality * 9 + 0.5);
		this->encoder->flags |= AV_CODEC_FLAG_QSCALE;
	}break;
	case BitRateMode::Average: {
		av_dict_set(&options, "abr", "true", 0);
	}break;
	default:break;
	};

	for each(KeyValuePair<String^, String^>^ pair in this->parameters->Metadata) {
		std::string key = Interop::convert(pair->Key);
		std::string value = Interop::convert(pair->Value);
		av_dict_set(&this->format->metadata, key.c_str(), value.c_str(), 0);
	}

	if (this->format->oformat->flags & AVFMT_GLOBALHEADER) {
		this->encoder->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	this->stream->time_base.den = this->encoder->sample_rate;
	this->stream->time_base.num = 1;

	ret = avcodec_open2(this->encoder, this->codec, &options);
	if (ret < 0) {
		THROW_SINK_EXCEPTION(this->url, "Cannot open new encoder for output.");
	}

	ret = avcodec_parameters_from_context(this->stream->codecpar, this->encoder);
	if (ret < 0) {
		THROW_SINK_EXCEPTION(this->url, "Cannot initialize stream parameters output.");
	}
}
void Sink::Insert(AVFilterGraph* graph)
{
	const AVFilter * abuffersink = avfilter_get_by_name("abuffersink");
	if (!abuffersink) {
		THROW_SINK_EXCEPTION(this->url, "Cannot find filter 'abuffersink' for output.");
	}

	std::string label = Interop::convert(this->label);
	AVFilterContext* filter = nullptr;
	int ret = avfilter_graph_create_filter(&filter, abuffersink, label.c_str(), NULL, NULL, graph);
	this->filter = filter;
	if (ret < 0) {
		THROW_SINK_EXCEPTION(this->url, "Cannot create filter 'abuffersink' for output.");
	}

	ret = av_opt_set_bin(this->filter, "sample_fmts",
		(uint8_t*)&this->encoder->sample_fmt, sizeof(this->encoder->sample_fmt),
		AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		THROW_SINK_EXCEPTION(this->url, "Cannot set sample formats for output.");
	}

	ret = av_opt_set_bin(this->filter, "channel_layouts",
		(uint8_t*)&this->encoder->channel_layout, sizeof(this->encoder->channel_layout),
		AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		THROW_SINK_EXCEPTION(this->url, "Cannot set channel layouts for output.");
	}

	ret = av_opt_set_bin(this->filter, "sample_rates",
		(uint8_t*)&this->encoder->sample_rate, sizeof(this->encoder->sample_rate),
		AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		THROW_SINK_EXCEPTION(this->url, "Cannot set sample rates for output.");
	}
}
void  Sink::Write(void)
{
	if (!this->eof) {

		if (!this->frame) {
			frame = av_frame_alloc();
		}
		if (!this->packet) {
			packet = av_packet_alloc();
		}
		try {

			while (!this->eof) {

				int ret = 0;
				if (!this->header) {
					this->header = true;
					TRACE("Writing header for output. ({0})", this->url);
					ret = avformat_write_header(this->format, NULL);
					if (ret < 0) {
						THROW_SINK_EXCEPTION(this->url, "Cannot write header for output.");
					}
				}
				TRACE("Getting frame from the sink filter for output. ({0})", this->url);
				ret = av_buffersink_get_frame(this->filter, this->frame);

				if (ret == AVERROR(EAGAIN)) {
					break;
				}
				else if (ret < 0 && ret != AVERROR_EOF) {
					THROW_SINK_EXCEPTION(this->url, "Cannot get frame from sink filter for output.");
				}
				else {
					this->eof = (ret == AVERROR_EOF);
					if (this->eof) {
						TRACE("Sending eof to the encoder for output. ({0})", this->url);
						ret = avcodec_send_frame(this->encoder, nullptr);
					}
					else {
						if (frame->pts == AV_NOPTS_VALUE) {
							// THROW_SINK_EXCEPTION(this->url, "Presentational timestamp is not available for output.");
						}
						TRACE("Sending frame to the encoder for output. ({0})", this->url);
						ret = avcodec_send_frame(this->encoder, this->frame);
					}
					av_frame_unref(this->frame);

					if (ret < 0) {
						THROW_SINK_EXCEPTION(this->url, "Cannot send frame to the encoder for output.");
					}
					bool eof2 = false;
					while (!eof2) {

						TRACE("Receiving packet from the encoder for output. ({0})", this->url);
						ret = avcodec_receive_packet(this->encoder, this->packet);

						if (ret == AVERROR(EAGAIN) && !this->eof) {
							break;
						}
						else if (ret < 0 && ret != AVERROR_EOF) {
							THROW_SINK_EXCEPTION(this->url, "Cannot get packet from decoder for output.");
						}
						else {
							eof2 = (ret == AVERROR_EOF);

							if (eof2) {
								TRACE("Writing trailer for output.({0})", this->url);
								ret = av_write_trailer(this->format);
								if (ret < 0) {
									THROW_SINK_EXCEPTION(this->url, "Cannot write trailer for output.");
								}
							}
							else {
								TRACE("Writing packet for output.({0})", this->url);
								ret = av_interleaved_write_frame(this->format, this->packet);
								if (ret < 0) {
									THROW_SINK_EXCEPTION(this->url, "Cannot write packet for output.");
								}
							}
							this->eof = this->eof || eof2;
						}
						av_packet_unref(this->packet);
					}
				}
			}
		}
		finally{
			av_packet_unref(this->packet);
			av_frame_unref(this->frame);
		}
	}
}
bool Sink::Eof::get()
{
	return this->eof;
}
String^ Sink::Label::get()
{
	return this->label;
}
Avf::Parameters^  Sink::Parameters::get()
{
	return this->parameters;
}
String^ Sink::Url::get()
{
	return this->url;
}
SinkException::SinkException(String^ url, String^ message) :Exception(message)
{
	this->url = url;
}
SinkException::~SinkException()
{
}
String^ SinkException::Url::get() {
	return this->url;
}
