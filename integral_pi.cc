#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include "integral_pi_cl.h"

#define ITERS 1000
#define RED "\e[0;31m"
#define GREEN "\e[0;92m"
#define NC "\e[0m"

int main(int argc, char *argv[]) {
    try {
        cl::Context context(CL_DEVICE_TYPE_GPU);
        std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
        std::cout << GREEN "Running on '" << devices[0].getInfo<CL_DEVICE_NAME>() << "' ...\n" NC;
        cl::CommandQueue queue(context, devices[0]);
        cl::Program program(context, kIntegralPiCl, true);
        cl::Kernel ko_pi(program, "pi");
        const size_t nwork_groups = devices[0].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
        const size_t work_group_size = ko_pi.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(devices[0]);
        const size_t nsteps = nwork_groups * work_group_size * ITERS;
        const float step_size = 1.f / static_cast<float>(nsteps);
        std::vector<float> h_psum(nwork_groups);
        std::cout << GREEN << nwork_groups << " work groups of size " << work_group_size
            << ". " << nsteps << " integral steps.\n" NC;
        cl::Buffer d_partial_sums(context, CL_MEM_WRITE_ONLY, sizeof(float) * nwork_groups);
        cl::make_kernel<int, float, cl::LocalSpaceArg, cl::Buffer> cl_pi(program, "pi");
        const auto start = std::chrono::high_resolution_clock::now();
        cl_pi(cl::EnqueueArgs(
                queue, cl::NDRange(nwork_groups * work_group_size), cl::NDRange(work_group_size)),
            ITERS, step_size, cl::Local(sizeof(float) * work_group_size), d_partial_sums);
        cl::copy(queue, d_partial_sums, h_psum.begin(), h_psum.end());
        float pi = 0.f;
        for (size_t i = 0; i < nwork_groups; i++) pi += h_psum[i];
        pi *= step_size;
        const std::chrono::duration<double> duration =
            std::chrono::high_resolution_clock::now() - start;
        std::cout << GREEN "PI=" << pi << "(" <<
            std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()
            << " ms used).\n" NC;
    } catch (cl::Error err) {
        std::cerr << RED "ERROR: " << err.what() << "(" << err.err() << ")\n" NC;
    }
}
