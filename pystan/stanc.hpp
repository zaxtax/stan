#include <iostream>
#include <streambuf>
#include <string>
#include <exception>
#include <stdexcept>

#include "Python.h"

#include <stan/version.hpp>
#include <stan/gm/compiler.hpp>

class std_buf: public std::streambuf {
  public:
    std_buf(std::FILE* file): m_file(file) {}
  protected:
    std::streambuf::int_type overflow(std::streambuf::int_type c) {
      return std::fputc(c, m_file) ==EOF? std::streambuf::traits_type::eof(): c;
    }
    FILE* m_file;
};

std::string stanc(std::string model_stancode, std::string model_name, PyObject* err) {
  std::stringstream out;
  std::istringstream in(model_stancode);
     
    std::cout << "Compiling STAN model now" << std::endl;

    if (!PyFile_Check(err)) {
      throw "Not a python fileobject";
    }

    std::FILE* f = PyFile_AsFile(err);
    std_buf errbuf(f);
    std::ostream errstream(&errbuf);

    bool valid_model
      = stan::gm::compile(&errstream,in,out,model_name);
    if (!valid_model) {
       throw std::invalid_argument("Parsing error");
    }

  return out.str();

}

