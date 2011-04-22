#ifndef __CStatistics__
#define __CStatistics__
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define Distrib Distribuition
#define Rand Random
#define Expo Exponential
#define Weib Weibull
#define Norm Normal
#define NormLim NormalLimited

#define DEFAULT 0

#define DEFAULT 0
#define ADD_VECTOR 1
#define ADD_FILE 2
#define ADD_VECTOR_FILE 3
#define INFINITE 1e300
#define StdDev StandardDeviation
#define DIM 1000
#define TINY 1e-7
//--------------------------------------------------------------------
// CLASS Distribution: randon variates generator
class CDistribution
{
public:
    double U;  // random variate  value
    CDistribution()
    {}
    void Seed(int seed ) { // set a seed to random number
        srand(seed);
    };
    void Randomize() { // set a seed taking the current time as seed
        srand( (unsigned)time( NULL ));
    };
    double Random() { // generates uniform normalized random variates (0-1) using rand()
        U=(rand()%RAND_MAX)/((double)RAND_MAX);
        return U;
    }
    double Uniform(double alpha, double beta) { // generates uniform random variates
        double U=Random();
        if (alpha>0) {
            U=alpha+(beta-alpha)*U;
        }
        return U;
    }
    double Exponential(double beta) { // generates exponential random variates
        double U=Random();
        if (U<TINY) {
            U=TINY;
        }
        U=-beta*log(U);
        return U;
    }

//  Normal generates a normalized normal random number
// using the polar method N(0,1) (avg = 0 std_dev=1)
    double Normal() {
        double Y,W,V1,V2;

        do {
            V1=2*Random()-1;
            V2=2*Random()-1;
            W = V1*V1+V2*V2;
            if (W<1.0) {
                Y=sqrt(-2*log(W)/W);
                V1*=Y;
                V2*=Y;
            }
        } while (W>1.0);
        U=V1;
        return U;
    }

// Normal generates normal distribution variates with average avg
//	and standard deviation std_dev
    double Normal(double avg, double std_dev) {
        U= avg+std_dev*Normal();
        return U;
    }
// Normal generates a normal dist number cuttted by lim_inf and lim_sup
    double  NormalLimited(double avg, double std_dev,
                          double lim_inf,double lim_sup) {
        int n=0;
        double U;
        do {
            n++;
            U=Normal(avg, std_dev);
            if (n>100) { // try 100 times
                break;
            }
        } while (U<lim_inf || U>lim_sup);
        if (n<100) {
            return U;
        }
        if (U<lim_inf) {
            U=lim_inf;
        } else {
            U=lim_sup;
        }
        return U;
    }

};

//--------------------------------------------------------------------
// CLASS CStatistics: keep some CStatistics

class CStatistics
{
private:
    double value,sum_x, sum_x2; // sum of values and sum of value square
    double *vector; // vector to keep data values
    int n,dim;  // n is the number of data, dim is the dimention of teh vector
    bool add_file,add_vector;  // flags to set
    FILE *file;
public:
    double mean, standard_deviation,max,min;
    CStatistics() { // constructor
        sum_x=sum_x2=0.0;
        vector=new double[DIM];
        dim=DIM;
        n=0;
        min=INFINITE;
        max=-INFINITE;
        add_vector=add_file=false;
//file =fopen("values.txt","w");
    };
// another constructor:
//input file_name (to dump data value) mode={ADD_VECTOR,ADD_FILE,ADD_VECTOR_FILE}
    CStatistics(char *file_name, int mode) {
        sum_x=sum_x2=0.0;
        vector=new double[DIM];
        dim=DIM;
        n=0;
        min=INFINITE;
        max=-INFINITE;
        file =fopen(file_name,"w");
        add_vector=add_file=false;
        switch (mode) {
        case ADD_VECTOR:
            add_vector=true;
            break;
        case ADD_FILE:
            add_file=true;
            break;
        case ADD_VECTOR_FILE:
            add_file=add_vector=true;
        }
    };
    ~CStatistics() { // destructor(free memory and close file)
        delete vector;
        fclose(file);
    }
    void SetFile(char *file_name) { // set file name
        file =fopen(file_name,"w");
        add_file=true;
    };
    void SetMode( int mode) { // set mode ; Note: if mode os ADD_FILE must call SetFile name.
        switch (mode) {
        case ADD_VECTOR:
            add_vector=true;
            break;
        case ADD_FILE:
            add_file=true;
            break;
        case ADD_VECTOR_FILE:
            add_file=add_vector=true;
        }
    }
    void Reset() { // reset CStatistics, BUT don't reset file, use SetFile to do it
        sum_x=sum_x2=0.0;
        n=0;
    };
    double *Vector() { // inquire vector
        return vector;
    };
    int N() { // inquire n
        return n;
    };
    void Add(double value) // Add value to Statistics.
    //Keep it to vector and file if respective flag is on
    {
        sum_x+=value;
        sum_x2+=value*value;
        if (value>max) {
            max=value;
        }
        if (value<min) {
            min=value;
        }
        if (add_vector) {
            if (n+1>dim) {
                dim+=DIM;
                vector=(double*)realloc(vector,dim*sizeof(double));
            }
            vector[n]=value;
        }
        if(add_file) {
            fprintf(file,"%lg \n",value);
            fflush(file);
        }
        n++;
    };
    double Mean() { // calculates the average
        if (n>0) {
            return mean=sum_x/n;
        } else {
            return mean=0.0;
        }
    };
    double StandardDeviation() { // calculates de standard deviation
        double std=0;
        if (n>1) {
            std=(sum_x2-sum_x*sum_x/n)/(n-1);
            std=sqrt(std);
        }
        standard_deviation=std;
        return std;
    };

};


#endif

