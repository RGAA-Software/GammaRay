#pragma once

#include "tc_common_new/data.h"

#include <mutex>
#include <vector>
#include <functional>

namespace tc
{

    class FFT32 {
    public:

        static void DoFFT(std::vector<double> &fft, DataPtr one_channel_pcm_data, int bytes = 0);

        static std::mutex fft_mtx;

    };

}
