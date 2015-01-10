#ifndef _Converter_h_
#define _Converter_h_
#include <vector>

vector<unsigned int> DecimalToBaseX (unsigned int a, unsigned int x, unsigned int length)
{
        vector<unsigned int> ans(100,0);
        int i=0;

        while (a>0)
        {
                ans[i]=a%x;
                //ans.push_back(a%x);
                a=a/x;
                i+=1;
        }

	if (length>0) ans.resize(length); else ans.resize(i);

        return ans;
}

#endif

