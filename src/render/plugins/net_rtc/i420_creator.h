#ifndef I420_CREATOR_H_
#define I420_CREATOR_H_

#include <cstddef>
#include <cstdint>
#include <vector>
#include <functional>
#include <thread>

namespace tc
{

    class I420Creator {
    public:
        using I420Frame = std::shared_ptr<std::vector<uint8_t>>;
        using I420FrameObserver = std::function<void(I420Frame)>;
    public:
        explicit I420Creator(I420FrameObserver &&observer)
                : observer_(observer) {};

        ~I420Creator();

        void set_resolution(int w, int h) {
            w_ = w;
            h_ = h;
        }

        void run(int fps = 30);

    private:
        //I420Frame process();
        I420Frame ReadI420File();

        int w_ = 0;
        int h_ = 0;
        I420FrameObserver observer_;
        bool running_ = false;
        std::thread thread_;

        I420Frame i420_frame_ = nullptr;
    };

}

#endif
