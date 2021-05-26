#pragma once

#include <cvs/pipeline/iview.hpp>
#include <tbb/flow_graph.h>

#include <tuple>

namespace cvs::pipeline::tbb {

template <typename, typename>
class TbbView;

template <typename... In, typename... Out>
class TbbView<std::tuple<In...>, std::tuple<Out...>> : public IView {
 public:
  bool addSender(std::size_t i, std::any sndr) override {
    //    return false;
    return connectToPort<0>(std::move(sndr), i, senders);
  }

  bool addReceiver(std::size_t i, std::any rcvr) override {
    //    return false;
    return connectToPort<0>(std::move(rcvr), i, receivers);
  }

  template <std::size_t I>
  static bool connectToPort(std::any sndr, std::size_t i, std::tuple<>& list) {
    return false;
  }

  template <std::size_t I, template <typename> typename Port, typename... T>
  static bool connectToPort(std::any sndr, std::size_t i, std::tuple<Port<T>*...>& list) {
    if constexpr (std::tuple_size_v<std::tuple<Port<T>*...>> == 0)
      return false;

    if (I == i) {
      using Type        = decltype(std::get<I>(list));
      std::get<I>(list) = std::any_cast<Type>(sndr);

      return true;
    }

    if constexpr (I + 1 < std::tuple_size_v<std::tuple<Port<T>*...>>)
      return connectToPort<I + 1>(std::move(sndr), i, list);

    return false;
  }

 protected:
  template <std::size_t I>
  using InType = std::remove_cvref_t<std::tuple_element_t<I, std::tuple<In...>>>;

  template <std::size_t I>
  using OutType = std::remove_cvref_t<std::tuple_element_t<I, std::tuple<Out...>>>;

  template <std::size_t I>
  bool tryGet(InType<I>& data) {
    auto sender = std::get<I>(senders);
    if (sender)
      return sender->try_get(data);
    return false;
  }

  template <std::size_t I>
  bool tryPut(const OutType<I>& data) {
    auto receiver = std::get<I>(receivers);
    if (receiver)
      return receiver->try_put(data);
    return false;
  }

 protected:
  std::tuple<::tbb::flow::sender<In>*...>    senders;
  std::tuple<::tbb::flow::receiver<Out>*...> receivers;
};

}  // namespace cvs::pipeline::tbb
