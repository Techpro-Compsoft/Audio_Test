#include "stdafx.h"
#include "Util.h"


double Avf::Util::Time(Int64 units, AVRational base)
{
	if (base.den) {
		return ((double)(units * base.num)) / base.den;
	}
	else {
		return 0;
	}
}