/*
Copyright (c) 2018 Ondrej Starek

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef Staon__OCALLBACK__CALLBACK__H_
#define Staon__OCALLBACK__CALLBACK__H_

#include <cassert>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

namespace OCallback {

namespace Private {

/**
 * @brief Compile-time sequence of integers
 */
template<int...>
struct NumSequence {

};

/**
 * @brief Compile-time generator of the sequence of integers
 */
template<int N_, int... Seq_>
struct GenSequence : public GenSequence<N_ - 1, N_ - 1, Seq_...> {

};

template<int... Seq_>
struct GenSequence<0, Seq_...> {
  typedef NumSequence<Seq_...> SeqType;
};

/**
 * @brief Compile-time selection of the most appropriate type
 */
template<typename T_>
struct SelectType {
  typedef typename std::conditional<std::is_scalar<T_>::value, T_, const T_&>::type Type;
};

/**
 * @brief Compile-time counting of function arguments 
 */
template<typename Function_> struct ArgumentCount;

template<typename Receiver_, typename FRet_, typename... FArgs_>
struct ArgumentCount<FRet_ (Receiver_::*)(FArgs_...)> :
    public std::integral_constant<std::size_t, sizeof...(FArgs_)> {

};

} /* -- namespace Private */

/**
 * @brief The callback object
 *
 * This class offers registration and unregistration of callback methods
 * and emission of callbacks.
 */
template<typename... Args_>
class Callback {
  private:
    /**
     * @brief A crate for passing of arguments
     */
    typedef std::tuple<typename Private::SelectType<Args_>::Type...> Arguments;

    /**
     * @brief Generic demarshaller interface
     *
     * Demarshallers are responsible for invocation of callback
     * methods.
     */
    class Demarshaller {
      public:
        /**
         * @brief Ctor
         */
        Demarshaller();

        /**
         * @brief Dtor
         */
        virtual ~Demarshaller();

        /* -- avoid copying */
        Demarshaller(
            const Demarshaller&) = delete;
        Demarshaller& operator = (
            const Demarshaller&) = delete;

        /**
         * @brief Emit the callback
         *
         * @param args_ Callback arguments
         */
        virtual void emitCallback(
            const Arguments& args_) const = 0;

        /**
         * @brief Check equality of two demarshallers
         *
         * The method compares two demarshallers. It's used
         * for removing of demarshallers during unregistration
         * of a callback.
         *
         * @param d2_ Second demarshaller
         * @return True if the demarshallers are equal
         * @warning The @a d2_ demarshaller can be of different
         *     type! The method must handle it!
         */
        virtual bool isEqual(
            const Demarshaller* d2_) const = 0;
    };

    /**
     * @brief Typed demarshaller - the callback function is a method
     *     of an object
     */
    template<typename Receiver_, typename Method_>
    class DemarshallerMethod : public Demarshaller {
      private:
        Receiver_* receiver;
        Method_ method;

      protected:
        template<int... Seq_, int... DataSeq_, typename UserData_>
        void callMethod(
            Private::NumSequence<Seq_...>,
            Private::NumSequence<DataSeq_...>,
            const Arguments& arguments_,
            const UserData_& userdata_) const {
          (receiver->*method)(
              std::get<Seq_>(arguments_)...,
              std::get<DataSeq_>(userdata_)...);
        }


      public:
        /**
         * @brief Ctor
         *
         * @param receiver_ The receiving object. The ownership
         *     is not taken!
         * @param method_ The callback method
         */
        explicit DemarshallerMethod(
            Receiver_* receiver_,
            Method_ method_) :
          receiver(receiver_),
          method(method_) {
 
        }

        /**
         * @brief Dtor
         */
        virtual ~DemarshallerMethod() {

        }

        /* -- avoid copying */
        DemarshallerMethod(
            const DemarshallerMethod&) = delete;
        DemarshallerMethod& operator = (
            const DemarshallerMethod&) = delete;

        virtual bool isEqual(
            const Demarshaller* d2_) const override {
          const DemarshallerMethod* dm2_(
              dynamic_cast<const DemarshallerMethod*>(d2_));
          return 
              dm2_ != nullptr 
              && receiver == dm2_ -> receiver
              && method == dm2_ -> method;
        }

        virtual void emitCallback(
            const Arguments& args_) const override {
          /* -- it should never happen, but I need an instance
                of this class */
          assert(false);
        }
    };

    /**
     * @brief Typed demarshaller with user data - the callback function is a method
     *     of an object
     */
    template<typename Receiver_, typename Method_, typename... Data_>
    class DemarshallerMethodData : public DemarshallerMethod<Receiver_, Method_> {
      private:
        typedef std::tuple<Data_...> UserData;
        UserData userdata;

      public:
        explicit DemarshallerMethodData(
            Receiver_* receiver_,
            Method_ method_,
            Data_... data_) :
          DemarshallerMethod<Receiver_, Method_>(receiver_, method_),
          userdata(data_...) {

        }

        virtual void emitCallback(
            const Arguments& args_) const override {
            /* -- There is a trick here. Count of user data is subtracted
                  from the count of method arguments. Hence, some arguments
                  can be omitted. */
            this -> callMethod(
                typename Private::GenSequence<
                    Private::ArgumentCount<Method_>::value 
                        - std::tuple_size<UserData>::value>::SeqType(),
                typename Private::GenSequence<std::tuple_size<UserData>::value>::SeqType(),
                args_,
                userdata);
        }
    };

    typedef std::shared_ptr<Demarshaller> DemarshallerPtr;
    typedef std::vector<DemarshallerPtr> Demarshallers;
    Demarshallers demarshallers; /**< list of demarshallers */

    /**
     * @brief Append new demarshaller
     *
     * @param dm_ The demarshaller. The ownership is taken!
     */
    void appendDemarshaller(
        DemarshallerPtr&& dm_);

    /**
     * @brief Remove all demarshallers which are equal to @a dm_
     */
    void removeDemarshaller(
        const Demarshaller& dm_);

  public:
    /**
     * @brief Ctor
     */
    Callback();

    /**
     * @brief Dtor
     */
    ~Callback();

    /**
     * @brief Emit the callback
     *
     * @param args_ Callback arguments
     */
    void emitCallback(
        typename Private::SelectType<Args_>::Type... args_);

    /**
     * @brief Register new callback method
     *
     * @param receiver_ Receiver of the callback. The ownership is not taken!
     * @param method_ The callback method. The signature of the method must
     *     be compatible with the callback arguments (less or equal convertible
     *     parameters).
     * @param udata_ Optional user data stored at the callback method. The data
     *     are appended at the end of list of callback parameters.
     *
     * @note The tuple (receiver_, method_) is uses as an identifier of the callback
     *     record. You can register the same callback method several times but
     *     the unregisterCallbackMethod removes all records with the same ID.
     */
    template<typename Receiver_, typename FRet_, typename... FArgs_, typename... UData_>
    void registerCallbackMethod(
        Receiver_* receiver_,
        FRet_ (Receiver_::* method_)(FArgs_...),
        UData_... udata_) {
      typedef DemarshallerMethodData<Receiver_, FRet_ (Receiver_::*)(FArgs_...), UData_...> DM;
      appendDemarshaller(std::make_shared<DM>(receiver_, method_, udata_...));
    }

    /**
     * @brief Unregister all callback records identified by @a receiver_ and @a method_
     */
    template<typename Receiver_, typename FRet_, typename... FArgs_>
    void unregisterCallbackMethod(
        Receiver_* receiver_,
        FRet_ (Receiver_::* method_)(FArgs_...)) {
      typedef DemarshallerMethod<Receiver_, FRet_ (Receiver_::*)(FArgs_...)> DM;
      removeDemarshaller(DM(receiver_, method_));
    }
};

} /* -- namespace OCallback */

#endif /* -- Staon__OCALLBACK__CALLBACK__H_ */

