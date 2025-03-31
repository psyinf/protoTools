#pragma once
#include <unordered_set>

namespace protos::utils::details {

template <typename Key, typename F>
inline void once_impl(const Key& key, F f)
{
    static std::unordered_set<Key> once_set;

    if (once_set.insert(key).second) { f(); }
}
} // namespace datafw::utils::details

namespace protos::utils {
/*
 * pattern to execute something only once based on a key.
 * E.g. for logging things only once: datafw::utils::once(std::string{"X not found"}, [&]() { warn("X not found") });
 * Key type must be hashable
 */
template <typename Key, typename F>
inline void once(const Key& key, F f)
{
    // TODO: concepts requires Key type to be usable with unordered_set
    details::once_impl(key, f);
}

} // namespace datafw::utils
