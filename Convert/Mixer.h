#pragma once

namespace Avf {
	public ref class Mixer : FilterGraph
	{
	public:
		Mixer(Options^ options, Dictionary<String^, String^>^ inputs,
			Dictionary<String^, String^>^ outputs, List<String^>^ sequence);
		virtual ~Mixer();
		virtual void Open(void);
		virtual property cli::array<String^>^ Inputs{
			  cli::array<String^>^ get(void);
		}
		virtual property  cli::array<String^>^  Outputs{
			 cli::array<String^>^  get(void);
		}
		virtual property  cli::array<String^>^ Sequence{
			 cli::array<String^>^  get(void);
		}
	private:
		String^ Serialize(List<String^>^ inputs, List<String^>^ outputs);
	private:
		Options^ options = nullptr;
		Dictionary<String^, String^>^ inputs = nullptr;
		Dictionary<String^, String^>^ outputs = nullptr;
		List<String^>^ sequence = nullptr;
	};

	public ref class MixerException :
		public Exception
	{
	public:
		MixerException(String^ message);
		virtual ~MixerException();
	};
}

#define THROW_MIXER_EXCEPTION(fmt,...) \
	throw gcnew MixerException(String::Format(fmt,__VA_ARGS__))