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

#ifndef Staon__OCALLBACK__CALLBACK_IMPL__H_
#define Staon__OCALLBACK__CALLBACK_IMPL__H_

#include "callback.h"

#include <algorithm>
#include <memory>

namespace OCallback {

template<typename... Args_>
Callback<Args_...>::Demarshaller::Demarshaller() {

}

template<typename... Args_>
Callback<Args_...>::Demarshaller::~Demarshaller() {

}

template<typename... Args_>
Callback<Args_...>::Callback() :
  demarshallers() {

}

template<typename... Args_>
Callback<Args_...>::~Callback() {

}

template<typename... Args_>
void Callback<Args_...>::appendDemarshaller(
    Callback<Args_...>::DemarshallerPtr&& dm_) {
  demarshallers.push_back(dm_);
}

template<typename... Args_>
void Callback<Args_...>::removeDemarshaller(
  const Callback<Args_...>::Demarshaller& dm_) {
  auto new_end_(std::remove_if(
      demarshallers.begin(),
      demarshallers.end(),
      [&dm_](
          const DemarshallerPtr& dm2_) -> bool {
        return dm_.isEqual(dm2_.get());
      }));
  demarshallers.erase(new_end_, demarshallers.end());
}

template<typename... Args_>
void Callback<Args_...>::emitCallback(
  typename Private::SelectType<Args_>::Type... args_) {
  const Arguments arguments_(args_...);

  /* -- Make copy of the list to allow modifications from
        the callback methods during the iteration. */
  Demarshallers dms_(demarshallers);
  for(auto demarshaller_ : dms_) {
    demarshaller_ -> emitCallback(arguments_);
  }
}

} /* -- namespace OCallback */

#endif /* -- Staon__OCALLBACK__CALLBACK_IMPL__H_ */

