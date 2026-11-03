#include <pybind11/pybind11.h>
#include "../serv/components/c_sock_path.h"
#include "cmds.h"
namespace py = pybind11;

PYBIND11_MODULE(_core, m){
    m.doc() = "ACC native functions";
    m.def("http_ping", &http_ping, 
        py::arg("fd"), py::arg("host"), py::arg("path") = "/",
        py::call_guard<py::gil_scoped_release>());
    m.def("control_socket_path", &control_socket_path)
;}