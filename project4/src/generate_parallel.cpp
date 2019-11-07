#include "energy_solver.h"
#include <chrono>
#include <mpi.h>


class ParallelEnergySolver: public IsingModel
{
public:
    ParallelEnergySolver(int spin_mat_dim, int mc_iterations_input, long seed) 
    : IsingModel(spin_mat_dim, mc_iterations_input, seed){}

    void iterate_temperature_parallel(double initial_temp, double final_temp,
        int num_of_temp_divided_by_num_of_threds)
    {   /*
        ????????????????
        */
        MPI_Init(NULL, NULL);
        int world_rank;
        int world_size;
        MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);
        double diff_temp = (final_temp - initial_temp)/((world_size + 1)*num_of_temp_divided_by_num_of_threds);
        double init_temp = initial_temp + diff_temp*num_of_temp_divided_by_num_of_threds*world_rank;
        double fin_temp = initial_temp + diff_temp*num_of_temp_divided_by_num_of_threds*(world_rank + 1);

        double* sum_total_energy_array = new double[num_of_temp_divided_by_num_of_threds];
        double* sum_total_energy_squared_array = new double[num_of_temp_divided_by_num_of_threds];
        double* sum_total_magnetization_array = new double[num_of_temp_divided_by_num_of_threds];
        double* sum_total_magnetization_absolute_array = new double[num_of_temp_divided_by_num_of_threds];
        double* sum_total_magnetization_squared_array = new double[num_of_temp_divided_by_num_of_threds];

        double temp;

        for (int num_iteration = 0; num_iteration <= num_of_temp_divided_by_num_of_threds; num_iteration++)
        {   // looping over temperature values

            temp = init_temp + diff_temp*num_iteration;
            // pre-calculated exponential values
            exp_delta_energy[0]  = std::exp(8*J/temp);
            exp_delta_energy[4]  = std::exp(4*J/temp);
            exp_delta_energy[8]  = 1;
            exp_delta_energy[12] = std::exp(-4*J/temp);
            exp_delta_energy[16] = std::exp(-8*J/temp);

            mc_iteration_stable(temp);

            sum_total_energy_array[world_rank*num_iteration] = sum_total_energy;
            sum_total_energy_squared_array[world_rank*num_iteration] = sum_total_energy_squared;
            sum_total_magnetization_array[world_rank*num_iteration] = sum_total_magnetization;
            sum_total_magnetization_absolute_array[world_rank*num_iteration] = sum_total_magnetization_absolute;
            sum_total_magnetization_squared_array[world_rank*num_iteration] = sum_total_magnetization_squared;
        }
        MPI_Finalize();
    }
};


int main()
{   
    int spin_matrix_dim = 2;
    int mc_iterations = 1e3;
    
    double initial_temp = 0.5;
    double final_temp = 2;
    double num_of_points = 10;

    time_t seed;
    time(&seed);
    
    ParallelEnergySolver convergence_model(spin_matrix_dim, mc_iterations, seed);
    convergence_model.iterate_temperature_parallel(initial_temp, final_temp, num_of_points);

    return 0;
}