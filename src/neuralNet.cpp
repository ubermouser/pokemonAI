#include "pokemonai/neuralNet.h"
#include <torch/torch.h>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <iomanip>
#include "pokemonai/init_toolbox.h"

// neuron methods kept for compatibility but largely unused/deprecated
float& neuron::getWeight(neuralNet& net, size_t iWeight) const { static float dummy = 0.0f; return dummy; }
const float& neuron::getWeight(const neuralNet& net, size_t iWeight) const { static float dummy = 0.0f; return dummy; }
std::vector<float>::iterator neuron::weightsBegin(neuralNet& parent) const { return parent.inputBegin(); }
std::vector<float>::iterator neuron::weightsEnd(neuralNet& parent) const { return parent.inputBegin(); }
std::vector<float>::const_iterator neuron::weightsBegin(const neuralNet& parent) const { return parent.inputBegin(); }
std::vector<float>::const_iterator neuron::weightsEnd(const neuralNet& parent) const { return parent.inputBegin(); }
neuron::neuron(neuralNet& parent, size_t numWeights) : iWeightBegin(0), iWeightEnd(0), iNeuronIndex(0) {}

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

void neuralNet::output(std::ostream& oFile, bool printHeader) const {
    // Basic output for identification, real serialization should use torch::save
    if (printHeader) {
        oFile << "PKNNT_TORCH\t32\t" << layerWidths.size() << "\t" << getName() << "\n";
    }
    for (size_t width : layerWidths) {
        oFile << width << "\t";
    }
    oFile << "\n";
}

bool neuralNet::input(const std::vector<std::string>& lines, size_t& iLine) {
    // Simplified input for now, real deserialization should use torch::load
    if (lines.size() <= iLine) return false;
    // Just a placeholder for now to avoid breaking existing logic completely
    return true;
}
