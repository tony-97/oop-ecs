#pragma once

#include <format>

#include <nlohmann/json.hpp>

#include "tmpl/sequence.hpp"
#include "traits.hpp"

template <class ECSMan_t> struct GameFactory_t {

  using json = nlohmann::json;

  constexpr auto LoadScene(const json &scene) const -> void {
    for (auto [k, j] : scene.items()) {
      int i{std::stoi(k)};
      TMPL::Sequence::ForEach_t<typename ECSMan_t::EntitySignatures_t>::Do(
          [&]<class T>() {
            if (i == TMPL::Sequence::IndexOf_v<
                         T, typename ECSMan_t::EntitySignatures_t>) {
              EntityFromJSON<T>(j);
            }
          });
    }
  }

  template <class EntSig_t, class... Args_t>
  constexpr auto EntityFromJSON(const json &j, Args_t &&...args) const -> auto {
    using Components_t = TMPL::Sequence::Difference_t<
        ECS::Traits::Components_t<EntSig_t>,
        TMPL::TypeList_t<std::remove_cvref_t<Args_t>...>>;
    auto e = TMPL::Sequence::Unpacker_t<Components_t>::Call(
        [&]<class... Ts, class... Cmps_t>(Cmps_t &&...cmps) {
          return mECSMan.template CreateEntity<EntSig_t>(
              j.get<Ts>()..., std::forward<Cmps_t>(cmps)...);
        },
        std::forward<Args_t>(args)...);
    ConfigureEntityFromJson(e, j);

    return e;
  }

  template <class EntSig_t, class... Args_t>
  constexpr auto EntityFromConfig(Args_t &&...args) const -> auto {
    constexpr auto type_id{
        TMPL::Sequence::IndexOf_v<EntSig_t,
                                  typename ECSMan_t::EntitySignatures_t>};
    if (mConfig.contains(std::format("{}", type_id))) {
      return EntityFromJSON<EntSig_t>(mConfig[std::to_string(type_id)],
                                      std::forward<Args_t>(args)...);
    } else {
      return mECSMan.template CreateEntity<EntSig_t>();
    }
  }

protected:
  ECSMan_t &mECSMan;
  const json mConfig{};
};
