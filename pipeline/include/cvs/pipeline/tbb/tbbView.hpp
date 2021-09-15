#pragma once

#include <cvs/common/config.hpp>
#include <cvs/pipeline/iview.hpp>
#include <cvs/pipeline/tbb/tbbBroadcastNode.hpp>
#include <cvs/pipeline/tbb/tbbBufferNode.hpp>
#include <fmt/format.h>
#include <tbb/flow_graph.h>

#include <map>
#include <tuple>

namespace cvs::pipeline::tbb {

namespace detail {

CVS_CONFIG(InputsConfig, "Input slots description.") {
  CVS_FIELD(from, std::string, "Output node name.");
  CVS_FIELD_DEF(output, std::size_t, 0, "Output node slot.");
  CVS_FIELD(input, std::size_t, "View input slot.");
};

CVS_CONFIG(OutputsConfig, "Output slots description.") {
  CVS_FIELD(to, std::string, "Input node name.");
  CVS_FIELD_DEF(input, std::size_t, 0, "Input node slot.");
  CVS_FIELD(output, std::size_t, "View output slot.");
};

}  // namespace detail

template <typename, typename>
class TbbView;

template <typename... In, typename... Out>
class TbbView<std::tuple<In...>, std::tuple<Out...>> : public cvs::pipeline::IView {
 public:
  template <typename T>
  using Brodcast = std::shared_ptr<cvs::pipeline::tbb::TbbBroadcastNode<cvs::pipeline::NodeType::Unknown, T>>;
  template <typename T>
  using Buffer = std::shared_ptr<cvs::pipeline::tbb::TbbBufferNode<cvs::pipeline::NodeType::Unknown, T>>;

  template <std::size_t I>
  using InType = std::remove_cvref_t<std::tuple_element_t<I, std::tuple<In...>>>;
  template <std::size_t I>
  using OutType = std::remove_cvref_t<std::tuple_element_t<I, std::tuple<Out...>>>;

  using InputTuple  = std::tuple<In...>;
  using OutputTuple = std::tuple<Out...>;

  TbbView(const boost::property_tree::ptree& cfg, cvs::pipeline::IExecutionGraphPtr g)
      : graph(g)
      , config(cfg) {}

  static void parseSettings(TbbView&                                                       view,
                            const common::Properties&                                      config,
                            const std::map<std::string, cvs::pipeline::IExecutionNodePtr>& nodes) {
    auto inputs = config.get_child("inputs");
    for (auto in : inputs) {
      auto params = detail::InputsConfig::make(in.second).value();

      auto sender_iter = nodes.find(params.from);
      if (sender_iter == nodes.end())
        throw std::runtime_error("Can't find node " + params.from);
      auto sender = sender_iter->second->sender(params.output);
      if (!view.addSender(params.input, std::move(sender)))
        throw std::runtime_error(
            fmt::format("Can't connect node {}:{} with input {}", params.from, params.output, params.input));
    }

    auto outputs = config.get_child("outputs");
    for (auto out : outputs) {
      auto params = detail::OutputsConfig::make(out.second).value();

      auto receiver_iter = nodes.find(params.to);
      if (receiver_iter == nodes.end())
        throw std::runtime_error("Can't find node " + params.to);
      auto receiver = receiver_iter->second->receiver(params.input);
      if (!view.addReceiver(params.output, std::move(receiver)))
        throw std::runtime_error(
            fmt::format("Can't connect node {}:{} with output {}", params.to, params.input, params.output));
    }
  }

  bool addSender(std::size_t i, std::any sender) override { return connectToPort<0>(std::move(sender), i, inputs); }
  bool addReceiver(std::size_t i, std::any receiver) override {
    return connectToPort<0>(std::move(receiver), i, outputs);
  }

 protected:
  template <std::size_t I>
  bool connectToPort(std::any sndr, std::size_t i, std::tuple<>& list) {
    return false;
  }

  template <std::size_t I, template <typename> typename Port, typename... T>
  bool connectToPort(std::any node, std::size_t i, std::tuple<Port<T>...>& list) {
    if constexpr (std::tuple_size_v<std::tuple<Port<T>*...>> == 0)
      return false;

    if (I == i) {
      using Type = decltype(std::get<I>(list));

      if (!std::get<I>(list)) {
        // TODO: find buffer settings
        std::get<I>(list) = std::remove_cvref_t<decltype(std::get<I>(list))>::element_type::make(config, graph, {});
      }

      if (!connectToInternalNode(node, std::get<I>(list))) {
        throw std::runtime_error(fmt::format("Can't connect to port {}. Types {} {}.", i,
                                             boost::core::demangle(typeid(Type).name()),
                                             boost::core::demangle(node.type().name())));
        return false;
      }

      return true;
    }

    if constexpr (I + 1 < std::tuple_size_v<std::tuple<Port<T>*...>>)
      return connectToPort<I + 1>(std::move(node), i, list);

    throw std::out_of_range(fmt::format("Ports count: {}. Can't connect to port {}.", I, i));
    return false;
  }

  template <typename T>
  bool connectToInternalNode(std::any sender, Buffer<T>& buf) {
    return buf->connect(sender, 0);
  }

  template <typename T>
  bool connectToInternalNode(std::any receiver, Brodcast<T>& bc) {
    if (receiver.type() != typeid(::tbb::flow::receiver<T>*))
      return false;

    ::tbb::flow::make_edge(*std::any_cast<::tbb::flow::sender<T>*>(bc->sender(0)),
                           *std::any_cast<::tbb::flow::receiver<T>*>(receiver));
    return true;
  }

  template <std::size_t I>
  bool tryGet(InType<I>& data) {
    if (std::get<I>(inputs))
      return std::get<I>(inputs)->tryGet(data);
    return false;
  }

  template <std::size_t I>
  bool tryPut(const OutType<I>& data) {
    if (std::get<I>(outputs))
      return std::get<I>(outputs)->tryPut(data);
    return false;
  }

 protected:
  std::tuple<Buffer<In>...>    inputs;
  std::tuple<Brodcast<Out>...> outputs;

  cvs::pipeline::IExecutionGraphPtr graph;
  const common::Properties          config;
};

}  // namespace cvs::pipeline::tbb
