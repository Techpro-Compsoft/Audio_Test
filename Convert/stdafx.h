#pragma once

// TODO: reference additional headers your program requires here
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
}


using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Text;

#include <string>

#include "Interop.h"
#include "Util.h"
#include "Codec.h"
#include "Parameters.h"
#include "Options.h"
#include "Source.h"
#include "Sink.h"
#include "FilterGraph.h"
#include "Mixer.h"


#define TRACE(fmt,...) \
	Console::WriteLine(fmt,__VA_ARGS__)

/*
#define TRACE(fmt,...) 
*/
