#pragma once


namespace Avf {

	public ref class Parameters
	{
	public:
		Parameters();
		virtual ~Parameters();
	public:
		static Parameters^ Read(String^ url);
	public:
		Parameters^ Clone();
	public:
		virtual property Int64 BitRate {
			Int64 get(void);
			void set(Int64 value);
		}
		virtual property Avf::BitRateMode BitRateMode{
			Avf::BitRateMode get(void);
			void set(Avf::BitRateMode  value);
		}
		virtual property int Channels {
			int get(void);
			void set(int value);
		}
		virtual property Int64 ChannelLayout {
			Int64 get(void);
			void set(Int64 value);
		}
		virtual property double  Duration {
			double get(void);
			void set(double value);
		}
		virtual property Dictionary<String^, String^>^ Metadata {
			Dictionary<String^, String^>^ get(void);
		}
		virtual property  double Quality{
			double get(void);
			void set(double value);
		}
		virtual property int SampleRate {
			int get(void);
			void set(int value);
		}
	private:
		Int64 bitrate = 128000;
		int channels = 2;
		int sample_rate = 44100;
		double duration = 0;
		double quality = 1;
		Int64 channel_layout = (Int64)Avf::ChannelLayout::Stereo;
		Avf::BitRateMode bitrate_mode = Avf::BitRateMode::Variable;
		Dictionary<String^, String^>^ metadata = gcnew Dictionary<String^, String^>();
	};
}
