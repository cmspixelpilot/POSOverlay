#include <iostream>
#include <fstream>
#include <string>

using namespace std;

///////////////////////////////////////////////////////////////////////////
// Decode the FIFO-2 data in  transparent mode from piggy
// ADD SIZE
int decodePTrans(unsigned long *data1, unsigned long *data2, const int length)
{
	unsigned long mydat[16] = {0x80, 0x90, 0xa0, 0xb0, 0x70, 0x70, 0x70, 0x70,
							   0x70, 0x70, 0x70, 0x70, 0xc0, 0xd0, 0xe0, 0xf0};

	if (length < 16)
		return -2;
	// Print & analyze the data buffers
	int tempcode1 = 0;
	int tempcode2 = 0;
	int tempcode3 = 0;

	for (int icx = 0; icx < 16; icx++) {
		if (((data1[icx] & 0xf0) == 0x80) == ((data2[icx] & 0xf0) == 0x80)) {
			if ((data1[icx] != data2[icx]))
				tempcode3 = 4;
		}
		if (((data1[icx] & 0xf0) == 0x90) == ((data2[icx] & 0xf0) == 0x90)) {
			if ((data1[icx] != data2[icx]))
				tempcode3 = 4;
		}

		if ((data1[icx] & 0xf0) != (data2[icx] & 0xf0))
			tempcode1 = 1;
		if (((data1[icx] & 0xf0) != mydat[icx]) |
			((data2[icx] & 0xf0) != mydat[icx]))
			tempcode2 = 2;
	}

	if ((tempcode1) != 0)
		cout << "Buffers 0-15 dont match each other!" << endl;
	if ((tempcode2) != 0)
		cout << "Buffers 0-15 dont match expected pattern!" << endl;
	if ((tempcode3) != 0)
		cout << "Buffers 0-15 dont match event numbers!" << endl;

	return (tempcode1 + tempcode2 + tempcode3);

} // end

int main()
{
	unsigned long mydat1[16] = {0x80, 0x90, 0xa0, 0xb0, 0x70, 0x70, 0x70, 0x70,
								0x70, 0x70, 0x70, 0x70, 0xc0, 0xd0, 0xe0, 0xf0};
	unsigned long mydat2[16] = {0x81, 0x90, 0xa0, 0xb0, 0x60, 0x70, 0x70, 0x70,
								0x70, 0x70, 0x70, 0x70, 0xc0, 0xd0, 0xe0, 0xf0};
	unsigned long dummy;
	cin >> hex >> dummy;
	cout << decodePTrans(mydat1, mydat2, 16) << endl;
	return 0;
}
