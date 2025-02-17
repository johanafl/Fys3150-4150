#include <cmath>
#include <chrono>
#include "gauss_laguerre.cpp"

double const pi = 3.14159265359; 


void gauss_legendre_points(double x1, double x2, double x[], double w[], int n)
{
    int m, j, i;
    double z1, z, xm, xl, pp, p3, p2, p1;
    double *x_low, *x_high, *w_low, *w_high;

    double const ZERO = std::pow(10, -10);

    m  = (n + 1)/2;          // roots are symmetric in the interval
    xm = 0.5 * (x2 + x1);
    xl = 0.5 * (x2 - x1);

    x_low  = x;              // pointer initialization
    x_high = x + n - 1;
    w_low  = w;
    w_high = w + n - 1;

    for(i = 1; i <= m; i++)
    {      // loops over desired roots
        z = std::cos(pi * (i - 0.25)/(n + 0.5));

        /*
        Starting with the above approximation to the ith root
        we enter the main loop of refinement bt Newtons method.
        */

        do
        {
            p1 = 1.0;
	        p2 = 0.0;

   	        /*
	        loop up recurrence relation to get the
            Legendre polynomial evaluated at x
            */

            for(j = 1; j <= n; j++)
            {
	            p3 = p2;
	            p2 = p1;
	            p1 = ((2.0 * j - 1.0) * z * p2 - (j - 1.0) * p3)/j;
	        }

	        /*
	        p1 is now the desired Legrendre polynomial. Next compute
            ppp its derivative by standard relation involving also p2,
            polynomial of one lower order.
            */
 
	        pp = n*(z*p1 - p2)/(z*z - 1.0);
	        z1 = z;
	        z  = z1 - p1/pp;                   // Newton's method
        }
        while(std::fabs(z - z1) > ZERO);

        /* 
	    ** Scale the root to the desired interval and put in its symmetric
        ** counterpart. Compute the weight and its symmetric counterpart
        */

      *(x_low++)  = xm - xl*z;
      *(x_high--) = xm + xl*z;
      *w_low      = 2.0*xl/((1.0 - z*z)*pp*pp);
      *(w_high--) = *(w_low++);
    }
}


double integrand(double r1, double r2, double theta1, double theta2, double phi1, double phi2)
{
    /*
    Here we calculate the integrand that we want to integrate.

    Parameters
    ----------
    r1 : double
        The r-value for the first part of the integrand/for the first electron.

    r2 : double
        The r-value for the second part of the integrand/for the second electron.

    theta1 : double
        The theta-value for the first part of the integrand/for the first electron.

    theta2 : double
        The theta-value for the second part of the integrand/for the second electron.

    phi1 : double
        The phi-value for the first part of the integrand/for the first electron.

    phi2 : double
        The phi-value for the second part of the integrand/for the second electron.


    Returns
    -------
    : double
        The value of the integrand in the spesified point.
    */

    double tol = 1e-10;
    double cos_beta = std::cos(theta1)*std::cos(theta2) + std::sin(theta1)*std::sin(theta2)*std::cos(phi1 - phi2);
    double r12 = r1*r1 + r2*r2 - 2*r1*r2*cos_beta;
    
    if (r12 < tol)
    {
        return 0;
    }    
    else
    {
        r12 = std::sqrt(r12);
        return 1/r12;
    }
    
}


void gauss_laguerre_quadrature(int N_start, int N_end, int dN)
{
    /*
    Calculate the integral of exp(-2*alpha*(r1 + r2))/|r1 - r2|, with alpha=1,
    using gaussian quadrature with laguerre polynomials for the r-dependence
    and legendre polynomials for the theta- and phi-dependence.

    Parameters
    ----------
    N_start : int
        Start grid point value.

    N_end : int
        End grid point value.

    dN : int
        Grid point step size.
    */

    // generating data file
    std::ofstream laguerre_data_file;
    laguerre_data_file.open("data_files/laguerre_data.txt", std::ios_base::app);

    // writing title to file
    laguerre_data_file << std::setw(20) << "N" << std::setw(20) << "error";
    laguerre_data_file << std::setw(20) << "calculated";
    laguerre_data_file << std::setw(20) << "exact";
    laguerre_data_file << std::setw(20) << "comp time (s)" << std::endl;

    for (int N = N_start; N <= N_end; N += dN)
    {   // loops over grid values

        // starting timer
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        
        double alpha = 2;   // laguerre assumes a function of the form x^{alpha} exp(-x), and we must specify alpha.

        double *r     = new double[N+1];  // Arrays for the r, theta and phi points.
        double *theta = new double[N];
        double *phi   = new double[N];

        double *w_r     = new double[N+1]; // Arrays for the r, theta and phi weights.
        double *w_theta = new double[N];
        double *w_phi   = new double[N];

        // Finding the weights and points for integration.
        gauss_laguerre(r, w_r, N, alpha);     // possibly change to N
        gauss_legendre_points(0, pi, theta, w_theta, N);
        gauss_legendre_points(0, 2*pi, phi, w_phi, N);


        double integral_sum = 0;

        // The actual integral is approximated with a sum.
        for (int i0 = 1; i0 < N+1; i0++)
        {   
            std::cout << "outer loop: " << i0 << " of " << N << std::endl;
            
            for (int i1 = 1; i1 < N+1; i1++)
            {
                for (int i2 = 0; i2 < N; i2++)
                {
                    for (int i3 = 0; i3 < N; i3++)
                    {
                        for (int i4 = 0; i4 < N; i4++)
                        {
                            for (int i5 = 0; i5 < N; i5++)
                            {
                                // Multiplying the weights with the integrand.
                                integral_sum += w_r[i0]*w_r[i1]*w_theta[i2]*w_theta[i3]*w_phi[i4]*w_phi[i5]
                                    *integrand(r[i0], r[i1], theta[i2], theta[i3], phi[i4], phi[i5])
                                    *std::sin(theta[i2])*std::sin(theta[i3]);
                            }
                        }
                    }
                }
            }
        }

        // factor from change of variables
        integral_sum /= std::pow( (2*2), 5);

        // ending timer
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double> comp_time = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);

        double exact = 5*pi*pi/(16*16);
        double error = std::fabs(integral_sum - exact);

        std::cout << "\ncalculated: " << integral_sum << std::endl;
        std::cout << "correct answer: " << exact << std::endl;
        std::cout << "error: " << error << std::endl;
        std::cout << N << " of " << N_end << "\n" << std::endl;

        laguerre_data_file << std::setw(20) << N << std::setw(20) << error;
        laguerre_data_file << std::setw(20) << integral_sum;
        laguerre_data_file << std::setw(20) << exact;
        laguerre_data_file << std::setw(20) << comp_time.count() << std::endl;

        delete[] r;
        delete[] theta;
        delete[] phi;

        delete[] w_r;
        delete[] w_theta;
        delete[] w_phi;
    }
}


int main()
{   
    int N_start = 1;
    int N_end = 30;
    int dN = 1;
    
    gauss_laguerre_quadrature(N_start, N_end, dN);
    
    return 1;
}