
#ifndef VAZTERAN_APPLICATION_HPP
#define VAZTERAN_APPLICATION_HPP

#include <string>

namespace vzt{
    class Instance;
    class Renderer;
    class Window;

    class Application {
    public:
        explicit Application(std::string_view name);

        void Run();

        ~Application();
    private:
        std::unique_ptr<vzt::Instance> m_instance;
        std::unique_ptr<vzt::Window> m_window;
        std::unique_ptr<vzt::Renderer> m_renderer;
    };
}


#endif //VAZTERAN_APPLICATION_HPP
