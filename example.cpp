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

#include <iostream>
#include <string>

#include "callbackimpl.h"

/* Our callback has 3 parameters: */
typedef ::OCallback::Callback<int, const char*, std::string> CB;

/*
 * This class is a simple callback receiver. There are no conditions
 * how the class should be. It can be structure, class, polymorphic
 * class. There is no need to inherit from anything.
 */
class A {
  private:
    std::string name;

  public:
    explicit A(
        const std::string& name_) :
      name(name_) {

    }

    /* This callback method matches exactly the signature of the callback
       object. */
    void CB1(
        int arg1_,
        const char* arg2_,
        const std::string& arg3_) {
      std::cout << name << " (CB1): " << arg1_ << " " << arg2_ << " " 
          << arg3_ << std::endl;
    }

  /* This method misses last callback argument. This feature
     allows extension of callbacks - you can append new argument
     but the already existing code is not forced to be changed. */
  void CB2(
      int arg1_,
      const char* arg2_) {
    std::cout << name << " (CB2): " << arg1_ << " " << arg2_ << std::endl;
  }

  /* This method matches the signature and accepts one more user datum. */
  void CB3(
      int arg1_,
      const char* arg2_,
      const std::string& arg3_,
      int udata_) {
    std::cout << name << " (CB3): " << arg1_ << " " << arg2_ << " "
        << arg3_ << " " << udata_ << std::endl;
  }

  /* This method misses last callback argument but it accepts one
     more user datum. */
  void CB4(
      int arg1_,
      const char* arg2_,
      int udata_) {
    std::cout << name << " (CB4): " << arg1_ << " " << arg2_ << " " 
        << udata_ << std::endl;
  }

  /* This method has no arguments but one user datum. */
  void CB5(
      CB* cb_) {
    std::cout << name << " (CB5)" << std::endl;

    /* It's safe to unregister callbacks (including self) from
       the callback method. */
    cb_ -> unregisterCallbackMethod(this, &A::CB5);
  }
};

int main(
    int argc_,
    char* argv_[]) {
  /* -- prepare the receivers */
  A a1("first");
  A a2("second");

  /* -- create the callback object */
  CB cb;

  /* -- register callbacks */
  cb.registerCallbackMethod(&a1, &A::CB1);
  cb.registerCallbackMethod(&a1, &A::CB2);
  cb.registerCallbackMethod(&a1, &A::CB3, 13);
  cb.registerCallbackMethod(&a1, &A::CB4, 15);
  cb.registerCallbackMethod(&a1, &A::CB5, &cb);
  cb.registerCallbackMethod(&a2, &A::CB1);
  cb.registerCallbackMethod(&a2, &A::CB2);
  cb.registerCallbackMethod(&a2, &A::CB3, 210);
  cb.registerCallbackMethod(&a2, &A::CB4, 120);
  cb.registerCallbackMethod(&a2, &A::CB5, &cb);

  /* -- emit first callback */
  cb.emitCallback(10, "ahoj", "Foo");
  std::cout << std::endl;

  /* -- unregister one callback record */
  cb.unregisterCallbackMethod(&a1, &A::CB2);

  /* -- Emit the callback again. Notice that first CB2
        callback and both CB5 callbacks have disappeared. */
  cb.emitCallback(11, "ahoj2", "Foo2");

  return 0;
}

