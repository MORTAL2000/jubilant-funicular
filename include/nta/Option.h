#ifndef NTA_OPTION_H_INCLUDED
#define NTA_OPTION_H_INCLUDED

#include <functional>

#include "nta/MyEngine.h"
#include "nta/format.h"

namespace nta {
    namespace utils {
        /// A class that optionally holds some data
        ///
        /// Used to signal that a value may or may not be present
        ///
        /// T can be a reference type (e.g. Option<std::string&> works as expected)
        template<typename T>
        class Option {
        private:
            using type = typename std::remove_reference<T>::type;
            /// Only exists to avoid a warning when using Option<T&>
            struct Nop { Nop(const type&& _) {} };

            using storage_type = typename std::conditional<std::is_reference<T>::value,
                                                           void*, T>::type;
            using placement_type = typename std::conditional<std::is_reference<T>::value,
                                                             Nop, type>::type;

            /// Private constructor (use some or none instead)
            Option(const T& d) : m_some(true) { 
                if constexpr (std::is_reference<T>::value) {
                    // Maybe I should just memcpy instead
                    new(&m_data) const void*(std::addressof(d));
                } else {
                    new(&m_data) placement_type(std::move(d));
                }
            }

            typename std::aligned_storage_t<sizeof(storage_type), alignof(storage_type)> m_data;
            bool m_some;
        public:
            /// Defaults to none variant
            Option() : m_some(false) {}
            Option(const Option&) = default;
            template<typename S>
            Option(const Option<S>&);
            ~Option() { m_some = false; }
            /// Creates an Option holding some data
            static Option some(const T& data) { return Option<T>(data); }
            /// Creates a None variant Option
            static Option none() { return Option<T>(); }
            /// Same as is_some
            explicit operator bool() const { return m_some; }
            /// Does this hold some data
            bool is_some() const { return m_some; }
            /// Does this hold nothing?
            bool is_none() const { return !m_some; }
            /// Retrieve the underlying data
            ///
            /// You should always call is_some/is_none beforehand
            T get() const;
            /// unwrap and get are the same thing
            T unwrap() const { return get(); }
            /// Return the data held by this Option or optb if it's None
            T get_or(const T& optb) { return m_some ? get() : optb; }
            T unwrap_or(const T& optb) { return get_or(optb); }
            /// Turns this into None variant, calling destructor if necessary
            void destroy();
            /// Returns an Option holding the result of applying func to data
            /// or None if this == None
            template<typename S>
            Option<S> map(std::function<S(T)> func);
            template<typename S>
            Option<S> map(std::function<Option<S>(T)> func);
            template<typename S>
            S map_or(std::function<S(T)> func, const S& def);
            void map(std::function<void(T)> func);
        };
        template<typename T> template<typename S>
        Option<T>::Option(const Option<S>& orig) {
            static_assert(std::is_convertible_v<S, T>,
                          "Option: attempted invalid conversion from Option<S> to Option<T>");
            // This line isn't sketch at all
            *this = orig ? some(orig.unwrap()) : none();
        }
        template<typename T>
        T Option<T>::get() const {
            /// \todo Push an error to ErrorManager instead
            if (!m_some) {
                assert(false && "Tried getting data from a none Option");
            } else if (std::is_reference<T>::value) {
                return (T)**reinterpret_cast<const type * const *>(&m_data);
            } else {
                return (T)*reinterpret_cast<const type*>(&m_data);
            }
        }
        template<typename T>
        void Option<T>::destroy() {
            if (m_some) {
                reinterpret_cast<type*>(&m_data)->~type();
            }
            m_some = false;
        }
        template<typename T> template<typename S>
        Option<S> Option<T>::map(std::function<S(T)> func) {
            return m_some ? Option<S>::some(func(get())) : Option<S>::none();
        }
        template<typename T> template<typename S>
        Option<S> Option<T>::map(std::function<Option<S>(T)> func) {
            return m_some ? func(get()) : Option<S>::none();
        }
        template<typename T> template<typename S>
        S Option<T>::map_or(std::function<S(T)> func, const S& def) {
            return m_some ? func(get()) : def;
        }
        template<typename T>
        void Option<T>::map(std::function<void(T)> func) {
            if (m_some) func(get());
        }
        template<typename T, typename S, typename std::enable_if_t<can_check_equality<T, S>, void*> = nullptr>
        bool operator==(const Option<T>& lhs, const S& rhs) {
            return lhs.is_some() && lhs.unwrap() == rhs;
        }
        template<typename T, typename S, typename std::enable_if_t<can_check_equality<T, S>, void*> = nullptr>
        bool operator==(const Option<T>& lhs, const Option<S>& rhs) {
            return (lhs.is_none() && rhs.is_none()) ||
                   (lhs && rhs && lhs.unwarp() == rhs.unwarp());
        }
        template<typename T, typename S, typename std::enable_if_t<can_check_equality<T, S>, void*> = nullptr>
        bool operator==(const T& lhs, const Option<S>& rhs) {
            return rhs.is_some() && lhs == rhs.unwrap();
        }
        /// Replacement for calling Option<T>::some()
        template<typename T>
        Option<T> make_some(const T& data) {
            return Option<T>::some(data);
        }
        /// Replacement for calling Option<T>::none()
        template<typename T>
        Option<T> make_none() {
            return Option<T>::none();
        }
        template<typename T>
        struct Formatter<Option<T>> {
            std::string operator()(const Option<T>& arg) {
                using decayed = typename std::decay<T>::type;
                if (arg.is_some()) {
                    if constexpr (std::is_invocable_v<Formatter<decayed>, decayed>)  {
                        return format("Some({})", arg.unwrap());
                    } else {
                        return "Some";
                    }
                } else {
                    return "None";
                }
            }
        };
    }
};

#endif // NTA_OPTION_H_INCLUDED
