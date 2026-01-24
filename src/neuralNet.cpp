#include "pokemonai/neuralNet.h"

#include <fmt/format.h>
#include <torch/torch.h>

#include <algorithm>
#include <boost/filesystem.hpp>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "pokemonai/init_toolbox.h"

namespace po = boost::program_options;


boost::program_options::options_description neuralNet::Config::options(
    const std::string& category, std::string prefix) {
  po::options_description desc(category);
  if (prefix.size() > 0 && prefix.back() != '-') { prefix.append("-"); }
  // clang-format off
  desc.add_options()
    ((prefix + "architecture").c_str(),
    po::value<std::vector<int>>(&architecture)->multitoken(),
    "Architecture of the neural network (size of hidden layers)")
    ((prefix + "model-path").c_str(),
    po::value<std::string>(&modelPath),
    "Path to a pre-trained model file");
  // clang-format on
  return desc;
}


neuralNet::neuralNet(const Config& cfg, const FeatureVector& featureVector)
    : neuralNet(cfg, featureVector.inputSize(), featureVector.outputSize()) {}

neuralNet::neuralNet(const Config& cfg, size_t inputSize, size_t outputSize)
    : cfg_(cfg) {
  std::vector<size_t> layer_sizes = {inputSize};
  layer_sizes.insert(
      layer_sizes.end(), cfg.architecture.begin(), cfg.architecture.end());
  layer_sizes.push_back(outputSize);

  model = torch::nn::Sequential();
  inputBuffer.resize(layer_sizes.front(), 0.0f);
  outputBuffer.resize(layer_sizes.back(), 0.0f);

  auto add_layer = [this](size_t input_size, size_t output_size) {
    model->push_back(torch::nn::Linear(input_size, output_size));
    model->push_back(torch::nn::Sigmoid());
  };

  for (size_t i = 0; i < layer_sizes.size() - 1; ++i) {
    add_layer(layer_sizes[i], layer_sizes[i + 1]);
  }

  layerWidths = std::move(layer_sizes);
  updateIdent();
}


void neuralNet::updateIdent() {
  // Set name based on modelPath or architecture hash
  if (!cfg_.modelPath.empty()) {
    setName(boost::filesystem::path(cfg_.modelPath).stem().string());
  } else if (!layerWidths.empty()) {
    size_t hash = 0;
    for (size_t val : layerWidths) {
      hash ^= std::hash<size_t>{}(val) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    std::stringstream ss;
    ss << std::hex << std::setw(8) << std::setfill('0') << (hash & 0xFFFFFFFF);
    setName(ss.str());
  }
}


void neuralNet::feedForward() {
    if (!model) return;
    
    torch::NoGradGuard no_grad;
    torch::Tensor input = torch::from_blob(inputBuffer.data(), {1, (long)inputBuffer.size()}, torch::kFloat);
    torch::Tensor output = model->forward(input);
    
    std::memcpy(outputBuffer.data(), output.data_ptr<float>(), outputBuffer.size() * sizeof(float));
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


neuralNet& neuralNet::initialize() {
  if (initialized_) { return *this; }

  if (cfg_.modelPath.empty()) {
    throw std::invalid_argument(
        "neuralNet: modelPath is required for initialization");
  }

  loadModel();
  initialized_ = true;

  return *this;
}


void neuralNet::loadModel() {
  SPDLOG_WARN("Loading neural network from {}...", cfg_.modelPath);
  std::ifstream iFile(cfg_.modelPath, std::ios::binary);
  if (!iFile) {
    throw std::invalid_argument(fmt::format(
        "neuralNet: could not open model file {}!", cfg_.modelPath));
  }
  input(iFile);

  if (model.is_empty()) {
    throw std::invalid_argument(
        "neuralNet: model not initialized after loading!");
  }
}


void neuralNet::output(std::ostream& oFile) const {
    if (!model) return;
    torch::save(model, oFile);
}


void neuralNet::input(std::istream& iFile) {
  if (!model) {
    throw std::invalid_argument("neuralNet: model not initialized");
  }
  try {
    torch::load(model, iFile);
  } catch (const std::exception& e) {
    SPDLOG_CRITICAL("Failed to load neural network: {}", e.what());
    throw;
  }
}
