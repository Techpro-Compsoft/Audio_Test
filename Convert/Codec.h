#pragma once

namespace Avf {
	public enum class BitRateMode: int {
		Constant,
		Variable,
		Average,
	};
	public enum class ChannelLayout: Int64 {
		Mono = AV_CH_LAYOUT_MONO,
		Stereo = AV_CH_LAYOUT_STEREO,
	};
	public ref class Codec
	{
	private:
		Codec();
		virtual ~Codec();
	public:
		static property cli::array<Int64>^ BitRates{
			 cli::array<Int64>^ get();
		}
		static property cli::array<BitRateMode>^ BitRateModes{
			 cli::array<BitRateMode>^ get();
		}
		static property cli::array<ChannelLayout>^ ChannelLayouts{
			 cli::array<ChannelLayout>^ get();
		}
		static property cli::array<int>^ SampleRates{
			 cli::array<int>^ get();
		}
		static property cli::array<String^>^ Tags{
			 cli::array<String^>^ get();
		}
		static property String^ Name {
			String^ get();
		}
	};
}
