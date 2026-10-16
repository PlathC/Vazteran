#include <random>

#include <vzt/compiler.hpp>
#include <vzt/core/logger.hpp>
#include <vzt/render_graph.hpp>
#include <vzt/vulkan/instance.hpp>

int main(int /* argc */, char** /* argv */)
{
    constexpr uint32_t RowNb    = 2048;
    constexpr uint32_t ColumnNb = 1024;
    constexpr uint32_t SampleNb = 1000;

    auto instance = vzt::Instance{"Vazteran Compute"};
    auto device   = instance.getDevice(vzt::DeviceBuilder::offline());
    auto compiler = vzt::Compiler(instance);
    auto queue    = device.getQueue(vzt::QueueType::Compute);

    // Init input buffers
    std::mt19937                   generator;
    std::uniform_real_distribution distribution(-1.f, 1.f);
    std::vector<float>             matrix(RowNb * ColumnNb), vector(ColumnNb);

    for (float& value : matrix)
        value = distribution(generator);

    for (float& value : vector)
        value = distribution(generator);

    // Transfer to GPU
    const auto matrixBuffer = vzt::Buffer::From<float>(device, matrix, vzt::BufferUsage::StorageBuffer);
    const auto vectorBuffer = vzt::Buffer::From<float>(device, vector, vzt::BufferUsage::StorageBuffer);
    const auto resultBuffer = vzt::Buffer(device, RowNb * sizeof(float), vzt::BufferUsage::StorageBuffer);

    for (const auto& [name, threadPerRow] : {std::pair{"thread", true}, {"group", false}, {"warp", false}})
    {
        vzt::TaskGraph graph{device};
        graph.setProfiling(true);

        auto& product = graph.addCompute(name, compiler(std::string("shaders/compute/matvec_") + name + ".slang"));
        product.read(graph.external(matrixBuffer), vzt::Use::StorageBuffer, "matrixData");
        product.read(graph.external(vectorBuffer), vzt::Use::StorageBuffer, "vectorData");
        product.write(graph.external(resultBuffer), vzt::Use::StorageBuffer, "result");
        product.setRecordFunction([&](vzt::ComputeContext& ctx) {
            if (threadPerRow)
                ctx.commands.dispatch(ctx.groups(RowNb)); // Launch one job per row
            else
                ctx.commands.dispatch(RowNb); // Launch one group per row
        });
        graph.compile();

        float duration = 0.f;
        for (uint32_t run = 0; run < SampleNb; run++)
        {
            graph.execute(queue);
            device.wait();
            duration += product.getDuration(0);
        }

        vzt::logger::info("{:<8} {:8.4f} ms", name, duration / SampleNb);
    }

    return 0;
}
