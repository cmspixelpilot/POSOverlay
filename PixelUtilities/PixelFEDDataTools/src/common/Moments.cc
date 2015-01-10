#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include <algorithm>

void Moments::push_back(Moments another)
{
	if ( another.count() == 0 ) return;
	sum_     += another.sum();
	squares_ += another.squares();
	if ( count() == 0 )
	{
		N_   = another.count();
		min_ = another.min();
		max_ = another.max();
		return;
	}
	N_ += another.count();
	min_ = std::min( min(), another.min() );
	max_ = std::max( max(), another.max() );
}
