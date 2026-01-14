#ifndef EVALUATOR_NETWORK_H
#define EVALUATOR_NETWORK_H

#include "pokemonai/evaluator.h"
#include "pokemonai/neuralNet.h"
#include <array>
#include <string>

// Forward declarations
class EnvironmentNonvolatile;
class ConstEnvironmentVolatile;

/**
 * @brief Base class for feature vectors, used by experienceNet (deprecated).
 */
class featureVector
{
public:
  virtual ~featureVector() { };
  void seed(neuralNet& cNet, const ConstEnvironmentVolatile& env, size_t iTeam) const;
  virtual void seed(float* cInput, const ConstEnvironmentVolatile& env, size_t iTeam) const = 0;
  void seed(neuralNet::floatIterator_t cInput, const ConstEnvironmentVolatile& env, size_t iTeam) const;
  virtual const float* getInput() const { return NULL; };
  virtual void setNetwork(const neuralNet& cNet) = 0;
  virtual size_t inputSize() const = 0;
  virtual size_t outputSize() const = 0;
  virtual void outputNames(std::ostream& oS) const;
};

typedef std::array< std::array< std::array< std::array<uint8_t, 4> , 6> , 6>, 2> bestMoveOrders_t;
typedef std::array< std::array< std::array< std::array<float, 4> , 6> , 6>, 2> bestMoveDamages_t;
typedef std::array< std::array< std::array< uint8_t, 6> , 6> , 2> orders_t;

namespace featureVector_impl
{
  void generateBestMoves(const EnvironmentNonvolatile& envNV, bestMoveOrders_t& iBestMoves, bestMoveDamages_t& dBestMoves);
  void generateOrders(const bestMoveDamages_t& dBestMoves, orders_t& orders);
};

class EvaluatorNetwork : public Evaluator, public featureVector {
public:
    struct Config : public Evaluator::Config {
        std::string modelPath;

        Config() : Evaluator::Config() {}
        virtual ~Config() {}

        virtual boost::program_options::options_description options(
            const std::string& category="evaluator options", std::string prefix="") override;
    };

    EvaluatorNetwork(const Config& cfg = Config{});
    EvaluatorNetwork(const neuralNet& network, const Config& cfg = Config{});
    EvaluatorNetwork(const EvaluatorNetwork& other);
    virtual ~EvaluatorNetwork() override;

    virtual Evaluator& initialize() override;

    virtual void setNetwork(const neuralNet& network) override;
    virtual Evaluator& setEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& env) override;

    virtual EvalResult calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const override;
    virtual EvalResult calculateFitness(neuralNet& cNet, const ConstEnvironmentVolatile& env, size_t iTeam) const;

    virtual size_t inputSize() const override = 0;
    virtual size_t outputSize() const override = 0;
    virtual const float* getInput() const override;

    virtual void seed(float* cInput, const ConstEnvironmentVolatile& env, size_t iTeam) const override = 0;
    virtual void outputNames(std::ostream& oS) const override = 0;

protected:
    Config cfg_;
    std::shared_ptr<neuralNet> network_;

    bestMoveOrders_t iBestMoves_;
    bestMoveDamages_t dBestMoves_;
    orders_t orders_;

    virtual void generateBestMoves();
    virtual void generateOrders();

    virtual std::string baseName() const override { return "Network"; }
    void updateIdent();
};

#endif // EVALUATOR_NETWORK_H
