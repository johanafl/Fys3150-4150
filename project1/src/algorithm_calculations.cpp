#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <armadillo>
#include <chrono>

void write_to_file(std::string filename, int n, double*v);
void write_to_file_error(std::string filename, int n, double*v);
double relative_error(double v, double u);
void LU_arma(int n);

double rhs_func(double x) {
    /*
    Function for calculating the right-hand-side of the equation, i.e.,
    f(x) = 100e^(-10x).

    Parameters
    ----------
    x : double
        Function variable.
    */
    return 100*exp(-10*x);
}

double exact_solution(double x) {
    /*
    Function for calculating the exact solution to our numerical problem. This will
    be used for comparing the numerical precision for our computed solution.

    Parameters
    ----------
    x : double
        Function variable.
    */
    return 1 - (1 - exp(-10))*x - exp(-10*x);
}

double thomas_algorithm(int n, bool write, bool write_error) {
    /*
    Function for doing Gaussian elimination on our trigonal matrix, i.e.,
    Thomas algorithm, and saving the the values to a .txt-file.

    Since we have a trigonal matrix, this function will iterate over three vectors
    instead of the whole nxn matrix; one vector with n elements which will
    represent the diagonal elements, and two vectors with n-1 elements which will
    represent the upper and lower diagonal elements.

    Parameters
    ----------
    n : int
        Dimension of matrix. Number of discrete points.

    write : bool
        Boolean for toggling write to file on/off.

    write_error : bool
        Boolean value for toggling write error to file on/off.
    */

    double stepsize = 1.0/(n+1);

    double* lower_diag = new double[n-1]; 
    double* diag       = new double[n];   
    double* upper_diag = new double[n-1]; 
    double* rhs_val    = new double[n];   
    double* computed   = new double[n];   

    for (int i=0; i<n; i++)
    {   // calculating the r.h.s. values
        double x = stepsize*(i+1);
        rhs_val[i] = rhs_func(x)*(stepsize*stepsize);
    }

    for (int i=0; i<n-1; i++)
    {   // inserting values in the diagonals
        lower_diag[i] = -1.0;
        diag[i]       = 2.0;
        upper_diag[i] = -1.0;
    }
    diag[n-1] = 2.0;

    // start of timing
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

    for (int i=0; i<n-1; i++)
    {   // elimination of bottom diagonal by forward substitution
        diag[i+1] = diag[i+1] - lower_diag[i]/diag[i]*upper_diag[i];
        rhs_val[i+1] = rhs_val[i+1] - lower_diag[i]/diag[i]*rhs_val[i];
    }

    computed[n-1] = rhs_val[n-1]/diag[n-1];
    for (int i=n-1; i>=1; i--)
    {   // elimination of top diagonal by backwards substitution
        computed[i-1] = (rhs_val[i-1] - upper_diag[i-1]*computed[i])/diag[i-1];
    }

    // end of timing
    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    std::chrono::duration<double> total_time = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);
    

    if (write) 
    {
        std::string filename = "thomas_algorithm_n_" + std::to_string(n) + ".txt";
        write_to_file(filename, n, computed);
    }

    if (write_error)
    {
        std::string filename = "thomas_algorithm_error.txt";
        write_to_file_error(filename, n, computed);
    }
    delete[] lower_diag;
    delete[] diag;
    delete[] upper_diag;
    delete[] rhs_val;
    delete[] computed;

    return total_time.count();
}

double thomas_algorithm_special(int n, bool write, bool write_error) {
    /*
    Function for doing Gaussian elimination for the special trigonal matrix, i.e.,
    Thomas algorithm on a special trigonal matrix, and saving the the values to a
    .txt-file.

    This function works just like the general Thomas algorithm function, only here
    we don't need to include the upper and lower diagonal elements of the matrix
    since they er both just one, saving us some FLOPS.

    Parameters
    ----------
    n : int
        Dimension of matrix. Number of discrete points.

    write : bool
        Boolean for toggling write to file on/off.

    write_error : bool
        Boolean value for toggling write error to file on/off.
    */

    double stepsize = 1.0/(n+1);

    double* diag     = new double[n];   
    double* rhs_val  = new double[n];   
    double* computed = new double[n];   

   for (int i=0; i<n; i++)
    {   // calculating the r.h.s.
        double x = stepsize*(i+1);
        rhs_val[i] = rhs_func(x)*(stepsize*stepsize);
    }

    for (int i=0; i<n; i++)
    {   // inserting values in the diagonal
        diag[i] = 2.0;
    }

    for (int i=0; i<n-1; i++)
    {   // forward substituting, separate loop to exclude from timing
        diag[i+1] = 2.0 - 1.0/diag[i];
    }

    // starting timer
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

    for (int i=0; i<n-1; i++)
    {   // forward substituting
        rhs_val[i+1] = rhs_val[i+1] + rhs_val[i]/diag[i];
    }

    computed[n-1] = rhs_val[n-1]/diag[n-1];
    
    for (int i=n-1; i>=1; i--)
    {   // backward substituting
        computed[i-1] = (rhs_val[i-1] + computed[i])/diag[i-1];
    }

    // ending timer
    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    std::chrono::duration<double> total_time = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);



    if (write) 
    {   
        std::string filename = "thomas_algorithm_special_n_" + std::to_string(n) + ".txt";
        write_to_file(filename, n, computed);
    }

    if (write_error)
    {
        std::string filename = "thomas_algorithm_special_error.txt";
        write_to_file_error(filename, n, computed);
    }

    delete[] diag;
    delete[] rhs_val;
    delete[] computed;

    return total_time.count();
}

void write_to_file(std::string filename, int n, double* computed_val) {
    /*
    Function for writing the values to a .txt-file. Writing the exact and
    computed solution as well as the error for the computed relative to the
    exact solution. If a file with 'filename' already exixts, it is overwritten.

    Parameters
    ----------
    filename : std::string
        A string input with the name of the file which data are written to.

    n : int
        Dimension of matrix. Number of discrete points.

    computed_val : double*
        Pointer to array of computed values to be written to file.
    */
    
    double* exact_val = new double[n];
    double* eps       = new double[n];
    double stepsize   = 1.0/(n+1);
    double x;

    for (int i=0; i<n; i++)
    {   // calculating exact solution and relative error
        x = stepsize*(i+1);
        exact_val[i] = exact_solution(x);
        eps[i] = relative_error(computed_val[i], exact_val[i]);
       
    }

    // writing titles and first known values to file
    std::ofstream results_file;
    results_file.open(filename);
    results_file << std::setw(25) << "U(x) (exact)"
                 << std::setw(25) << "V(x) (discretized)"
                 << std::setw(25) << "Relative error \n"
                 << std::setw(25) << 0
                 << std::setw(25) << 0
                 << std::setw(25) << 0 << std::endl;
    for (int i=0; i<n; i++)
    {   // writing calculated values to file
        results_file << std::setw(25) << std::setprecision(16) << exact_val[i]
                     << std::setw(25) << std::setprecision(16) << computed_val[i]
                     << std::setw(25) << std::setprecision(16) << eps[i]
                     << std::endl;
    }

    // writing final known values to file
    results_file << std::setw(25) << 0
                 << std::setw(25) << 0
                 << std::setw(25) << 0 << std::endl;
    results_file.close();
}

void write_to_file(std::string filename, int n, arma::vec computed_val) {
    /*
    Function for writing the values to a .txt-file.
    Writing the exact and computed solution as well as the error for the
    computed relative to the exact solution. This function is used when we use
    armadillo to do the Thomas algorithm.  If a file with 'filename' already
    exixts, it is overwritten.

    Parameters
    ----------
    filename : std::string
        Filename of output textfile.

    computed_val : arma::vec
        Vector with computed values.

    n : int
        Dimension of matrix. nxn.
    */
    double* computed = new double[n];
    for(int i=0; i<n; i++)
    {
        computed[i] = computed_val[i];
    }
    write_to_file(filename, n, computed);
    delete[] computed;
}

void write_to_file_error(std::string filename, int n, double* computed_val) {
    /*
    Function for writing the relative error to a .txt-file. Appends values to
    already existing file with 'filename'.

    Parameters
    ----------
    filename : std::string
        Filename of output textfile.

    n : int
        Dimension of matrix. nxn.

    computed_val : double*
        Pointer to array with calculated values.
    */
    double exact_val;
    double max_rel_error = 0;
    double stepsize = 1.0/(n+1);

    for (int i=0; i<n; i++)
    {
        double x = stepsize*(i+1);
        exact_val = exact_solution(x);
        double rel_error = relative_error(computed_val[i], exact_val);
        
        if (max_rel_error < rel_error)
        {   // checks if the relative error is larger than 
            max_rel_error = rel_error;
        }
    }

    std::ofstream error_file;
    error_file.open(filename, std::ios_base::app);
    error_file << std::setw(10) << n;
    error_file << std::setw(25) << std::setprecision(16) << max_rel_error << std::endl;
    error_file.close();
}

void write_to_file_error(std::string filename, int n, arma::vec computed_val) {
    /*
    Overloaded function for passing arma::vec instead of double*. Appends values
    to already existing file with 'filename'.

    Parameters
    ----------
    filename : std::string
        Filename of output textfile.

    computed_val : arma::vec
        Vector with computed values.

    n : int
        Dimension of matrix. nxn.
    */

    double* computed = new double[n];
    
    for(int i=0; i<n; i++)
    {
        computed[i] = computed_val[i];
    }
    
    write_to_file_error(filename, n, computed);
    
    delete[] computed;
}

double relative_error(double computed_val, double exact_val) {
    /*
    Function for calculating the relative error for the computed solution relative
    to the exact solution.

    Parameters
    ----------
    computed_val : double
        A single computed value.

    exact_val : double
        A single exact value.
    */
    return fabs((computed_val - exact_val)/exact_val);
}

double LU_arma(int n, bool write, bool write_error) {
    /*
    Function for calculating the Thomas algorithm using the armadillo library.
    This function also prints the time it takes to run the algorithm.

    Parameters
    ----------
    n : int
        Dimension of matrix. Number of discrete points.
    
    write : bool
        Boolean value for toggling write to file on/off.

    write_error : bool
        Boolean value for toggling write error to file on/off.
    */

    double stepsize = 1.0/(n+1);
    arma::mat A(n, n);

    A.diag(0)  += 2.0;
    A.diag(1)  += -1.0;
    A.diag(-1) += -1.0;

    arma::vec rhs_val(n);
    arma::vec tmp(n);
    arma::vec computed;

    arma::mat L;
    arma::mat U;

    L.zeros();
    U.zeros();

   for (int i=0; i<n; i++)
    {   // calculating r.h.s. values
        double x = stepsize*(i+1);
        rhs_val(i) = rhs_func(x)*(stepsize*stepsize);
    }

    // starting timer
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

    arma::lu(L, U, A);
    arma::solve(tmp, L, rhs_val);
    arma::solve(computed, U, tmp);

    // ending timer
    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    std::chrono::duration<double> total_time = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);


    if (write)
    {
        std::string filename = "LU_n_" + std::to_string(n) + ".txt";
        write_to_file(filename, n, computed);
    }

    if (write_error)
    {
        std::string filename = "LU_error.txt";
        write_to_file_error(filename, n, computed);
    }

    return total_time.count();


}

void compare_times() {
    /*
    Function for comparing computation times of thomas, thomas special, and LU.
    Each grid size is calculated 'runs' amount of times, and all timings are
    written to a text file compare_times.txt. If file with 'filename' already
    exists, it is overwritten.
    */

    int grid_values = 12;    // number of different grid values
    int runs = 10;          // number of runs for each grid size
    int N[grid_values];

    N[0]  = 10;
    N[1]  = 100;
    N[2]  = 500;
    N[3]  = 1000;
    N[4]  = 5000;
    N[5]  = 10000;
    N[6]  = 50000;
    N[7]  = 100000;
    N[8]  = 500000;
    N[9]  = 1000000;
    N[10] = 5000000;
    N[11] = 10000000;



    // creating file and writing headers
    std::string filename = "compare_times.txt";
    std::ofstream compare_times_file;
    compare_times_file.open(filename);
    compare_times_file  << std::setw(40) << "thomas algorithm"
                        << std::setw(40) << "thomas algorithm special"
                        << std::setw(40) << "LU\n"
                        << std::setw(40) << runs
                        << std::setw(40) << runs
                        << std::setw(40) << runs << "\n"
                        << std::setw(40) << grid_values
                        << std::setw(40) << grid_values
                        << std::setw(40) << grid_values << "\n";

    for (int i=0; i<grid_values; i++)
    {   // looping over each grid size
        // writing grid size info to file
        std::cout << "calculating grid size " + std::to_string(N[i])
            + " of " + std::to_string(N[grid_values-1]) << std::endl;
        compare_times_file  << std::setw(40) << std::to_string(N[i])
                            << std::setw(40) << std::to_string(N[i])
                            << std::setw(40) << std::to_string(N[i])
                            << "\n";
        
        for (int _=0; _<runs; _++)
        {   // looping over each grid size 'runs' amount of times
            // writing timing data to file
            compare_times_file  << std::setw(40) << std::setprecision(32) 
                                << thomas_algorithm(N[i], false, false)
                                << std::setw(40) << std::setprecision(32)
                                << thomas_algorithm_special(N[i], false, false);
            
            if (N[i] <= 5000)
            {   // limits the gid size for LU since the calculations are
                // impossible with normal hardware at values approaching 100000
            compare_times_file  << std::setw(40) << std::setprecision(32)
                                << LU_arma(N[i], false, false) << std::endl;
            }
            
            else
            {   // writes -1 if LU is incapable of providing results
            compare_times_file  << std::setw(40) << std::setprecision(32)
                                << -1 << std::endl;  
            }
                                
        }
    }

    compare_times_file.close();
}

void calculate_error() {
    /*
    Function for running the three different algorithms in error write mode for
    a set of grid point values. The error data are written to three .txt files.
    */

    int end = 1000;
    int tmp = 0;

    for (int i=145; i<=end; i++)
    {   // iterating over a set of grid values
        int n = (int)std::pow(10, 7.0/end*i);
        
        if (n != tmp)
        {   // excluding equal values caused by rounding from double to int
            tmp = n;
            std::cout << "calculating n = " + std::to_string(n) << std::endl;
            
            thomas_algorithm(n, false, true);
            thomas_algorithm_special(n, false, true);
            
            if (n < 1500)
            {   // sets a limit for the LU decomp. because of hardware limitations
                LU_arma(n, false, true);
            }
        }
    }
}

void calculate_data() {
    /*
    Function for running the three different algorithms for a set of grid point
    values. Data is written to three .txt files.
    */

    int n;

    for (int i=1; i<4; i++)
    {   // looping over a set of grid values
        n = (int)std::pow(10, i);
        
        thomas_algorithm(n, true, false);
        thomas_algorithm_special(n, true, false);
        LU_arma(n, true, false);      
    }
}



int main()
{
    compare_times();
    calculate_error();
    calculate_data();
 
    return 0;
}