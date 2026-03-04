/////////////////////////////////////////////////////////////////////////
// Software License Agreement (BSD License)                            //
//                                                                     //
// Copyright (c) 2011                                                  //
// Gerard Benjamin Pierre                                              //
// imdb     : http://www.imdb.com/name/nm0682633/                      //
// linkedin : http://www.linkedin.com/profile/view?id=9048639          //
//                                                                     //
// All rights reserved.                                                //
//                                                                     //
// Redistribution and use in source and binary forms, with or without  //
// modification, are permitted provided that the following conditions  //
// are met:                                                            //
//                                                                     //
//  * Redistributions of source code must retain the above copyright   //
//    notice, this list of conditions and the following disclaimer.    //
//  * Redistributions in binary form must reproduce the above          //
//    copyright notice, this list of conditions and the following      //
//    disclaimer in the documentation and/or other materials provided  //
//    with the distribution.                                           //
//  * Neither the name of the EPFL nor the names of its                //
//    contributors may be used to endorse or promote products derived  //
//    from this software without specific prior written permission.    //
//                                                                     //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS //
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT   //
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS   //
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE      //
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, //
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,//
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;    //
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER    //
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT  //
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN   //
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE     //
// POSSIBILITY OF SUCH DAMAGE.                                         //
/////////////////////////////////////////////////////////////////////////

static const char* const HELP = "CrankItUp\n\nCranks up the colour!\n";

#include <DDImage/NukeWrapper.h>
#include <DDImage/PixelIop.h>
#include <DDImage/Row.h>
#include <DDImage/Knobs.h>
#include <DDImage/DDMath.h>
#include <DDImage/Vector3.h>
#include <DDImage/Vector4.h>

using namespace DD::Image;

static const char* const modes[] = {"magnitude","max","inverse min","max range","max inverse range","pow","min range",NULL};

class CrankItUp : public PixelIop
{
	int mode;

public:

	CrankItUp(Node *node) : PixelIop(node)
	{
		mode = 0;
	}

	~CrankItUp()
	{
	}

	virtual void in_channels(int input, ChannelSet& channels) const
	{
		// Must turn on the other color channels if any color channels are requested:
		ChannelSet done;
		foreach (z, channels) {
			if (colourIndex(z) < 4) { // it is red, green, blue or alpha
				if (!(done & z)) { // save some time if we already turned this on
					done.addBrothers(z, 4); // add all four to the "done" set
				}
			}
		}
		channels += done; // add the colors to the channels we need
	}
	
	//0
	inline Vector4 norm_vec(Vector3 rgb)
	{
		float magnitude = rgb.length();
		return Vector4(rgb.x/magnitude, rgb.y/magnitude, rgb.z/magnitude, magnitude);
	}
	//1
	inline Vector4 norm_max(Vector3 rgb)
	{
		float mx = MAX(MAX(rgb.x,rgb.y),rgb.z);
		return Vector4(rgb.x/mx, rgb.y/mx, rgb.z/mx, mx);
	}
	//2
	inline Vector4 norm_inv_min(Vector3 rgb)
	{
		float mn = MIN(MIN(rgb.x,rgb.y),rgb.z);
		return Vector4(1-(1/(rgb.x/mn)), 1-(1/(rgb.y/mn)), 1-(1/(rgb.z/mn)), mn);
	}
	//3
	inline Vector4 norm_max_range(Vector3 rgb)
	{
		float mx = MAX(MAX(rgb.x,rgb.y),rgb.z);
		float mn = MIN(MIN(rgb.x,rgb.y),rgb.z);
		float range = mx-mn;
		return Vector4((rgb.x/mx)*(1-range), (rgb.y/mx)*(1-range), (rgb.z/mx)*(1-range), (mx)*(1-range));
	}
	//4
	inline Vector4 norm_max_inv_range(Vector3 rgb)
	{
		float mx = MAX(MAX(rgb.x,rgb.y),rgb.z);
		float mn = MIN(MIN(rgb.x,rgb.y),rgb.z);
		float range = mx-mn;

		return Vector4(1-(range/(rgb.x/mx)), 1-(range/(rgb.y/mx)), 1-(range/(rgb.z/mx)), 1-(range/mx));
	}
	//5
	inline Vector4 norm_pow(Vector3 rgb)
	{
		float mn = MIN(MIN(rgb.x,rgb.y),rgb.z);
		return Vector4(powf(1/mn,rgb.x)-1, powf(1/mn,rgb.y)-1, powf(1/mn,rgb.z)-1, powf(1/mn,0.1)-1);
	}
	//6
	inline Vector4 norm_min_range(Vector3 rgb)
	{
		float mx = MAX(MAX(rgb.x,rgb.y),rgb.z);
		float mn = MIN(MIN(rgb.x,rgb.y),rgb.z);
		float range = mx-mn;
		return Vector4((rgb.x/mn)*range, (rgb.y/mn)*range, (rgb.z/mn)*range, (0.1/mn)*range);
	}

	virtual void _validate(bool for_real)
	{
		copy_info();
		info_.channels(Mask_RGBA);
		set_out_channels(Mask_RGBA);
		info_.turn_on(Mask_RGBA);
	}

	virtual void pixel_engine(const Row& in, int y, int x, int r, ChannelMask channels, Row& out)
	{
		ChannelSet done;
		foreach (z, channels)
		{
			if (done & z)
				continue;

			if (colourIndex(z) >= 4) {
				out.copy(in, z, x, r);
				continue;
			}

			done += brother(z, 0);
			done += brother(z, 1);
			done += brother(z, 2);
			done += brother(z, 3);
		}

		const float* inr = in[Chan_Red]+x;
		const float* ing = in[Chan_Green]+x;
		const float* inb = in[Chan_Blue]+x;

		float* outr = out.writable(Chan_Red)+x;
		float* outg = out.writable(Chan_Green)+x;
		float* outb = out.writable(Chan_Blue)+x;
		float* outa = out.writable(Chan_Alpha)+x;

		const float* END = outr+(r-x);

		while (outr < END) {
			Vector3 rgb(*inr, *ing, *inb);
			Vector4 cranked;

			if (mode==0) cranked = norm_vec(rgb);
			if (mode==1) cranked = norm_max(rgb);
			if (mode==2) cranked = norm_inv_min(rgb);
			if (mode==3) cranked = norm_max_range(rgb);
			if (mode==4) cranked = norm_max_inv_range(rgb);
			if (mode==5) cranked = norm_pow(rgb);
			if (mode==6) cranked = norm_min_range(rgb);

			*outr++ = cranked.x;
			*outg++ = cranked.y;
			*outb++ = cranked.z;
			*outa++ = cranked.w;
			*inr++;	*ing++; *inb++;
		}
	}

virtual void knobs(Knob_Callback f)
{
	Text_knob(f, "Crank It Up!","(c) Ben Pierre 2011 "); SetFlags(f, Knob::STARTLINE);
	Enumeration_knob(f, &mode, modes, "mode");
	SetFlags(f, Knob::NO_ANIMATION | Knob::NO_UNDO);
	Tooltip(f, "Cranking up the colour! (by these methods).");
}

const char* Class() const {return d.name;}
const char* node_help() const {return HELP;}
static const Iop::Description d;
};


static Iop* build(Node *node)
{
	return new NukeWrapper(new CrankItUp(node));
}

const Iop::Description CrankItUp::d("CrankItUp", "CrankItUp", build);

// end of CrankItUp.cpp

