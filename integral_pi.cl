__kernel void pi(int niters, float step_size, __local float* local_sums, __global float* partial_sums) {
    const int num_work_items = get_local_size(0);
    const int local_id = get_local_id(0);
    const int group_id = get_group_id(0);
    const istart = (group_id * num_work_items + local_id) * niters;
    const iend = istart + niters;
    float accum = 0.0f;
    for (int i = istart; i < iend; i++) {
        const float x = (i + .5f) * step_size;
        accum += 4.f / (1.f + x * x);
    }
    local_sums[local_id] = accum;
    barrier(CLK_LOCAL_MEM_FENCE);
    if (local_id == 0) {
        float sum = 0.f;
        for (int i = 0; i < num_work_items; i++) sum += local_sums[i];
        partial_sums[group_id] = sum;
    }
}
