#ifndef __STAN__AGRAD__REV__OPERATOR_MINUS_EQUAL_HPP__
#define __STAN__AGRAD__REV__OPERATOR_MINUS_EQUAL_HPP__

#include <stan/agrad/rev/var.hpp>
#include <stan/agrad/rev/operator_subtraction.hpp>

namespace stan {
  namespace agrad {

    inline var& var::operator-=(const var& b) {
      vi_ = new subtract_vv_vari(vi_,b.vi_);
      return *this;
    }

    inline var& var::operator-=(const double b) {
      if (b == 0.0)
        return *this;
      vi_ = new subtract_vd_vari(vi_,b);
      return *this;
    }

  }
}
#endif
