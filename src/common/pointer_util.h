#ifndef ODR_COMMON_POINTER_UTIL_H
#define ODR_COMMON_POINTER_UTIL_H

namespace odr::common {

template <typename To, typename From, typename Deleter>
std::unique_ptr<To, Deleter>
dynamic_pointer_cast(std::unique_ptr<From, Deleter> &&p) {
  To *cast = dynamic_cast<To *>(p.get());
  if (cast == nullptr)
    return std::unique_ptr<To, Deleter>();
  std::unique_ptr<To, Deleter> result(cast, std::move(p.get_deleter()));
  p.release();
  return result;
}

} // namespace odr::common

#endif // ODR_COMMON_POINTER_UTIL_H
