#ifndef _Moments_h_
#define _Moments_h_

#include <math.h>
#include <iostream>
#include <vector>
#include <assert.h>

class Moments
{

	public:
		Moments() {sum_=0; squares_=0; N_=0; min_=0; max_=0;}
		
		template <class T>
		Moments(std::vector<T> in)
		{
			sum_=0; squares_=0; N_=0; min_=0; max_=0;
			push_back(in);
		}

		void push_back(double a) {
			if (a < min_ || N_==0) min_ = a; if (a > max_ || N_==0) max_ = a;
			N_+=1; sum_+=a; squares_+=a*a;}
		void push_back(double a, int b) {
			if (a < min_ || N_==0) min_ = a; if (a > max_ || N_==0) max_ = a;
			N_+=b; sum_+=b*a; squares_+=a*a*b;}

		void push_back(Moments another);
		
		template <class T>
		void push_back(std::vector<T> in)
		{
			for ( unsigned int i = 0; i < in.size(); i++ )
			{
				push_back((double)(in[i]));
			}
		}
		
		// Note: min_ and max_ may be wrong after removal if a==min_ or a==max_.
		void removeEntry(double a) { N_ -= 1; sum_ -= a; squares_ -= a*a;}

		unsigned int count() const {return N_;}

		double mean() const
		{
			if (N_!=0) return sum_/double(N_);
			else return 0;
		}
		
		double uncertainty() const
		{
			if (N_<=1) return -1;
			else return stddev()/sqrt((double)count());
		}

		double stddev() const
		{
		  if (N_<=1) return -1;
		  double tmp=squares_/double(N_)-mean()*mean();
		  if (tmp>=0.) return sqrt(tmp);
		  if (squares_/double(N_)<0.99999999*mean()*mean()){
		    std::cout << "N_ sum_ squares_:"<<N_<<" "<<sum_<<" "<<squares_<<std::endl; std::cout << "(Don't use stddev() on Moments which have been added or subtracted.)"<<std::endl; assert(0);}
		  return 0.;
		}

		void clear() {N_=0; sum_=0; squares_=0;}

		double sum() const {return sum_;}
		double squares() const {return squares_;}
		double min() const {return min_;}
		double max() const {return max_;}

		// "sort of" addition and subtraction -- at least keeps the mean the same
		const Moments operator+(const Moments& another) const{
			Moments toReturn;
			toReturn.N_ = std::min(N_,another.N_);
			toReturn.sum_ = toReturn.N_*(mean() + another.mean());
			toReturn.squares_ = -1;
			toReturn.min_ = min_ + another.min_;
			toReturn.max_ = max_ + another.max_;
			return toReturn;
		}
		
		const Moments operator-() const{
			Moments toReturn;
			toReturn.sum_ = -sum_;
			toReturn.squares_ = squares_;
			toReturn.N_ = N_;
			toReturn.min_ = max_;
			toReturn.max_ = min_;
			return toReturn;
		}
		
		const Moments operator-(const Moments& another) const{
			return (*this) + (-another);
		}

	private:

		double sum_, squares_;
		unsigned int N_ ;
		double min_, max_;
};

#endif
