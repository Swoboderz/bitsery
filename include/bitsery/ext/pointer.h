//MIT License
//
//Copyright (c) 2017 Mindaugas Vinkelis
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#ifndef BITSERY_EXT_POINTER_H
#define BITSERY_EXT_POINTER_H

#include <cassert>
#include "../traits/core/traits.h"
#include "inheritance.h"
#include "utils/pointer_utils.h"
#include "utils/polymorphism_utils.h"
#include "utils/rtti_utils.h"

namespace bitsery {

    namespace ext {

        namespace pointer_details {

            template <typename T>
            struct PtrOwnerManager {
                static_assert(std::is_pointer<T>::value, "");

                using TElement = typename std::remove_pointer<T>::type;

                static TElement* getPtr(T& obj){
                    return obj;
                }

                static constexpr PointerOwnershipType getOwnership() {
                    return PointerOwnershipType::Owner;
                }

                static void assign(T& obj, TElement* valuePtr) {
                    delete obj;
                    obj = valuePtr;
                }

                static void clear(T& obj) {
                    delete obj;
                    obj = nullptr;
                }
            };

            template <typename T>
            struct PtrObserverManager {
                static_assert(std::is_pointer<T>::value, "");

                using TElement = typename std::remove_pointer<T>::type;

                //observer must return reference to pointer, so that it could be updated later
                static TElement*& getPtrRef(T& obj){
                    return obj;
                }

                static TElement* getPtr(T& obj){
                    return obj;
                }

                static constexpr PointerOwnershipType getOwnership() {
                    return PointerOwnershipType::Observer;
                }

                static void assign(T& obj, TElement* valuePtr) {
                    //do not delete existing object
                    obj = valuePtr;
                }

                static void clear(T& obj) {
                    obj = nullptr;
                }

            };

            template <typename T>
            struct NonPtrManager {
                
                static_assert(!std::is_pointer<T>::value, "");
                
                using TElement = T;
                
                static TElement* getPtr(T& obj){
                    return &obj;
                }

                static constexpr PointerOwnershipType getOwnership() {
                    return PointerOwnershipType::Owner;
                }
    
                // this code is unreachable for reference type, but is necessary to compile
                // LCOV_EXCL_START
                static void assign(T& obj, TElement* valuePtr) {}
                static void clear(T& obj) {}
                // LCOV_EXCL_STOP
                
            };

        }

        template <typename RTTI>
        using PointerOwnerBase = pointer_utils::PointerObjectExtensionBase<
                pointer_details::PtrOwnerManager, PolymorphicContext, RTTI>;

        using PointerOwner = PointerOwnerBase<StandardRTTI>;


        using PointerObserver = pointer_utils::PointerObjectExtensionBase<
                pointer_details::PtrObserverManager, PolymorphicContext, NoRTTI>;


        //inherit from PointerObjectExtensionBase in order to specify PointerType::NotNull
        class ReferencedByPointer: public pointer_utils::PointerObjectExtensionBase<
                pointer_details::NonPtrManager, PolymorphicContext, NoRTTI>{
        public:
            ReferencedByPointer():pointer_utils::PointerObjectExtensionBase<
                    pointer_details::NonPtrManager, PolymorphicContext, NoRTTI>(PointerType::NotNull) {}
        };
        
    }

    namespace traits {


        template<typename T, typename RTTI>
        struct ExtensionTraits<ext::PointerOwnerBase<RTTI>, T *> {
            using TValue = T;
            static constexpr bool SupportValueOverload = true;
            static constexpr bool SupportObjectOverload = true;
            //if underlying type is not polymorphic, then we can enable lambda syntax
            static constexpr bool SupportLambdaOverload = !RTTI::template isPolymorphic<TValue>();
        };

        template<typename T>
        struct ExtensionTraits<ext::PointerObserver, T *> {
            //although pointer observer doesn't serialize anything, but we still add value overload support to be consistent with pointer owners
            //observer only writes/reads pointer id from pointer linking context
            using TValue = T;
            static constexpr bool SupportValueOverload = true;
            static constexpr bool SupportObjectOverload = true;
            static constexpr bool SupportLambdaOverload = false;
        };

        template<typename T>
        struct ExtensionTraits<ext::ReferencedByPointer, T> {
            //allow everything, because it is serialized as regular type, except it also creates pointerId that is required by NonOwningPointer to work
            using TValue = T;
            static constexpr bool SupportValueOverload = true;
            static constexpr bool SupportObjectOverload = true;
            static constexpr bool SupportLambdaOverload = true;
        };
    }

}


#endif //BITSERY_EXT_POINTER_H
