#include <iostream>
#include <algorithm>
#include <fstream>
#include "Functions.h"

// MTUniform is the Mersenne twister, MWCUniform is the multiply with carry
//    generator that we discussed in class.


double MWCUniform (unsigned int);
unsigned int Temper (unsigned int);
//int power(int, int);
//int test(double *);
//double Uniform(int);


int power(int x, int a) {
    int val = 1;

    // iterates to the power provided in order to determine the expression x^a
    for (int i = 0; i < a; ++i) {
        val *= x;
    }
    return val;
}
    

double Uniform(int w) {
    // determines which uniform random number generator is to be used either MTUniform (Mersenne twister) or MWCUniform (multiply with carry)
    if (w == 1) {
        return MTUniform(0);
    }
    else if (w == 2) {
        return MWCUniform(0);
    }
    else {
        return 0;
    }
}


int test(double *U) {
    // split the unit interval into 40 disjoint subintervals of equal length 
    double lower, upper, delta = 0.025;
    int pw = 3, c = 0, M = 0;

    /*M1 = int(MWCUniform(0) / 0.025) * 40 * 40;
    M2 = int(MWCUniform(0) / 0.025) * 40;
    M3 = int(MWCUniform(0) / 0.025);
    M = M1 + M2 + M3;*/

    for (int i = 0; i < 3; i++) {
        std::cout << "Vector " << U[i] << "\n";
        std::cout << "Sample " << MWCUniform(0) << "\n";
        M += int(MWCUniform(0) / delta) * power(40, pw - 1);
        --pw;
    }

    //// k iterates from 0 < k < 40 for each disjoint interval
    //for (double k = 0; k < 40; k++) {
    //    // creating the bounds for the restricted interval I, (k*delta, (k+1)*delta]
    //    lower = k * delta;
    //    upper = (k + 1.0) * delta;

    //    // if we have that our uniform random variable U[c] at index (c) is in the restricted interval I denoted by the lower/upper levels we consider M_1, M_2, M_3  
    //    if ((U[c] > lower) && (U[c] <= upper) && (pw > 0)) {
    //        // if the condition is met we form our M through the sumproduct of k's and 40 raised to the powers [2,1,0] in that order
    //        M += (k * power(40, pw - 1));
    //        --pw;
    //        ++c;
    //    }
    //}
    return M;
}


int main() {
    double Z, p, q, mu, sigma, U[3];
    int w, n, m;
    
    // Allocate space for X[1],... X[64000] and initialize each to 0.
    int X[64000] = { };

    // allow the user to select their specific number generator 
    printf("Which random number generator would you like to implement \n");
    printf("(1) MTUniform \n");
    printf("(2) MWCUniform \n");
    w = GetInteger("Enter your selection: ");

    // select the number of simulations you would like to run
    n = GetInteger("How many simulations would you like to run (suggested 10 million): ");

    // running n simulations (suggested 10, 50, 100 million) to test normalacy
    for (int i = 0; i < n; i++) {

        for (int j = 0; j < 3; j++) {
            U[j] = Uniform(w);
        }

        // Compute the value of M provided the testing instructions
        std::sort(U, U + 3);
        m = test(U);       

        // Increment the appropriate X by 1.
        ++ X[m];
        break;
    }

    // Now each X[m] should be Binomial (n, p), where p = 1/40320.
    p = 1.0 / 64000.0;
    q = 1.0 - p;
    mu = n * p;
    sigma = sqrt(n * p * q);

    // If n is large, (X[m] - mu)/sigma should be approximately Normal(0,1).
    // Normalize and add to a normal histogram.
    for (int i = 0; i < 64000; i++) {
        Z = (X[i] - mu) / sigma;
        NormalHistogram(Z, 40, 0);
    }

    // Create the TeX files for viewing.
    NormalHistogram(0, 40, 1);
    
    printf("The simulation is complete...\n");
    
    Exit ();
}



////////////////////////////////////////////////////////////////////////////////
// The following code implements the Multiply-With-Carry RNG.
////////////////////////////////////////////////////////////////////////////////
double MWCUniform (unsigned int seed) {

   // Static variables retain their values between function calls.
   static unsigned int a1, a0, n1, n0, c1, c0, initialized=0;

   unsigned int w1, w0, x1, x0, y1, y0, z1, z0, carry, N;

   // This function is called only be MWCUniform and follows below.
   void Split (unsigned int, unsigned int *, unsigned int *);

   // Seed the MWC function when it is passed a non-zero seed or it is not yet
   //    initialized.
   // Represent a, n, and c as with pairs of 16-bit numbers:
   //  a = a1 * 2^16 + a0 (the multiplier)
   //  n = n1 * 2^16 + n0 (initially the seed)
   //  c = c1 * 2^16 + c0 (the increment, initially 1)
   if (seed || !initialized) {
      seed = (seed ? seed : 1);
      Split (4294967118, &a1, &a0);
      Split (seed, &n1, &n0);
      c0 = 1;                                 // first 16 bits of "c"
      c1 = 0;                                 // second 16 bits
      initialized = 1;
   }

   // Generate the next number in the sequence.

   // Compute w1, w0, where a0 * n0 = w1 * 2^16 + w0.
   Split (a0 * n0, &w1, &w0);

   // Compute x1, x0, where a1 * n0 = x1 * 2^16 + x0.
   Split (a1 * n0, &x1, &x0);

   // Compute y1, y0, where a0 * n1 = y1 * 2^16 + y0.
   Split (a0 * n1, &y1, &y0);

   // Compute z1, z0, where a1 * n1 = z1 * 2^16 + z0.
   Split (a1 * n1, &z1, &z0);

   // Compute next n0, n1, c0, c1, where an + c = c1 * 2^48 + c0 * 2^32 + n1 * 2^16 + n0.

   // First compute n0 and whatever is carried.
   Split (w0 + c0, &carry, &n0);

   // Compute n1 and whatever is carried.
   Split (carry + w1 + x0 + y0 + c1, &carry, &n1);

   // Compute c0 and whatever is carried.
   Split (carry + x1 + y1 + z0, &carry, &c0);

   // Compute c1; nothing is carried.
   c1 = z1 + carry;

   // Re-assemble n1 and n0.
   N = (n1 << 16) + n0;

   return ((N + 0.5) / 4294967296.0);

}

// This function computes 16-bit numbers x1 and x0 such that x = x1 * 2^16 + x0.
// Pointers *x1 and *x0 are used because a function can return only one value.
void Split (unsigned int x, unsigned int *x1, unsigned int *x0) {

   // First 16 bits of x.
   *x0 = x & 0xffff;

   // Second 16 bits of x.
   *x1 = x >> 16;

   return;

}

// This is the Mersenne Twister Temper function.
unsigned int Temper (unsigned int N) {

   N ^= (N >> 11);
   N ^= (N << 7) & 0x9d2c5680;
   N ^= (N << 15) & 0xefc60000;
   N ^= (N >> 18);

   return N;

}










