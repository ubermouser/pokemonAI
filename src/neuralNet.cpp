#include "pokemonai/neuralNet.h"
#include <torch/torch.h>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <iomanip>
#include "pokemonai/init_toolbox.h"


void neuralNet::feedForward() {
    if (!model) return;
    
    torch::NoGradGuard no_grad;
    torch::Tensor input = torch::from_blob(inputBuffer.data(), {1, (long)inputBuffer.size()}, torch::kFloat);
    torch::Tensor output = model->forward(input);
    
    std::memcpy(outputBuffer.data(), output.data_ptr<float>(), outputBuffer.size() * sizeof(float));
}

void neuralNet::randomizeWeights() {
    if (!model) return;
    torch::NoGradGuard no_grad;
    for (auto& param : model->parameters()) {
        if (param.dim() >= 2) {
            torch::nn::init::xavier_uniform_(param);
        } else {
            torch::nn::init::zeros_(param);
        }
    }
}

void neuralNet::jitterWeights(float jitterMax) {
    if (!model) return;
    torch::NoGradGuard no_grad;
    for (auto& param : model->parameters()) {
        param.add_(torch::randn(param.sizes()) * jitterMax);
    }
}

void neuralNet::clearInput() {
    std::fill(inputBuffer.begin(), inputBuffer.end(), 0.0f);
}

void neuralNet::clear() {
    model = nullptr;
    inputBuffer.clear();
    outputBuffer.clear();
    layerWidths.clear();
}

void neuralNet::output(std::ostream& oFile) const {
    if (!model) return;
    torch::save(model, oFile);
}

bool neuralNet::input(std::istream& iFile) {
    if (!model) return false;
    try {
        torch::load(model, iFile);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Failed to load neural network: {}", e.what());
        return false;
    }
    return true;
}
