
#ifndef VAZTERAN_APPLICATION_HPP
#define VAZTERAN_APPLICATION_HPP

#include <string>

namespace vzt{
    class Instance;
    class Window;

    class Application {
    public:
        explicit Application(std::string_view name);

        void Run();

        ~Application();
    private:
        std::unique_ptr<Instance> m_instance;
        std::unique_ptr<Window> m_window;
    };
}


#endif //VAZTERAN_APPLICATION_HPP
