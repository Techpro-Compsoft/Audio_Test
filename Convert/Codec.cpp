#include "stdafx.h"
#include "Codec.h"

using namespace Avf;

Codec::Codec()
{
}
Codec::~Codec()
{
}
cli::array<Int64>^ Codec::BitRates::get()
{
	cli::array<Int64>^ values = {
		128000,
		160000,
		192000,
		256000,
		320000
	};
	return values;
}
cli::array<BitRateMode>^ Codec::BitRateModes::get()
{
	cli::array<BitRateMode>^ values = {
		BitRateMode::Constant,
		BitRateMode::Variable,
		BitRateMode::Average,
	};
	return values;
}
cli::array<ChannelLayout>^ Codec::ChannelLayouts::get()
{
	cli::array<ChannelLayout>^ values = {
		ChannelLayout::Stereo,
		ChannelLayout::Mono
	};
	List<Int64>^ all = gcnew List<Int64>();
	AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MP3);
	if (codec->channel_layouts) {
		int i = 0;
		while (codec->channel_layouts[i]) {
			all->Add(codec->channel_layouts[i]);
			i++;
		}
	}
	List<ChannelLayout>^ supported = gcnew List<ChannelLayout>();
	for (int i = 0; i < values->Length; i++) {
		if (all->Contains((Int64)values[i])) {
			supported->Add(values[i]);
		}
	}
	return supported->ToArray();


	return values;
}
String^ Codec::Name::get()
{
	return "mp3";
}
cli::array<int>^ Codec::SampleRates::get()
{
	List<int>^ values = gcnew List<int>();
	AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MP3);

	if (codec->supported_samplerates) {
		int i = 0;

		while (codec->supported_samplerates[i]) {
			values->Add(codec->supported_samplerates[i]);
			i++;
		}
	}
	return values->ToArray();
}
cli::array<String^>^ Codec::Tags::get() {
	cli::array<String^>^  values = {

				 "title","artist", "album", "album_artist", "composer",
						"copyright", "encoder", "disc", "encoded_by", "genre" , "language",
						"performer", "lyrics", "publisher",  "track"
	};
	return values;
}

