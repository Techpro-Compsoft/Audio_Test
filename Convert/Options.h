#pragma once

namespace Avf {
	public ref class Options
	{
	public:
		Options();
		virtual ~Options();
	public:
		Options^ Clone();
	public:
		virtual property bool Ignore {
			bool get(void);
			void set(bool value);
		}
		virtual property bool Normalize {
			bool get(void);
			void set(bool value);
		}
		virtual property Avf::Parameters^  Parameters {
			Avf::Parameters^ get(void);
		}
	private:
		bool ignore = true;
		bool normalize = false;
		Avf::Parameters^ parameters = gcnew Avf::Parameters();

	};
}

