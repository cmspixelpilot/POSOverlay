#ifndef _LevelEncoderDecoder_h_
#define _LevelEncoderDecoder_h_
#include <vector>

unsigned int levelEncoder(int level)	//Often used for input to the test DAC
{
        unsigned int pulse;

        switch (level)
        {
                case 0: pulse=250; break;
                case 1: pulse=412; break;
                case 2: pulse=518; break;
                case 3: pulse=612; break;
                case 4: pulse=705; break;
		case 5: pulse=850; break;
                default: pulse=0; break;
        }

        return pulse;
}

#endif

