# ocallback

OCallback is a simple class template designed for distribution
of callbacks. The system is type-safe (based on the C++ variadic
templates) and there is no need to make changes into the receivers
(the receivers don't need to inherit from a base class). Optionally,
the user can store some user data at every callback record.

A commented example of usage can be found in the example.cpp file.

