FROM fedora
RUN sudo dnf install -y cmake clang pcre2-devel glibmm24-devel gtkmm30-devel jsoncpp-devel libsigc++20-devel git i3-devel
COPY . .
RUN mkdir build && cd build
RUN CC=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . --config Release
